# Viewing 3D models using a Webserver on ESP8266
This page contains the code and supporting libraries for the below video. 
It is a video demonstrating how to load 3D models and view them on a webserver running on an ESP8266 board, using an MPU 6050 gyroscope and accelerometer sensor. 

This project does not need the LittleFS library. You can load your 3D models directly from your device.

Video link:
[![Youtube Video](https://img.youtube.com/vi/08F5hAk-7Qk/0.jpg)](https://www.youtube.com/watch?v=08F5hAk-7Qk)


# Libraries:

1. Arduino JSON - http://github.com/arduino-libraries/Arduino_JSON
2. AsyncTCP - https://github.com/dvarrel/AsyncTCP
3. ESPAsyncWebServer - https://github.com/mathieucarbou/ESPAsyncWebServer
4. Adafruit MPU6050 - https://github.com/adafruit/Adafruit_MPU6050
5. Adafruit Sensor - https://github.com/adafruit/Adafruit_Sensor

# Connections (SPI pins):

1. SCL -> D1
2. SDA -> D2
3. Connect power pins accordingly

# Credits (3D models):
1. Jet by Poly by Google [CC-BY] (https://creativecommons.org/licenses/by/3.0/) via Poly Pizza (https://poly.pizza/m/3B3Pa6BHXn1)
2. Helicopter by jeremy [CC-BY] (https://creativecommons.org/licenses/by/3.0/) via Poly Pizza (https://poly.pizza/m/eb7b31pjGtQ)
3. Katana Sword by Nick Ladd [CC-BY] (https://creativecommons.org/licenses/by/3.0/) via Poly Pizza (https://poly.pizza/m/7Ly4_Ou8HXK)
