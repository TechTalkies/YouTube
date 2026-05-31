# ESP32-S3 USB Host — ST7735 Display Demos

A collection of USB Host demos for the **ESP32-S3** using the [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost) library, paired with a 160×128 ST7735 TFT display.
Each demo showcases a different USB HID or MSC use case with live visual feedback on the display.

---

Watch here:

[![Youtube Video](https://img.youtube.com/vi/dgBH8Cb6Fz4/0.jpg)](https://www.youtube.com/watch?v=dgBH8Cb6Fz4)
---

## Hardware

| Component | Details |
|-----------|---------|
| MCU | ESP32-S3 (USB Host capable) |
| Display | ST7735 TFT — 160×128px |
| Interface | SPI |

### Wiring

| Signal | GPIO |
|--------|------|
| SCK | 11 |
| MOSI | 10 |
| DC | 9 |
| RST | 8 |
| CS | 7 |

> **Note:** These are the connections made in the video. Can be customised and might differ for different boards.

---

## Dependencies

- [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost)
- [Adafruit ST7735 Library](https://github.com/adafruit/Adafruit-ST7735-Library)
- [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)

Install via Arduino Library Manager or add as ZIP.

---

## Demos

### `MouseTest`
Connects a USB HID mouse and renders a live cursor on the ST7735 display. Demonstrates pointer tracking with screen boundary clamping.

### `KeyboardTest`
Reads input from a USB HID keyboard and displays key presses on screen in real time.

### `DualTest`
Runs both mouse and keyboard simultaneously.

### `MassStorageTest`
Mounts a USB flash drive (FAT) and displays the root directory contents in a Windows 95-style file explorer UI. Click with a USB mouse to navigate folders and preview text files.


### `Firmware`
Main production firmware combining all features.

---

## Getting Started

1. Clone this repo
2. Open any demo folder in Arduino IDE
3. Select **ESP32S3 Dev Module** as the board
4. Wire up the ST7735 display per the table above
5. Upload
6. Firmware is flashed directly as a bin file

   
> **Note:** USB Host requires the ESP32-S3's native USB port (not the UART bridge port). Make sure your board exposes the D+/D− pins.

---

## License

MIT
