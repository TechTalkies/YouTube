import adafruit_display_text.label
import digitalio
import board
import busio, time
import adafruit_mpu6050
import displayio
import framebufferio
import rgbmatrix
import terminalio
import PixelDust

i2c = busio.I2C(sda=board.GP18,scl=board.GP19)
mpu = adafruit_mpu6050.MPU6050(i2c)

mpu = adafruit_mpu6050.MPU6050(i2c)

displayio.release_displays()

WIDTH = 64
HEIGHT = 32

matrix = rgbmatrix.RGBMatrix(
    width=WIDTH, height=HEIGHT, bit_depth=3,
    rgb_pins=[board.GP2, board.GP3, board.GP4, board.GP5, board.GP6, board.GP7],
    addr_pins=[board.GP8, board.GP9, board.GP10, board.GP11 ],
    clock_pin=board.GP12, latch_pin=board.GP13, output_enable_pin=board.GP14)

# Create a two color palette
palette = displayio.Palette(8)
palette[0] = 0x000000
palette[1] = 0x0099ff
palette[2] = 0xff66ff
palette[3] = 0xffcc66
palette[4] = 0x99ff33
palette[5] = 0x00ff99
palette[6] = 0xcc00ff
palette[7] = 0x336699

# Associate the RGB matrix with a Display so that we can use displayio features
display = framebufferio.FramebufferDisplay(matrix, auto_refresh=False)

# Create a bitmap with two colors
bitmap = displayio.Bitmap(display.width, display.height, 8)

# Create a TileGrid using the Bitmap and Palette
tile_grid = displayio.TileGrid(bitmap, pixel_shader=palette)

# Create a Group
group = displayio.Group()

# Add the TileGrid to the Group
group.append(tile_grid)

# Add the Group to the Display
display.root_group = group

prevGrains = PixelDust.Grain

num_pixels = 200
sand = PixelDust.PixelDust(WIDTH,HEIGHT, num_pixels)
sand.scale = 1

sand.randomize(num_pixels)  # Populate the grains before getting their positions

n = 0
for i in range(8):
    xx = int(i * 63 / 8)  # This will create values from 0 to 63
    for y in range(8):
        yy = int(31 * (7 + y) / 8)  # Adjust to ensure y is valid
        for x in range(8):
            if 0 <= xx + x < 64 and 0 <= yy < 32:  # Ensure bounds are valid
                sand.set_position(xx + x, yy)
                n += 1

while True:
    
    # Clear current grains for the next iteration
    for i in range(num_pixels):
        x, y = sand.get_position(i)
        bitmap[int(x), int (y)] = 0  # Clear the bitmap at the current grain position
        
    x = mpu.acceleration[0]
    y = mpu.acceleration[1]
    z = mpu.acceleration[2]
    adjustedValues = (-x, y, z)
    
    sand.iterate(adjustedValues)
                
    # Draw grains
    for i in range(num_pixels):  # Use actual number of grains
        x1, y1 = sand.get_position(i)
        n = int(i % 8) # Color index
        if 0 <= x1 < WIDTH and 0 <= y1 < HEIGHT:  # Check bounds
            bitmap[int(x1), int(y1)] = n
            
    display.refresh()