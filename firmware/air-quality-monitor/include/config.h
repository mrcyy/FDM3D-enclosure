#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// 空气质量监测器 - 配置
// ============================================

// WiFi 配置 — 通过 WiFiManager 首次运行时网页配置
// 如需硬编码可取消注释，否则将在首次启动自动进入配网模式
// #define WIFI_SSID           "YourWiFiSSID"
// #define WIFI_PASSWORD       "YourWiFiPassword"

// PMS5003 串口 (UART2)
#define PMS_RX_PIN          16
#define PMS_TX_PIN          17

// I2C 引脚
#define SDA_PIN             21
#define SCL_PIN             22

// DHT22
#define DHT_PIN             4

// OLED 显示 (复用硬件 I2C 引脚 GPIO21/22, 与 SGP30 同总线)
#define OLED_ADDR           0x3C

// 蜂鸣器
#define BUZZER_PIN          13

// 报警阈值
#define ALERT_PM25          100    // µg/m³
#define ALERT_TVOC          500    // ppb
#define ALERT_ECO2          1000   // ppm

// 数据上报间隔 (ms)
#define REPORT_INTERVAL     5000

// 风扇 PWM 控制 (使用 LEDC, 详见 main.cpp)
#define FAN_CTRL_PIN        26
#define FAN_PWM_CHANNEL     0       // LEDC 通道
#define FAN_PWM_FREQ        25000   // Hz, 超出人耳范围
#define FAN_PWM_RESOLUTION  8       // bits (0-255)

#endif
