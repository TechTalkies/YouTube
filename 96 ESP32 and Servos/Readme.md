# ESP32 Servo Control Tutorial

This project demonstrates multiple ways to control **servo motors using an ESP32**.  
It starts with basic servo control and gradually builds up to controlling **multiple servos using the PCA9685 I2C driver and a web interface**.

The final example allows controlling **5 servos from a web page using sliders**, with options to **lock all servos together or reset them to zero**.

---

## Features

- Control a **single servo using ESP32**
- Control **multiple servos directly from ESP32**
- Control servo position using a **potentiometer**
- Control **up to 16 servos using the PCA9685 I2C driver**
- **Web-based control interface**
- ESP32 runs in **WiFi Access Point mode**
- **5 servo sliders**
- **Lock mode** to move all servos together
- **Reset button** to return all servos to 0°

---

## Hardware Required

- ESP32 development board  
- Servo motors (SG90 or similar)  
- PCA9685 Servo Driver Module  
- Potentiometer (10k recommended)  
- External 5V power supply (recommended for multiple servos)  
- Jumper wires  

---

## Wiring

### PCA9685 to ESP32

| PCA9685 | ESP32 |
|--------|-------|
| VCC | 3.3V |
| GND | GND |
| SDA | SDA |
| SCL | SCL |

### Servo Connections

Servos connect directly to the **PCA9685 outputs**.

| Servo Wire | Connection |
|------------|------------|
| Brown / Black | GND |
| Red | 5V External Supply |
| Yellow / Orange | PCA9685 Signal Pin |

⚠️ When using multiple servos, **do not power them from the ESP32**.  
Use an **external 5V supply** capable of delivering sufficient current.

---

## Installing Required Libraries

Install the following library in Arduino IDE:

```
Adafruit PWM Servo Driver Library
ESP32Servo
```

This will also install the required **Adafruit BusIO** dependency.

---

## Running the webserver

1. Upload the code to the ESP32.
2. Power the ESP32 and connect to the WiFi network.

```
SSID: ESP32_SERVO
Password: 12345678
```

3. Open a browser and navigate to:

```
192.168.4.1
```

You will see the **servo control interface**.

---

## Web Interface

The web page contains:

- **5 sliders** for individual servo control
- **Lock button**
  - Moves all servos together
  - Sets all servos to 0°
- **Reset button**
  - Returns all servos to 0°

---

## Applications

This setup can be used in projects such as:

- Robotic arms
- Pan-tilt camera mounts
- Animatronics
- Remote robotics control
- Educational robotics projects

---

## License

This project is released under the MIT License.
