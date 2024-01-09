# Complete project details at https://RandomNerdTutorials.com/raspberry-pi-pico-dht11-dht22-micropython/

from machine import Pin, I2C
from time import sleep
import dht
from ssd1306 import SSD1306_I2C

WIDTH =128 
HEIGHT= 64

i2c = I2C(1, sda=Pin(14), scl=Pin(15), freq=400000)
oled = SSD1306_I2C(WIDTH,HEIGHT,i2c)

#sensor = dht.DHT22(Pin(22))
sensor = dht.DHT11(Pin(1))

while True:
  try:
    sleep(2)
    sensor.measure()
    temp = sensor.temperature()
    hum = sensor.humidity()
    temp_f = temp * (9/5) + 32.0
    print('Temperature: %3.1f C' %temp)
    print('Temperature: %3.1f F' %temp_f)
    print('Humidity: %3.1f %%' %hum)
    print('\n')
    
    oled.fill(0)
    oled.text("Tech Talkies", 10, 0)
    oled.text(str(hum), 13, 20)
    oled.show()
    
    
    
  except OSError as e:
    print('Failed to read sensor.')