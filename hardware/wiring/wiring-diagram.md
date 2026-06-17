# 接线图

## 系统架构

```
┌────────────────────────────────────────────┐
│              12V / 3A 电源                  │
│         (AC-DC 开关电源模块)                 │
└────────────────┬───────────────────────────┘
                 │
          ┌──────┴──────┐
          │  LM2596 DC-DC │  (12V → 5V/3.3V)
          │  降压模块     │
          └──────┬──────┘
                 │ 5V
          ┌──────┴──────────┐
          │    ESP32        │
          │  DevKitC        │
          └──────┬──────────┘
                 │
     ┌───────────┼───────────────┐
     │           │               │
     ▼           ▼               ▼
  PMS5003     SGP30 + DHT22   OLED 0.96"
  (UART)       (I²C)          (I²C)
     │           │               │
     └───────────┴───────────────┘
                 │
         ┌───────┴───────┐
         │  Arduino Nano │  (协处理器，可选)
         │  (风扇控制)    │
         └───────┬───────┘
                 │
     ┌───────────┴───────────┐
     │                       │
     ▼                       ▼
  排风扇 12038          进气风扇 12025
  (12V PWM)              (12V PWM)
```

## 详细接线

### 电源部分

| 模块 | 引脚 | 连接目标 | 线色建议 |
|------|------|----------|----------|
| 12V电源 V+ | OUT+ | LM2596 IN+ | 红色 |
| 12V电源 V- | OUT- | LM2596 IN- | 黑色 |
| LM2596 OUT+ | VOUT | ESP32 5V引脚 | 红色 |
| LM2596 OUT- | GND | ESP32 GND | 黑色 |
| 12V电源 V+ | OUT+ | 排风扇 VCC (通过MOSFET) | 红色 |
| 12V电源 V- | OUT- | 风扇 GND | 黑色 |

### 传感器部分 (ESP32)

| ESP32引脚 | 传感器 | 传感器引脚 | 备注 |
|-----------|--------|-----------|------|
| 3.3V | PMS5003 | VCC (pin 1) | 5V也可 |
| GND | PMS5003 | GND (pin 2) | |
| GPIO16 (RX2) | PMS5003 | TX (pin 4) | UART接收 |
| GPIO17 (TX2) | PMS5003 | RX (pin 5) | 非必须 |
| | PMS5003 | SET (pin 3) | 浮空 |
| 3.3V | SGP30 | VIN | |
| GND | SGP30 | GND | |
| GPIO21 | SGP30 | SDA | I²C |
| GPIO22 | SGP30 | SCL | I²C |
| 5V | DHT22 | VCC | |
| GND | DHT22 | GND | |
| GPIO4 | DHT22 | DATA | 需4.7kΩ上拉 |
| 3.3V | OLED | VCC | |
| GND | OLED | GND | |
| GPIO5 | OLED | SDA | 软件I²C |
| GPIO18 | OLED | SCL | |

### 风扇控制 (Arduino Nano 或直接 PWM)

**方案A: ESP32 直驱 (简单版)**
```
ESP32 GPIO26 ──→ MOSFET栅极 (IRLZ44N) ──→ 排风扇
风扇 VCC ──→ 12V
风扇 GND ──→ MOSFET漏极
MOSFET源极 ──→ GND
```

**方案B: Arduino 协处理器 (推荐，独立控制)**
```
ESP32 GPIO26 ──→ Arduino A0     (控制信号)
Arduino D9    ──→ MOSFET栅极    (排风扇PWM)
Arduino D10   ──→ MOSFET栅极    (进气风扇PWM)
```

## MOSFET 驱动电路

```
        12V
         │
      ┌──┴──┐
      │ 风扇 │
      └──┬──┘
         │ (风扇负极)
         │
       ┌─┴─┐
  PWM ─┤ G │
       │   │  IRLZ44N
      S┌─┴─┐D
        │
       GND
```

> **重要:** 12V风扇必须通过MOSFET开关，**不可**直接接GPIO引脚！
