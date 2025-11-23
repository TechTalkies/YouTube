# RF Remote Control for Arduino Projects (YK04 Module)

This project shows how to easily add **long-range wireless remote control** to Arduino projects using the **YK04 RF receiver + remote kit**.  
No Wi-Fi, no pairing, no libraries. Just wire it and start using.

---

Click on image to watch video:

[![YouTube Video](https://img.youtube.com/vi/ZQ8YeK9cHrs/0.jpg)](https://www.youtube.com/watch?v=ZQ8YeK9cHrs)

---

## 📌 Features

- Works **with or without Arduino**
- LED control with **mode + brightness**
- Servo motor rotation (left / right)
- Up to **100m range** (open space)
- Can be used as a game controller input
- Optional relay control for lights/motors
- Bonus: Flappy Bird game controlled via RF button

---

## 🧰 Components Used

| Component | Notes |
|-----------|-------|
| YK04 RF Receiver | 314 MHz |
| RF Remote | A/B/C/D buttons, A27 12 V battery |
| Arduino Uno / Nano | Any 5 V board works |
| LED + Resistor | For demo |
| Servo Motor (SG90) | For movement example |
| Relay Module (optional) | For AC control (use isolated type) |

---

## 🔌 Wiring (Arduino Example)

| RF Module Pin | Arduino Pin |
|---------------|-------------|
| VCC | 5 V |
| GND | GND |
| D0 | A1 |
| D1 | A3 |
| D2 | A4 |
| D3 | A5 *(optional)* |
| VT | *(optional – can be used as interrupt)* |

---

## 🎮 Button Functions

| Button | Action |
|--------|--------|
| **A** | LED On / Off |
| **B** | Switch LED Mode (Static ↔ Blink) |
| **C** | Adjust Brightness |
| **D** | Not used (reserved) |

🔁 Brightness cycles 0 → 255 → 0  
🔁 Mode toggles every press (static/blink)
