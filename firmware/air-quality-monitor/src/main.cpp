/**
 * FDM3D-Enclosure 空气质量监测器固件
 * 
 * 硬件: ESP32 DevKitC
 * 传感器: PMS5003 (PM2.5), SGP30 (TVOC/eCO2), DHT22 (温湿度)
 * 输出: OLED 显示, 串口数据, PWM 风扇控制信号
 * 
 * MIT License — FDM3D-Enclosure Project
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_SSD1306.h>
#include <DHTNew.h>

#include "config.h"

// ===== 传感器对象 =====
Adafruit_SGP30 sgp;
DHTNew dht(DHT_PIN, DHT22);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_ADDR);

// ===== PMS5003 =====
HardwareSerial pmsSerial(2);
struct PmsData {
  uint16_t pm1_0 = 0;
  uint16_t pm2_5 = 0;
  uint16_t pm10 = 0;
  bool valid = false;
} pmsData;

// ===== 状态 =====
unsigned long lastReport = 0;
bool sgpReady = false;
unsigned long sgpCalibrationTime = 0;

// ===== 函数声明 =====
bool readPMS5003();
void updateDisplay(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum);
String createJSON(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum);

void setup() {
  Serial.begin(115200);
  Serial.println(F("[FDM3D-Enclosure] Air Quality Monitor v0.1"));

  // 初始化 I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // 初始化 PMS5003
  pmsSerial.begin(9600, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);

  // 初始化 DHT22
  if (!dht.begin()) {
    Serial.println(F("[ERROR] DHT22 not found!"));
  }

  // 初始化 SGP30
  if (!sgp.begin()) {
    Serial.println(F("[ERROR] SGP30 not found!"));
  } else {
    sgpReady = true;
    sgpCalibrationTime = millis();
    Serial.println(F("[OK] SGP30 initialized"));
  }

  // 初始化 OLED
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
  }

  // 蜂鸣器初始化
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // 风扇控制引脚
  pinMode(FAN_CTRL_PIN, OUTPUT);
  analogWrite(FAN_CTRL_PIN, 0);

  // WiFi 初始化
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("Connecting to WiFi"));
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("\n[OK] WiFi connected"));
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("\n[WARN] WiFi not connected, running offline"));
  }

  delay(1000);
  Serial.println(F("[READY] Monitoring started"));
}

void loop() {
  unsigned long now = millis();

  // 读取 PMS5003
  pmsData.valid = readPMS5003();

  // SGP30 初始化后需要一点时间稳定
  if (sgpReady && sgpCalibrationTime > 0 && (now - sgpCalibrationTime) > 15000) {
    sgpCalibrationTime = 0;  // 标记校准完成
  }

  // 每 REPORT_INTERVAL 上报一次
  if (now - lastReport >= REPORT_INTERVAL) {
    lastReport = now;

    // 读取所有传感器
    float pm25 = pmsData.valid ? (float)pmsData.pm2_5 : 0;

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

    // PID 风扇控制 (简化版)
    // 根据 PM2.5 和 TVOC 联合决策风扇转速
    float pmFactor = constrain(pm25 / 100.0, 0.0, 1.0);
    float vocFactor = constrain(tvoc / 500.0, 0.0, 1.0);
    float fanSpeed = max(pmFactor, vocFactor) * 255;
    analogWrite(FAN_CTRL_PIN, (int)fanSpeed);

    // 更新显示
    updateDisplay(pm25, tvoc, eco2, temp, hum);

    // 报警检测
    if (pm25 > ALERT_PM25 || tvoc > ALERT_TVOC || eco2 > ALERT_ECO2) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
    }

    // 串口 JSON 输出
    String json = createJSON(pm25, tvoc, eco2, temp, hum);
    Serial.println(json);

    // 通过 WiFi 发送 (如果后续实现 Web 服务器)
  }
}

/**
 * 读取 PMS5003 颗粒物数据
 */
bool readPMS5003() {
  static uint8_t buffer[32];
  static uint8_t index = 0;

  while (pmsSerial.available()) {
    uint8_t byte = pmsSerial.read();
    if (index == 0 && byte != 0x42) continue;  // 等待帧头
    if (index == 1 && byte != 0x4D) { index = 0; continue; }
    
    buffer[index++] = byte;
    if (index == 32) {
      index = 0;
      
      // 校验
      uint16_t checksum = (buffer[30] << 8) | buffer[31];
      uint16_t sum = 0;
      for (int i = 0; i < 30; i++) sum += buffer[i];
      
      if (sum == checksum) {
        pmsData.pm1_0 = (buffer[4] << 8) | buffer[5];
        pmsData.pm2_5 = (buffer[6] << 8) | buffer[7];
        pmsData.pm10  = (buffer[8] << 8) | buffer[9];
        return true;
      }
    }
  }
  return false;
}

/**
 * 更新 OLED 显示
 */
void updateDisplay(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(F("PM2.5: "));
  display.print(pm25, 0);
  display.println(F(" ug/m3"));

  display.setCursor(0, 10);
  display.print(F("TVOC:  "));
  display.print(tvoc);
  display.println(F(" ppb"));

  display.setCursor(0, 20);
  display.print(F("eCO2:  "));
  display.print(eco2);
  display.println(F(" ppm"));

  display.setCursor(0, 30);
  display.print(F("Temp:  "));
  display.print(temp, 1);
  display.println(F(" C"));

  display.setCursor(0, 40);
  display.print(F("Hum:   "));
  display.print(hum, 1);
  display.println(F(" %"));

  // WiFi 状态
  display.setCursor(70, 54);
  if (WiFi.status() == WL_CONNECTED) {
    display.print(F("WiFi:ON"));
  } else {
    display.print(F("WiFi:OFF"));
  }

  display.display();
}

/**
 * 生成 JSON 数据
 */
String createJSON(float pm25, uint16_t tvoc, uint16_t eco2, float temp, float hum) {
  StaticJsonDocument<256> doc;
  doc["pm25"] = pm25;
  doc["tvoc"] = tvoc;
  doc["eco2"] = eco2;
  doc["temp"] = temp;
  doc["hum"] = hum;
  doc["fan_pwm"] = analogRead(FAN_CTRL_PIN);
  
  String output;
  serializeJson(doc, output);
  return output;
}
