# 🖐️ Gesture-Controlled PC Controller (ESP32S3 + PAJ7620U2)

Control your PC using simple hand gestures — play/pause music, skip tracks, adjust volume, or send arrow key presses, all without touching a button.

This project uses the **Seeed Xiao ESP32S3** and the **PAJ7620U2 gesture sensor**, powered by **CircuitPython**.

## 💸 Get Your ESP32S3 at a Discount

[🛒 Buy the Seeed XIAO ESP32S3 from Seeed Studio](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html?sensecap_affiliate=P9GHEkF&referring_service=link)

Use promo code **`N891LZX6`** at checkout for a discount!

---

[![YouTube Video](https://img.youtube.com/vi/6lv5Pc7rqZE/0.jpg)](https://www.youtube.com/watch?v=6lv5Pc7rqZE)

---

## 🎥 Video Demo + Tutorial

📺 [Watch the full video tutorial here](https://www.youtube.com/watch?v=6lv5Pc7rqZE)

📘 [Learn how to install CircuitPython on the Xiao ESP32S3](https://www.youtube.com/watch?v=InWYwM2DwpM)

---

## 🔧 Hardware Required

| Component                  | Notes                          |
|---------------------------|---------------------------------|
| Seeed Xiao ESP32S3        | With USB HID support            |
| PAJ7620U2 Gesture Sensor  | I²C interface                   |
| Zero PCB / Breadboard     | For wiring                      |
| Jumper Wires              | Connect SDA, SCL, 3.3V, GND     |
| Optional: Acrylic Case    | With cutout for the sensor      |

---

## 🔌 Wiring

| Gesture Sensor Pin | Connect to Xiao ESP32S3 |
|--------------------|-------------------------|
| SDA                | D4                      |
| SCL                | D5                      |
| VCC                | 3.3V                    |
| GND                | GND                     |

---

## 🧰 Libraries & Downloads

To run this project, you'll need the following CircuitPython libraries:

- 📦 **PAJ7620 Gesture Sensor Driver**  
  [github.com/deshipu/circuitpython-paj7620](https://github.com/deshipu/circuitpython-paj7620)

- 📦 **Adafruit CircuitPython Library Bundle** (contains HID & BusDevice)  
  [circuitpython.org/libraries](https://circuitpython.org/libraries)

After downloading:
- Copy `adafruit_hid` and `adafruit_bus_device` folders into the `/lib` directory on your CIRCUITPY drive.
- Copy `paj7620.py` (from the gesture sensor repo) into `/lib` or root.
