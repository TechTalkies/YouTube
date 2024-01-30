import time
import neopixel
from digitalio import DigitalInOut, Direction, Pull
import board

# NeoPixel ring setup. Update num_pixels if using a different ring.
num_pixels = 16
pixels = neopixel.NeoPixel(board.GP17, num_pixels, brightness=0.3, auto_write=False)

WHITE = 255, 255, 255
TEAL = 0, 150, 150
RED = 255, 0, 0
GREEN = 0, 255, 0
YELLOW = 255, 255, 0
BLUE = 0, 0, 255
ORANGE = 255, 127, 0
VIOLET = 139, 0, 255

colors = [VIOLET, RED, GREEN, YELLOW, BLUE, ORANGE, WHITE, TEAL]

CLK_PIN = DigitalInOut(board.GP15)
DT_PIN = DigitalInOut(board.GP14)
SW_PIN = DigitalInOut(board.GP16)

CLK_PIN.direction = Direction.INPUT
DT_PIN.direction = Direction.INPUT
SW_PIN.direction = Direction.INPUT

CLK_PIN.pull = Pull.UP
DT_PIN.pull = Pull.UP
SW_PIN.pull = Pull.UP

previousValue = True
pixelPosition = 1
mode = 1

pixels.fill(0)
pixels.show()
LEFT = 1
RIGHT = 2
currentColor = VIOLET
colorIndex = 0
pixelCount = 1

def displayPixels():
    global num_pixels
    global pixelPosition
    global pixelCount
    
    pixels.fill(0)
    pixels.show()
    for i in range(pixelCount):
        
        if pixelPosition+i > num_pixels-1:
            nextPixel = pixelCount -i-1
        else: nextPixel = pixelPosition+i
        
        pixels[nextPixel] = currentColor
    
    pixels.show()

def rotatePosition(direction):
    global pixelPosition
    if direction == LEFT:
        pixelPosition = pixelPosition-1
        if pixelPosition < 0:
            pixelPosition = num_pixels-1
        
    else:
        pixelPosition = pixelPosition+1
        if pixelPosition > num_pixels-1:
            pixelPosition = 0
            
    displayPixels()
    print(pixelPosition)
    
displayPixels()

while True:
    if previousValue != DT_PIN.value:
        if DT_PIN.value == False:
            if CLK_PIN.value == False:
                print("to left")
                direction = LEFT
            else:
                print("to right")
                direction = RIGHT
                
            if mode == 1: rotatePosition(direction)
            
            elif mode == 2:
                colorIndex = colorIndex +1
                if colorIndex > len(colors)-1: colorIndex = 0
                if colorIndex < 0: colorIndex = len(colors-1)
                
                currentColor = colors[colorIndex]
                displayPixels()
                
            elif mode == 3:
                if direction == LEFT:
                    pixelCount = pixelCount -1
                else: pixelCount = pixelCount+1
                
                if pixelCount >5: pixelCount =5
                if pixelCount <1: pixelCount =1
                
                displayPixels()
                
    previousValue = DT_PIN.value
    
    if not SW_PIN.value:
        pixels.fill(0)
        pixels.show()
        time.sleep(0.5)
        displayPixels()
        mode = mode+1
        if mode > 3: mode = 1