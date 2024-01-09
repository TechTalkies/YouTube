from machine import Pin, I2C
from ssd1306 import SSD1306_I2C
from utime import sleep
import sys

WIDTH =128 
HEIGHT= 64
TOGGLE = True

i2c=I2C(1, sda=Pin(14), scl=Pin(15), freq=400000)

i2c_addr = [hex(ii) for ii in i2c.scan()] # get I2C address in hex format
if i2c_addr==[]:
    print('No I2C Display Found') 
    sys.exit() # exit routine if no dev found
else:
    print("I2C Address      : {}".format(i2c_addr[0])) # I2C device address
    print("I2C Configuration: {}".format(i2c)) # print I2C params
    
oled = SSD1306_I2C(WIDTH,HEIGHT,i2c)

while True:
    oled.fill(0)
    oled.text("Tech Talkies", 10, 0)
    oled.show()
    if TOGGLE:
        oled.text("Hello World!", 13, 20)
        oled.show()
    sleep(1)
    TOGGLE = not TOGGLE