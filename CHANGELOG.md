# 更新日志

## [0.1.0] - 2026-06-17

### Added
- 项目创建
- README 中文版 (项目理念、结构、快速开始)
- 文档: 概览、组装指南、物料清单、测试协议
- 固件: ESP32 空气质量监测器 (PMS5003 + SGP30 + DHT22 + OLED)
- 固件: Arduino 风扇控制器
- 软件: Node.js WebSocket 仪表盘 (Chart.js 实时图表)
- 硬件: 接线图、电路原理说明
- 配置文件: .gitignore, LICENSE (CC BY-SA 4.0)

## [0.1.1] - 2026-06-18

### Fixed
- 🐛 **OLED I2C 冲突** — OLED 改用硬件 I2C (GPIO21/22, 与 SGP30 同总线), 移除冲突的软件 I²C 引脚定义
- 🐛 **analogWrite → LEDC** — ESP32 不支持 analogWrite(), 改用 ledcSetup/ledcAttachPin/ledcWrite, 支持 25kHz 静音PWM
- 🐛 **阻塞式蜂鸣** — delay(200) 改为非阻塞状态机, 不影响传感器读取循环
- 🐛 **硬编码 WiFi** — 接入 WiFiManager, 首次运行自动进入配网门户, 无需硬编码 SSID/密码
- 🐛 **createJSON 风扇值** — 修复 analogRead(FAN_CTRL_PIN) → 直接读取 fanPwmValue 变量
- 🐛 **isnan 保护** — 温湿度传感器读取可能返回 NaN, 增加 isnan 检查防止序列化异常

### Added
- ✨ **CI/CD** — GitHub Actions 自动构建: ESP32 固件 + Arduino 固件 + Dashboard 语法检查
- 📘 **英文 README** — `README_EN.md` 完整英文版
- 🗂️ **3D模型目录结构** — `models/frame/`, `panels/`, `hinges/`, `filters/` 增加 README 说明
- 🧪 **测试目录** — `tests/README.md` 测试框架说明和计划
- 📡 **Dashboard v0.2** — 增加: 健康检查API (/api/health), CSV导出 (/api/export/csv), 串口热插拔重连, 手动指定端口, 演示模式自动切换, 数据点多重坐标轴, PM2.5 AQI 颜色指示条, 图例开关控制
- 📄 **Dashboard** — chart.js CDN fallback (CDN失效时自动降级)

### Changed
- 📝 **config.h** — WiFi 配置改为 WiFiManager 优先, OLED 引脚合并到硬件 I2C, 增加 LEDC 配置
- 📝 **README.md** — 增加 CI 徽章, 更新版本号
