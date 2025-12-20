# Wi-Fi Traffic Visualizer with ESP32 (Promiscuous Mode Sniffer)

Visualize nearby Wi-Fi activity in real time using an ESP32 microcontroller and an OLED display — **no Wi-Fi connection required**. This project uses *promiscuous mode* to passively detect and count wireless packets from nearby networks and devices.

Ideal for network tinkerers, security hobbyists, or just curious tech folks who want to see the invisible chatter of Wi-Fi around them.

[![YouTube Video](https://img.youtube.com/vi/qju-e4JQrVQ/0.jpg)](https://www.youtube.com/watch?v=qju-e4JQrVQ)

---

## 📡 How It Works

- The ESP32 is set to **promiscuous mode**, allowing it to listen to all 2.4GHz Wi-Fi traffic.
- It counts packets and displays a basic activity graph on a **1.3" OLED screen**.
- No need to connect to any Wi-Fi network.
- Completely passive. Just power it up and start watching the traffic spike!

---

## 🔌 Connections (ESP32 + OLED SSD1306 128x64)

| ESP32 Pin | OLED Pin | Function      |
| --------- | -------- | ------------- |
| 3.3V      | VCC      | Power         |
| GND       | GND      | Ground        |
| GPIO22    | SCL      | I2C Clock     |
| GPIO21    | SDA      | I2C Data      |

*You can change the I2C pins in code depending on your ESP32 board.*

---

## 📚 Libraries Used

- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)

Install them via the Arduino Library Manager.

---

## ⚠️ Notes

- This only works on **2.4GHz Wi-Fi** (ESP32 can't sniff 5GHz).
- The ESP32 must support promiscuous mode (most dev boards do).
- Visual display is minimal and focused on traffic volume, not deep packet inspection.

---

## 🧠 Ideas to Expand

- Add SSID or MAC logging (basic packet parsing)
- Log to SD card
- Send data to a web dashboard via ESP32 Wi-Fi (if needed)

---

## 📺 Watch the Full Video

Want to see how it works and how to build it?  
👉 [Watch on YouTube](https://www.youtube.com/watch?v=wB0XThfc2Rk)

