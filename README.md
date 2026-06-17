# FDM3D-Enclosure · 3D打印机安全外罩

> **模块化 · 可验证 · 可复现 · 可定制**
> 直击桌面3D打印长期被忽视的健康隐患。

![platform](https://img.shields.io/badge/platform-OpenSource-brightgreen)
![license](https://img.shields.io/badge/license-CC%20BY--SA%204.0-blue)
![status](https://img.shields.io/badge/status-alpha-yellow)

---

## 🌍 为什么需要这个项目？

FDM/树脂桌面3D打印机工作时会释放：
- **超细颗粒物 (UFPs)** — 穿透肺泡进入血液
- **挥发性有机化合物 (VOCs)** — 苯乙烯、甲醛、乙醛等
- **树脂蒸气** — 皮肤/呼吸道刺激物

大部分市售外罩要么是**装饰品**（无过滤），要么是**高昂商用方案**（数千元）。本项目提供一套**低成本、可验证、可复现**的安全替代方案。

---

## 🎯 核心特性

| 特性 | 说明 |
|------|------|
| 🧩 **模块化设计** | 面板/框架/过滤/监测独立，可按需组合 |
| 📡 **实时空气质量监测** | PMS5003 + SGP30 双传感器联动 |
| 🔄 **自适应排风** | 根据实时数据自动调节风扇转速 |
| 📊 **本地仪表盘** | Web界面可视化空气质量历史数据 |
| 🔧 **全部开源** | 3D模型(STL)、固件(Arduino/ESP32)、电路(KiCad)、软件 |
| 💰 **低成本** | 主体材料 $50-80，电子元件 $30-50 |

---

## 📦 项目结构

```
FDM3D-Enclosure/
├── docs/                    # 文档
│   ├── assembly/            # 组装指南
│   ├── bom/                 # 物料清单 (BOM)
│   └── safety/              # 安全与测试报告
├── models/                  # 3D打印模型文件
│   ├── panels/              # 外罩面板
│   ├── frame/               # 框架结构
│   ├── hinges/              # 铰链与连接件
│   └── filters/             # 滤网/滤盒支架
├── hardware/                # 电子硬件
│   ├── schematics/          # 电路原理图
│   ├── pcb/                 # PCB设计文件 (KiCad)
│   └── wiring/              # 接线图
├── firmware/                # 嵌入式固件
│   ├── air-quality-monitor/ # 空气质量监测 (ESP32)
│   └── fan-control/        # 风扇控制 (Arduino)
├── software/                # 上位机软件
│   ├── dashboard/           # Web仪表盘
│   └── data-logger/         # 数据记录器
└── tests/                   # 测试与校准
```

---

## 🚀 快速开始

### 最短路径 (已有打印机)

1. **打印结构件** — `models/` 目录下的STL文件
2. **采购材料** — 参考 `docs/bom/` 中的物料清单
3. **制作面板** — 亚克力/PETG板材按规格切割
4. **搭建电路** — 按 `hardware/wiring/` 接线图连接
5. **刷入固件** — 使用 Arduino IDE / PlatformIO
6. **部署监测** — 连接WiFi，打开仪表盘

### 开发环境

```bash
# 克隆仓库
git clone https://github.com/YOUR_USERNAME/FDM3D-Enclosure.git
cd FDM3D-Enclosure

# 固件 (PlatformIO)
cd firmware/air-quality-monitor
pio run --target upload

# Web仪表盘
cd software/dashboard
npm install
npm run dev
```

---

## 🧪 传感器规格

| 传感器 | 检测项 | 精度 | 接口 |
|--------|--------|------|------|
| PMS5003 | PM1.0 / PM2.5 / PM10 | ±10% | UART |
| SGP30 | TVOC / eCO₂ | ±15% | I²C |
| DHT22 | 温湿度 | ±0.5°C / ±2% | 单总线 |

---

## 🔬 验证方法

本项目的每一项指标都**可独立验证**：

- **颗粒物过滤效率** — 使用低成本粒子计数器 (如 Plantower PMS5003 回读)
- **VOC削减率** — SGP30 传感器前后对比
- **气密性** — 烟雾测试 + 压差测量
- **噪音水平** — 手机分贝计 App 即可

> **我们不承诺未经测试的数据。** 所有测试方法在 `docs/safety/` 中透明公开。

---

## 🤝 参与贡献

欢迎 fork、PR、issue。详见 [CONTRIBUTING.md](CONTRIBUTING.md)。

### 路线图

- [x] 基础框架设计
- [ ] v0.1 — 原型外罩 + 主动排风
- [ ] v0.2 — 空气质量监测上线
- [ ] v0.3 — 自适应PID控制
- [ ] v1.0 — 完整模块化发布

---

## 📄 许可证

本项目采用 **CC BY-SA 4.0** 协议。  
设计文件和固件均可自由修改和再分发，但需保留署名并以相同方式共享。

---

## ⚠️ 免责声明

本项目是社区驱动的安全增强方案，**不构成**职业健康安全建议。  
如有树脂/化学品暴露疑虑，请咨询工业卫生专家。作者不对因使用本项目造成的任何损害承担责任。

---

> *"你的下一台外罩，不应该靠猜。"*
