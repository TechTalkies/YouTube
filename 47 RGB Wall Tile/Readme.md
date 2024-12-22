# 47 Smart RGB Wall Tile | Arduino Project | With code
This page contains the code and supporting libraries for the below video. 
It is a Tutorial on making an RGD smart wall tile using an Arduino Nano and some NeoPixel LEDs. The tile changes color according to the ambient temperature, sensed using a DHT11 sensor module. The temperature and Humidity is also displayed in real time using a 7 segment display module.

Video link:


[![Youtube Video](https://img.youtube.com/vi/MXuIEUpzcPs/0.jpg)](https://www.youtube.com/watch?v=MXuIEUpzcPs)


# Libraries:

1. LedControl - https://github.com/wayoda/LedControl
2. Adafruit DHT - https://github.com/adafruit/DHT-sensor-library
3. Adafruit_NeoPixel - https://github.com/adafruit/Adafruit_NeoPixel

# Connections:
Connect power pins accordingly

| 	Component   |    Nano       |
| ------------- | ------------- |
|      DHT11 signal    |     D2       |
|      NeoPixel Data       |     D6       |
|      Display DIN       |     D11       |
|      Display CS       |     D12       |
|      Display CLK      |     D13       |
