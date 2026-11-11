# Home Automation

This repository contains Arduino/ESP8266/ESP32 projects for home automation — including sensor nodes, light control, motion detection, smoke detection, and temperature monitoring.

It supports the following microcontrollers:
- **ESP8266** (NodeMCU, Wemos D1 Mini, etc.)
- **Seeed XIAO ESP32S3**

Projects communicate over WiFi using MQTT (via PubSubClient) for messaging between nodes and a broker (e.g. Mosquitto on a Raspberry Pi).

---

## Repository Structure

```
HomeAutomation/        # Main sketch combining all sensors (motion, smoke, temperature)
libraries/             # Required Arduino libraries (see below)
```

---

## Hardware Requirements

- ESP8266 or Seeed XIAO ESP32S3 microcontroller
- DHT11 or DHT22 temperature/humidity sensor
- PIR motion sensor
- MQ-2 or MQ-135 smoke/gas sensor
- LDR light sensor
- WiFi network with MQTT broker (e.g. Mosquitto)

---

## Libraries

| Library                     | Version | Purpose                                          |
|-----------------------------|---------|--------------------------------------------------|
| DHT sensor library          | 1.4.7   | DHT11/DHT22 temperature & humidity (Arduino/AVR) |
| DHT sensor library for ESPx | 1.19    | DHT11/DHT22 optimized for ESP8266/ESP32          |
| Adafruit Unified Sensor     | 1.1.15  | Dependency for DHT sensor library                |
| PubSubClient                | 2.8     | MQTT client for WiFi messaging                   |
| rc-switch                   | 2.6.4   | 433 MHz RF transmit/receive (wireless switches)  |

---

## Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add the ESP8266 board package:
   - Go to **File → Preferences → Additional Board Manager URLs** and add:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Then **Tools → Board → Boards Manager**, search `esp8266`, install **ESP8266 by ESP8266 Community**
3. For XIAO ESP32S3, add the Seeed board package URL:
   ```
   https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
   ```
4. Install all libraries listed above via **Tools → Manage Libraries**
5. Configure your network credentials and MQTT broker address in `networkId.h`
6. Open the target sketch (e.g. `HomeAutomation.ino`) and upload

---

## Contribution Guidelines

- Branch from `main`, commit to a feature branch, open a PR — never commit directly to `main`
- Each project folder is self-contained with its own `.ino` and sensor modules
