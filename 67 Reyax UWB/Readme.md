# UWB Distance Measurement with Reyax RYUW122 Lite and ESP32 Nano

This project demonstrates how to use the **Reyax RYUW122 Lite** Ultra-Wideband (UWB) modules to measure distance with centimeter-level accuracy. An ESP32 Nano reads the distance and displays it on a 0.96" I2C OLED screen.

[![YouTube Video](https://img.youtube.com/vi/AEefx4ykJNI/0.jpg)](https://www.youtube.com/watch?v=AEefx4ykJNI)

## Components Used

- Reyax RYUW122 Lite UWB modules (x2)
- Waveshare ESP32 Nano
- 0.96" I2C OLED (SSD1306)
- Jumper wires and breadboard
- USB to Serial adapter (for initial configuration)

---

## Configuration Instructions

### 1. Configure the TAG module

Connect it via USB-to-Serial (3.3V only). Use Arduino Serial Monitor or any terminal software at **115200 baud**, and send the following commands:

AT
AT+MODE? // Should return 0 (TAG)
AT+NETWORKID=100
AT+ADDRESS=TAG1


### 2. Configure the ANCHOR module

Repeat the above process for the second module and run:

AT

AT+MODE=1 // Set to ANCHOR

AT+NETWORKID=100

AT+ADDRESS=ANCH1


> ⚠️ Use the same `NETWORKID` for all modules. Each must have a unique `ADDRESS`.

---

## ESP32 Wiring

| ESP32 Nano Pin | UWB Module | OLED |
|----------------|------------|------|
| D4             | TX         | —    |
| D5             | RX         | —    |
| A4             | —          | SDA  |
| A5             | —          | SCL  |
| 3.3V           | VCC        | VCC  |
| GND            | GND        | GND  |

> **Important:** Do not power UWB modules from 5V. They are 3.3V only.

---

## Upload the Sketch

Use the provided Arduino sketch. The ESP32:

- Sends `AT+ANCHOR_SEND=TAG1,2,HI` every 700 ms
- Parses the distance response from the ANCHOR
- Displays the result in meters on the OLED

If no distance is available, it shows `N/A`.

---

## Expected Output

- OLED shows live distance in meters
- Smooth refresh every ~700ms
- Ideal for short-range tests or desktop demos

---

## Demo

📺 [Watch the YouTube video](https://youtu.be/AEefx4ykJNI)

---

## Notes

- UWB requires **clear line-of-sight** for accurate results.
- Metal surfaces and walls may reflect or block signals.
- Default CPIN is `00000000000000000000000000000000`; all devices must share the same if using CPIN-based encryption.
- Factory default mode is TAG (`MODE=0`)

---

## More Projects

Check out our tutorials and projects at:  
🌐 [https://techtalkies.in](https://techtalkies.in)
