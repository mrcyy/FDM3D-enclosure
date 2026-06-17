/**
 * FDM3D-Enclosure 风扇控制器
 * 
 * 硬件: Arduino Nano
 * 输入: 接收 ESP32 控制信号 (PWM / 串口)
 * 输出: 排风扇 + 进气风扇 PWM 控制
 * 
 * MIT License — FDM3D-Enclosure Project
 */

#include <Arduino.h>

// ===== 引脚定义 =====
#define FAN_EXHAUST_PWM     9     // 排风扇 PWM
#define FAN_INTAKE_PWM      10    // 进气风扇 PWM
#define CTRL_SIGNAL_PIN     A0    // 来自 ESP32 的模拟控制信号
#define MANUAL_OVERRIDE     7     // 手动调速按钮
#define POT_SPEED           A1    // 电位器手动调速

// ===== 参数 =====
#define PWM_FREQ            25000 // Hz (超出人耳范围)
#define PWM_RESOLUTION      8     // bits (0-255)

unsigned long lastCtrlRead = 0;
const unsigned long READ_INTERVAL = 200; // ms

/**
 * 将 ESP32 传入的模拟电压映射到风扇 PWM
 * 0-5V → 0-255
 */
uint8_t mapControlSignal() {
  int raw = analogRead(CTRL_SIGNAL_PIN);    // 0-1023
  return map(constrain(raw, 0, 1023), 0, 1023, 0, 255);
}

void setup() {
  pinMode(FAN_EXHAUST_PWM, OUTPUT);
  pinMode(FAN_INTAKE_PWM, OUTPUT);
  pinMode(MANUAL_OVERRIDE, INPUT_PULLUP);
  pinMode(POT_SPEED, INPUT);

  // 设置 PWM 频率 (需要修改计时器)
  // TCCR1A = (TCCR1A & ~0x0F) | 0x01;
  // TCCR1B = (TCCR1B & ~0xE7) | 0x09;  // 8 分频

  digitalWrite(FAN_EXHAUST_PWM, LOW);
  digitalWrite(FAN_INTAKE_PWM, LOW);

  Serial.begin(9600);
  Serial.println(F("[FanCtrl] FDM3D-Enclosure Fan Controller v0.1"));
}

void loop() {
  unsigned long now = millis();

  if (now - lastCtrlRead >= READ_INTERVAL) {
    lastCtrlRead = now;

    uint8_t pwmValue;

    // 手动调速模式 (按钮按下时)
    if (digitalRead(MANUAL_OVERRIDE) == LOW) {
      int potValue = analogRead(POT_SPEED);         // 0-1023
      pwmValue = map(potValue, 0, 1023, 0, 255);
      Serial.print(F("[Manual] PWM: "));
    } else {
      // 自动模式: 接收 ESP32 控制信号
      pwmValue = mapControlSignal();
      Serial.print(F("[Auto] PWM: "));
    }

    analogWrite(FAN_EXHAUST_PWM, pwmValue);
    analogWrite(FAN_INTAKE_PWM, pwmValue);  // 或使用不同映射

    Serial.print(pwmValue);
    Serial.print(F(" ("));
    Serial.print((pwmValue / 255.0) * 100, 1);
    Serial.println(F("%)"));
  }
}
