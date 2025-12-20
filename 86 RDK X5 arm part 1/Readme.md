# Voice Controlled Robot Arm on RDK X5

This project demonstrates a **fully offline, voice-controlled robot arm**
built using the **D-Robotics RDK X5** single board computer.

The system uses **Vosk** for offline speech recognition and a **PCA9685 I2C
servo driver** for smooth, jitter-free control of multiple servos.

No cloud services are used.
No Arduino or external microcontroller is required.

## Video link:

[![YouTube Video](https://img.youtube.com/vi/qju-e4JQrVQ/0.jpg)](https://www.youtube.com/watch?v=qju-e4JQrVQ)

---

## Features

- Offline speech recognition using Vosk
- Smooth servo motion with linear easing
- Natural voice commands (left, right, up, down, open, close)
- PCA9685-based servo control over I2C
- Designed to run fully on the RDK X5

---

## Hardware Used

- D-Robotics RDK X5
- PCA9685 16-channel servo driver
- SG90 / MG90S servos
- 3D printed arm
- USB microphone
- External 5V power supply for servos

---

## Software Stack

- Ubuntu (RDK X5 official image)
- Python 3
- Vosk (offline speech recognition)
- sounddevice
- PCA9685_smbus2

---

## Voice Commands (Part 1)

Supported commands:
- `left`
- `right`
- `up`
- `down`
- `forward`
- `backward`
- `open`
- `close`
- `reset`

Each command moves the arm **incrementally and smoothly** for intuitive control.

---

## Running the Project

1. Install dependencies:
   ```bash
   pip3 install vosk sounddevice PCA9685_smbus2
2. Download a Vosk model:
https://alphacephei.com/vosk/models
Update the model path in the script.

3. Run:
   ```bash
   python3 voice_arm.py

## Links
- RDK X5: https://developer.d-robotics.cc/en/rdkx5
- Vosk: https://alphacephei.com/vosk/
- PCA9685 driver: https://github.com/sigmondkukla/Python_PCA9685
- X5 software: https://archive.d-robotics.cc/
- 3D print: https://www.printables.com/model/449747-brazo-robotico-robotic-arm


## Notes

- Servos must be powered from an external 5V supply
- GND must be shared between the X5 and the servo power supply
- PCA9685 is used to avoid GPIO/PWM timing issues on the SBC

## Next Steps (Part 2)
- Add CSI camera support
- Run real-time object detection on the X5
- Combine voice + vision for object-aware control
