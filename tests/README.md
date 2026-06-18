# 测试目录

## 目录结构

```
tests/
├── hardware/          # 硬件测试
│   ├── sensor-calibration/  # 传感器校准数据
│   ├── pms5003-test/       # PMS5003 功能测试
│   └── sgp30-test/         # SGP30 功能测试
├── firmware/          # 固件测试
│   ├── mock-sensors/       # 模拟传感器输入的单元测试
│   └── fan-pid-test/       # PID 风扇控制测试
└── integration/       # 集成测试
    ├── smoke-test/         # 冒烟测试（上电自检）
    └── 24h-run/            # 24小时稳定性测试
```

## 测试设备清单

- [ ] 第二块 ESP32 (用作串口监听器)
- [ ] USB-TTL 转换器 (模拟 PMS5003 数据)
- [ ] 可调电源
- [ ] 数字万用表
- [ ] 示波器 (可选)

## 快速开始

```bash
# 1. 编译并上传测试固件
cd tests/firmware/mock-sensors
pio run --target upload

# 2. 运行串口测试
python tests/hardware/sensor-calibration/check_readings.py

# 3. 24h 压力测试
python tests/integration/24h-run/run_stress_test.py
```
