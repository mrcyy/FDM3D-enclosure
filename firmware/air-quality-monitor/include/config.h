#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// 空气质量监测器 - 配置
// ============================================

// WiFi 配置 (通过 WiFiManager 可在首次运行时网页配置)
#define WIFI_SSID           "YourWiFiSSID"
#define WIFI_PASSWORD       "YourWiFiPassword"

// PMS5003 串口 (UART2)
#define PMS_RX_PIN          16
#define PMS_TX_PIN          17

// I2C 引脚
#define SDA_PIN             21
#define SCL_PIN             22

// DHT22
#define DHT_PIN             4

// OLED 显示
#define OLED_ADDR           0x3C
#define OLED_SDA            5
#define OLED_SCL            18

// 蜂鸣器
#define BUZZER_PIN          13

// 报警阈值
#define ALERT_PM25          100    // µg/m³
#define ALERT_TVOC          500    // ppb
#define ALERT_ECO2          1000   // ppm

// 数据上报间隔 (ms)
#define REPORT_INTERVAL     5000

// 串口命令控制引脚 (接 Arduino 风扇控制器)
#define FAN_CTRL_PIN        26     // PWM 输出

#endif
