import time
import board
import busio
import usb_hid
import paj7620

from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode

# I2C and Sensor setup
i2c = busio.I2C(scl=board.SCL, sda=board.SDA)
sensor = paj7620.PAJ7620Gesture(i2c)

# HID keyboard setup
keyboard = Keyboard(usb_hid.devices)

# Gesture name mapping
gesture_names = {
    paj7620.LEFT: "LEFT",
    paj7620.RIGHT: "RIGHT",
    paj7620.UP: "UP",
    paj7620.DOWN: "DOWN",
}

# Gesture to key mapping
# UP and RIGHT → send Right Arrow
# DOWN and LEFT → send Left Arrow
key_actions = {
    paj7620.LEFT: Keycode.LEFT_ARROW,
    paj7620.RIGHT: Keycode.RIGHT_ARROW,
    paj7620.UP: Keycode.RIGHT_ARROW,
    paj7620.DOWN: Keycode.LEFT_ARROW,
}

print("Gesture arrow key controller ready")

while True:
    result = sensor.read()
    for gesture, keycode in key_actions.items():
        if result & gesture:
            gesture_str = gesture_names.get(gesture, str(gesture))
            print(f"Gesture: {gesture_str} → Key: {keycode}")
            keyboard.send(keycode)
            break  # Only one gesture at a time
    time.sleep(0.2)
