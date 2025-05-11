# How to Control Any Motor with Arduino (DC, Servo, Stepper)

Learn how to control DC motors, servo motors, and stepper motors using Arduino! This video covers the basics of each motor type, how they work, and which motor drivers you need — including the L298N, TB6612FNG, ULN2003, DRV8833 and more.

Whether you're building a robot, automation system, or just starting with electronics, this guide gives you the foundation to control motors properly.
Video link:


[![Youtube Video](https://img.youtube.com/vi/jRvX1Un0ixM/0.jpg)](https://www.youtube.com/watch?v=jRvX1Un0ixM)


## Connections
## Stepper Motor (28BYJ-48 with ULN2003)
| Arduino Pin |        ULN2003 | Function             |
| ----------- | -------------- | -------------------- |
| D8          | IN1            | Stepper coil control |
| D10         | IN2            | Stepper coil control |
| D9          | IN3            | Stepper coil control |
| D11         | IN4            | Stepper coil control |

## L298N / TB6612FNG Wiring
| Arduino Pin | Driver Pin | Function            |
| ----------- | ---------- | ------------------- |
| D8          | IN1A       | Motor A control     |
| D9          | IN2A       | Motor A control     |
| D10 (PWM)   | ENA        | Motor A speed (PWM) |
| D6          | IN1B       | Motor B control     |
| D7          | IN2B       | Motor B control     |
| D5 (PWM)    | ENB        | Motor B speed (PWM) |

## DRV8833 Motor Driver Wiring
| Arduino Pin | DRV8833 Pin | Function        |
| ----------- | ----------- | --------------- |
| D9          | IN1         | Motor A control |
| D10         | IN2         | Motor A control |
| D6          | IN3         | Motor B control |
| D5          | IN4         | Motor B control |

## Libraries Used
- [Servo library documentation](https://docs.arduino.cc/libraries/servo/)
- [Stepper library documentation](https://docs.arduino.cc/libraries/stepper/)
