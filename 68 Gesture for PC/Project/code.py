import time
import board
import busio
import usb_hid
import paj7620

from adafruit_hid.consumer_control import ConsumerControl
from adafruit_hid.consumer_control_code import ConsumerControlCode

# I2C setup
i2c = busio.I2C(scl=board.SCL, sda=board.SDA)
sensor = paj7620.PAJ7620Gesture(i2c)

# HID setup
cc = ConsumerControl(usb_hid.devices)

# Gesture mappings
gesture_names = {
    paj7620.UP: "UP",
    paj7620.DOWN: "DOWN",
    paj7620.LEFT: "LEFT",
    paj7620.RIGHT: "RIGHT",
    paj7620.NEAR: "NEAR",
    paj7620.FAR: "FAR",
    paj7620.WAVE: "WAVE",
}

media_actions = {
    paj7620.UP: ConsumerControlCode.VOLUME_INCREMENT,
    paj7620.DOWN: ConsumerControlCode.VOLUME_DECREMENT,
    paj7620.LEFT: ConsumerControlCode.SCAN_PREVIOUS_TRACK,
    paj7620.RIGHT: ConsumerControlCode.SCAN_NEXT_TRACK,
    paj7620.NEAR: ConsumerControlCode.PLAY_PAUSE,
    paj7620.FAR: ConsumerControlCode.MUTE,
}

media_names = {
    ConsumerControlCode.VOLUME_INCREMENT: "VOLUME UP",
    ConsumerControlCode.VOLUME_DECREMENT: "VOLUME DOWN",
    ConsumerControlCode.SCAN_PREVIOUS_TRACK: "PREVIOUS TRACK",
    ConsumerControlCode.SCAN_NEXT_TRACK: "NEXT TRACK",
    ConsumerControlCode.PLAY_PAUSE: "PLAY/PAUSE",
    ConsumerControlCode.MUTE: "MUTE",
}

print("Gesture media controller ready")

while True:
    result = sensor.read()
    for gesture, action in media_actions.items():
        if result & gesture:
            gesture_str = gesture_names.get(gesture, str(gesture))
            media_str = media_names.get(action, str(action))
            print(f"Gesture: {gesture_str} → Media Action: {media_str}")
            cc.send(action)
            break  # Exit the for-loop after first match
    time.sleep(0.2)  # Small delay to prevent re-triggering
