/**
 * FDM3D-Enclosure 空气质量监测器固件
 * 
 * 硬件: ESP32 DevKitC
 * 传感器: PMS5003 (PM2.5), SGP30 (TVOC/eCO2), DHT22 (温湿度)
 * 输出: OLED 显示 (硬件 I2C), 串口 JSON, PWM 风扇控制 (LEDC)
 * 配网: WiFiManager 首次配网, 无需硬编码
 * 
 * v0.2 — 修复: OLED I2C 冲突, analogWrite→ledc, 非阻塞蜂鸣, WiFiManager
 * MIT License — FDM3D-Enclosure Project
 */

#include <Arduino.h>
#include <WiFiManager.h>    // WiFiManager 配网
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_SSD1306.h>
#include <DHTNew.h>

#include "config.h"

// ===== 传感器对象 =====
Adafruit_SGP30 sgp;
DHTNew dht(DHT_PIN, DHT22);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_ADDR);  // 复用 Wire (21/22)

// ===== PMS5003 =====
HardwareSerial pmsSerial(2);
struct PmsData {
  uint16_t pm1_0 = 0;
  uint16_t pm2_5 = 0;
  uint16_t pm10 = 0;
  bool valid = false;
  unsigned long lastRead = 0;  // 最后有效读取时间
} pmsData;

// ===== 状态 =====
unsigned long lastReport = 0;
bool sgpReady = false;
unsigned long sgpCalibrationTime = 0;
int fanPwmValue = 0;

// ===== 非阻塞蜂鸣 =====
bool buzzerActive = false;
unsigned long buzzerStartMs = 0;
const unsigned long BUZZER_DURATION = 200;  // ms

// ===== 函数声明 =====
bool readPMS5003();
void updateDisplay(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum);
String createJSON(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum);
void triggerBuzzer();
void handleBuzzer();
void setupLEDC();

void setup() {
  Serial.begin(115200);
  Serial.println(F("[FDM3D-Enclosure] Air Quality Monitor v0.2"));

  // ===== LEDC PWM 初始化 (替代 analogWrite) =====
  setupLEDC();

  // ===== 初始化 I2C (硬件, 21/22) =====
  Wire.begin(SDA_PIN, SCL_PIN);

  // ===== PMS5003 =====
  pmsSerial.begin(9600, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);

  // ===== DHT22 =====
  if (!dht.begin()) {
    Serial.println(F("[ERROR] DHT22 not found!"));
  }

  // ===== SGP30 =====
  if (!sgp.begin()) {
    Serial.println(F("[ERROR] SGP30 not found!"));
  } else {
    sgpReady = true;
    sgpCalibrationTime = millis();
    Serial.println(F("[OK] SGP30 initialized"));
  }

  // ===== OLED (0.96\", 128x64, 同 I2C 总线) =====
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[ERROR] OLED not found!"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println(F("FDM3D-Enclosure"));
    display.println(F("Air Quality Monitor"));
    display.display();
    delay(1500);  // 仅启动时一次性延迟
  }

  // ===== 蜂鸣器 =====
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // ===== WiFiManager 配网 =====
  WiFiManager wm;
  // 设置配网门户名称
  wm.setConfigPortalTimeout(180);  // 3分钟超时
  bool wifiConnected = wm.autoConnect("FDM3D-Enclosure");

  if (wifiConnected) {
    Serial.print(F("[OK] WiFi connected. IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("[WARN] WiFi not connected, running offline"));
  }

  Serial.println(F("[READY] Monitoring started"));
}

void loop() {
  unsigned long now = millis();

  // ===== 读取 PMS5003 =====
  pmsData.valid = readPMS5003();

  // ===== SGP30 初始化后等待 15s 稳定 =====
  if (sgpReady && sgpCalibrationTime > 0 && (now - sgpCalibrationTime) > 15000) {
    sgpCalibrationTime = 0;
    Serial.println(F("[OK] SGP30 calibration ready"));
  }

  // ===== 定时上报 =====
  if (now - lastReport >= REPORT_INTERVAL) {
    lastReport = now;

    // --- 读取所有传感器 ---
    float pm25 = pmsData.valid ? (float)pmsData.pm2_5 : 0.0f;

    uint16_t tvoc = 0, eco2 = 0;
    if (sgpReady && sgpCalibrationTime == 0) {
      if (!sgp.IAQmeasure()) {
        Serial.println(F("[WARN] SGP30 read failed"));
      } else {
        tvoc = sgp.TVOC;
        eco2 = sgp.eCO2;
      }
    }

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    // --- PM2.5 降权: 如果传感器长时间无数据, 逐渐降权 ---
    if (!pmsData.valid && (now - pmsData.lastRead) > 30000) {
      pm25 = 0;  // 传感器离线超过30s, 归零
    }

    // --- PID 风扇控制 (简化版) ---
    float pmFactor = constrain(pm25 / 100.0, 0.0, 1.0);
    float vocFactor = constrain(tvoc / 500.0, 0.0, 1.0);
    fanPwmValue = (int)(max(pmFactor, vocFactor) * 255);
    ledcWrite(FAN_PWM_CHANNEL, fanPwmValue);

    // --- 报警检测 (非阻塞) ---
    if (pm25 > ALERT_PM25 || tvoc > ALERT_TVOC || eco2 > ALERT_ECO2) {
      triggerBuzzer();
    }

    // --- 更新 OLED ---
    updateDisplay(pm25, tvoc, eco2, temp, hum);

    // --- 串口 JSON 输出 ---
    String json = createJSON(pm25, tvoc, eco2, temp, hum);
    Serial.println(json);
  }

  // ===== 非阻塞蜂鸣处理 =====
  handleBuzzer();
}

// ============================================================
//  LEDC PWM 初始化 (替代 ESP32 不支持的 analogWrite)
// ============================================================
void setupLEDC() {
  ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION);
  ledcAttachPin(FAN_CTRL_PIN, FAN_PWM_CHANNEL);
  ledcWrite(FAN_PWM_CHANNEL, 0);
}

// ============================================================
//  读取 PMS5003 颗粒物数据
// ============================================================
bool readPMS5003() {
  static uint8_t buffer[32];
  static uint8_t index = 0;

  while (pmsSerial.available()) {
    uint8_t byte = pmsSerial.read();
    if (index == 0 && byte != 0x42) continue;
    if (index == 1 && byte != 0x4D) { index = 0; continue; }
    
    buffer[index++] = byte;
    if (index == 32) {
      index = 0;
      
      // 校验和
      uint16_t checksum = (buffer[30] << 8) | buffer[31];
      uint16_t sum = 0;
      for (int i = 0; i < 30; i++) sum += buffer[i];
      
      if (sum == checksum) {
        pmsData.pm1_0 = (buffer[4] << 8) | buffer[5];
        pmsData.pm2_5 = (buffer[6] << 8) | buffer[7];
        pmsData.pm10  = (buffer[8] << 8) | buffer[9];
        pmsData.lastRead = millis();
        return true;
      }
    }
  }
  return false;
}

// ============================================================
//  更新 OLED 显示 (同 I2C 总线, 无冲突)
// ============================================================
void updateDisplay(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print(F("PM2.5: "));
  if (pm25 > 0) { display.print(pm25, 0); display.println(F(" ug/m3")); }
  else { display.println(F("-- ug/m3")); }

  display.setCursor(0, 10);
  display.print(F("TVOC:  "));
  display.print(tvoc); display.println(F(" ppb"));

  display.setCursor(0, 20);
  display.print(F("eCO2:  "));
  display.print(eco2); display.println(F(" ppm"));

  display.setCursor(0, 30);
  display.print(F("Temp:  "));
  if (!isnan(temp)) { display.print(temp, 1); display.println(F(" C")); }
  else { display.println(F("-- C")); }

  display.setCursor(0, 40);
  display.print(F("Hum:   "));
  if (!isnan(hum)) { display.print(hum, 1); display.println(F(" %")); }
  else { display.println(F("-- %")); }

  // WiFi + 风扇状态
  display.setCursor(0, 54);
  if (WiFi.status() == WL_CONNECTED) {
    display.print(F("WiFi:ON "));
  } else {
    display.print(F("CFG:ON "));  // 配置模式
  }
  display.print(F("FAN:"));
  display.print((fanPwmValue * 100) / 255);
  display.print(F("%"));

  display.display();
}

// ============================================================
//  生成 JSON 数据
// ============================================================
String createJSON(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum) {
  StaticJsonDocument<256> doc;
  doc["pm25"] = pm25;
  doc["tvoc"] = tvoc;
  doc["eco2"] = eco2;
  doc["temp"] = isnan(temp) ? (float)0.0 : temp;
  doc["hum"] = isnan(hum) ? (float)0.0 : hum;
  doc["fan_pwm"] = fanPwmValue;
  
  String output;
  serializeJson(doc, output);
  return output;
}

// ============================================================
//  非阻塞蜂鸣器触发
// ============================================================
void triggerBuzzer() {
  if (!buzzerActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerActive = true;
    buzzerStartMs = millis();
  }
}

void handleBuzzer() {
  if (buzzerActive && (millis() - buzzerStartMs >= BUZZER_DURATION)) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
  }
}
