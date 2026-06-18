# FDM3D-Enclosure — 3D Printer Safety Enclosure

> **Modular · Verifiable · Reproducible · Customizable**
> Addressing the overlooked health risks of desktop 3D printing.

![platform](https://img.shields.io/badge/platform-OpenSource-brightgreen)
![license](https://img.shields.io/badge/license-CC%20BY--SA%204.0-blue)
![status](https://img.shields.io/badge/status-alpha-yellow)
![CI](https://github.com/mrcyy/FDM3D-enclosure/actions/workflows/ci.yml/badge.svg)

---

## Why This Project?

FDM/resin desktop 3D printers emit:
- **Ultrafine Particles (UFPs)** — penetrate lung alveoli into bloodstream
- **Volatile Organic Compounds (VOCs)** — styrene, formaldehyde, acetaldehyde
- **Resin Vapors** — skin/respiratory irritants

Most commercial enclosures are either decorative (no filtration) or expensive ($500+). This project provides a **low-cost, verifiable, reproducible** safety solution.

---

## Core Features

| Feature | Description |
|---------|-------------|
| 🧩 **Modular Design** | Panels/frame/filtration/sensing are independent & combinable |
| 🌬️ **Real-time Air Monitoring** | PMS5003 + SGP30 dual sensor array |
| 🔄 **Adaptive Ventilation** | Auto-adjusts fan speed based on live sensor data |
| 📊 **Local Dashboard** | Web UI with real-time charts & history |
| 🔓 **Fully Open Source** | STL models, firmware (ESP32/Arduino), electronics, software |
| 💰 **Low Cost** | Frame ~$50-80, electronics ~$30-50 |

---

## Project Structure

```
FDM3D-Enclosure/
├── docs/                    # Documentation
│   ├── assembly/            # Assembly guide
│   ├── bom/                 # Bill of Materials
│   └── safety/              # Testing & safety protocols
├── models/                  # 3D printable STL files
│   ├── panels/              # Enclosure panels
│   ├── frame/               # Frame connectors & feet
│   ├── hinges/              # Hinges & latches
│   └── filters/             # Filter housings & fan mounts
├── hardware/                # Electronics
│   ├── schematics/          # Circuit schematics
│   ├── pcb/                 # KiCad PCB designs
│   └── wiring/              # Wiring diagrams
├── firmware/                # Embedded firmware
│   ├── air-quality-monitor/ # Air quality sensor (ESP32)
│   └── fan-control/        # Fan controller (Arduino)
├── software/                # PC software
│   ├── dashboard/           # Web-based dashboard (Node.js)
│   └── data-logger/         # Data logger (optional)
└── tests/                   # Test scripts & data
```

---

## Quick Start

### Shortest Path (Have a printer)

1. **Print** — STL files from `models/`
2. **Source** — BOM from `docs/bom/`
3. **Cut Panels** — Acrylic/PETG to spec
4. **Wire** — Follow `hardware/wiring/` diagrams
5. **Flash** — Upload firmware via PlatformIO
6. **Monitor** — Connect WiFi, open dashboard

### Development

```bash
# Clone
git clone https://github.com/mrcyy/FDM3D-Enclosure.git
cd FDM3D-Enclosure

# Firmware (PlatformIO)
cd firmware/air-quality-monitor
pio run --target upload

# Dashboard
cd software/dashboard
npm install
npm start
```

---

## Sensor Specs

| Sensor | Measures | Accuracy | Interface |
|--------|----------|----------|-----------|
| PMS5003 | PM1.0 / PM2.5 / PM10 | ±10% | UART |
| SGP30 | TVOC / eCO₂ | ±15% | I²C |
| DHT22 | Temp / Humidity | ±0.5°C / ±2% | 1-Wire |

---

## Verification

Every metric is **independently verifiable**:

- **Particulate efficiency** — loop-back PMS5003 comparison
- **VOC reduction** — SGP30 before/after filtration
- **Seal integrity** — smoke test & differential pressure measurement
- **Noise level** — phone decibel meter app

> **We don't claim unverified data.** All test methods are documented transparently in `docs/safety/`.

---

## Roadmap

- [x] Basic frame design
- [ ] v0.1 — Prototype enclosure + active ventilation
- [ ] v0.2 — Air quality monitoring live
- [ ] v0.3 — Adaptive PID control
- [ ] v1.0 — Full modular release

---

## License

**CC BY-SA 4.0** — Free to modify and redistribute with attribution and share-alike.

---

## Disclaimer

This is a community-driven safety enhancement project. It does **not** constitute professional occupational health & safety advice. Consult an industrial hygienist for resin/chemical exposure concerns. The authors assume no liability for damages arising from use of this project.

---

> *"Your next enclosure shouldn't be a guessing game."*
