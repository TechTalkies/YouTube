###############################################################
# SSD1306 OLED Display I2C Tests with the Raspberry Pi Pico
# -- ADC Reading and Display of MEMS Microphone
#
# by Joshua Hrisko, Maker Portal LLC (c) 2021
#
#
# Based on the Pico Micropython repository at:
# https://github.com/raspberrypi/pico-micropython-examples/tree/master/i2c/1306oled
###############################################################
#
#
from machine import Pin, I2C, ADC
from ssd1306 import SSD1306_I2C
import framebuf,sys,time

pix_res_x  = 128 # SSD1306 horizontal resolution
pix_res_y = 64   # SSD1306 vertical resolution

i2c_dev = I2C(1, sda=Pin(14), scl=Pin(15), freq=400000)
i2c_addr = [hex(ii) for ii in i2c_dev.scan()] # get I2C address in hex format
if i2c_addr==[]:
    print('No I2C Display Found') 
    sys.exit() # exit routine if no dev found
else:
    print("I2C Address      : {}".format(i2c_addr[0])) # I2C device address
    print("I2C Configuration: {}".format(i2c_dev)) # print I2C params


oled = SSD1306_I2C(pix_res_x, pix_res_y, i2c_dev) # oled controller

oled.write_cmd(0xc0) # flip display to place 0,0 at lower-left corner
adc_2 = machine.ADC(0) # ADC channel 2 for input
while True:
    oled.fill(0) # clear the display
    for jj in range(pix_res_x): # loop through horizontal display resolution
        adc_pt = adc_2.read_u16() # read adc and get 16-bit data point
        plot_pt = (adc_pt/((2**16)-1))*(pix_res_y-1) # convert to OLED pixels
        if int(plot_pt) >0: plot_pt = plot_pt  +8
        oled.text('|',jj,int(plot_pt)) # update x=jj with data point
    oled.show() # show the new text and image