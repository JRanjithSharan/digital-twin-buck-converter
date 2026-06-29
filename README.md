# ESP32-Based Digital Twin for an LM2596 DC-DC Buck Converter

> A lightweight, real-time Digital Twin implemented entirely on an ESP32 for health monitoring, anomaly detection, and predictive maintenance of an LM2596 DC-DC Buck Converter.

![ESP32](https://img.shields.io/badge/Platform-ESP32-blue)
![Language](https://img.shields.io/badge/Language-Arduino%20C++-orange)
![IoT](https://img.shields.io/badge/Cloud-ThingSpeak-green)
![Status](https://img.shields.io/badge/Status-Completed-success)

---

## 📖 Overview

Digital Twins have become an essential technology for predictive maintenance and intelligent monitoring in modern engineering systems. However, most Digital Twin implementations rely on cloud computing or high-performance hardware, making them unsuitable for low-cost embedded platforms.

This project demonstrates a **resource-constrained Digital Twin** running entirely on an **ESP32 microcontroller** for an **LM2596 DC-DC Buck Converter**. The system continuously acquires real-time voltage and current measurements, predicts converter behavior using a lightweight mathematical model, estimates converter health, detects anomalies through residual analysis, and uploads diagnostics to the ThingSpeak cloud platform for remote monitoring.

Unlike conventional implementations, all Digital Twin computations are performed locally on the ESP32 without requiring external computation.

---

## ✨ Features

- Real-time voltage and current sensing
- Embedded Digital Twin running entirely on ESP32
- Lightweight mathematical converter model
- Residual-based fault detection
- Adaptive nominal voltage estimation
- Internal resistance estimation for health monitoring
- Automatic ACS712 sensor calibration
- Multi-sample ADC noise filtering
- ThingSpeak cloud integration
- Real-time remote monitoring
- Edge computing architecture

---

# 🏗 System Architecture

```
                    +----------------------+
                    |  LM2596 Buck Converter |
                    +-----------+----------+
                                |
                     Vin, Vout, Iout
                                |
                                ▼
                       +----------------+
                       |     ESP32      |
                       +----------------+
                                |
          --------------------------------------------
          |        Embedded Digital Twin             |
          |------------------------------------------|
          | Predict Output Voltage                  |
          | Estimate Internal Resistance            |
          | Residual Computation                    |
          | Fault Detection                         |
          --------------------------------------------
                                |
                                ▼
                        ThingSpeak Cloud
```

---

# 🔧 Hardware Components

| Component | Purpose |
|-----------|---------|
| ESP32-WROOM-32 | Embedded Digital Twin |
| LM2596 Buck Converter | Physical system under observation |
| ACS712-20A Current Sensor | Output current measurement |
| Voltage Divider Network | Input/Output voltage sensing |
| 12V Li-ion Battery | Power source |
| 5V DC Gear Motor | Dynamic load |

---

# 💻 Software Stack

- Arduino IDE
- Embedded C++
- ESP32 Wi-Fi Library
- ThingSpeak API

---

# ⚙️ Working Principle

The Digital Twin operates using the following workflow:

1. Measure converter input voltage, output voltage, and output current.
2. Perform sensor calibration and noise filtering.
3. Estimate the expected converter output using a mathematical model.
4. Compute the residual between predicted and measured output.
5. Estimate converter internal resistance.
6. Detect abnormal operating conditions using residual thresholds.
7. Upload measurements and diagnostics to ThingSpeak.

---

# 🧠 Digital Twin Algorithms

## Adaptive Calibration

The ESP32 automatically calibrates the ACS712 current sensor during startup to compensate for offset drift.

---

## Noise Filtering

Each ADC channel performs multi-sample averaging to improve measurement stability.

---

## Residual-Based Fault Detection

The Digital Twin predicts the expected converter output and compares it with the measured output.

```
Residual = Vmeasured − Vpredicted
```

Persistent residuals above the threshold indicate abnormal converter behavior.

---

## Internal Resistance Estimation

The converter's effective internal resistance is continuously estimated to monitor long-term degradation.

---

# 📊 Experimental Validation

The system was evaluated under three operating conditions.

### Healthy Operation

- Stable converter output
- Near-zero residual
- Accurate Digital Twin prediction

---

### Normal Load Operation

- Tested using a 5V DC geared motor
- Digital Twin accurately tracked voltage droop
- Stable internal resistance estimation

---

### Fault Conditions

The Digital Twin successfully detected:

- Battery voltage sag
- Loose wiring
- Connector disturbances
- Dynamic load changes

Residual spikes triggered confirmed fault warnings after debounce verification.

---

# ☁️ ThingSpeak Dashboard

The ESP32 uploads the following parameters every 15 seconds:

- Input Voltage
- Output Voltage
- Predicted Voltage
- Residual
- Output Current
- Output Power

The dashboard provides real-time visualization of converter health and system performance.

---

# 🚀 Future Improvements

- TinyML-based fault classification
- Thermal monitoring
- Converter efficiency estimation
- Adaptive health learning
- MQTT integration
- Multi-node Digital Twin architecture
- Web-based analytics dashboard

---

# 📄 Documentation

The complete project report is available in the `docs/` directory.

---

# 👨‍💻 Author

**Ranjith Sharan J**

B.Tech Electronics & Instrumentation Engineering  
Vellore Institute of Technology

- LinkedIn: https://linkedin.com/in/your-profile
- GitHub: https://github.com/JRanjithSharan

---
