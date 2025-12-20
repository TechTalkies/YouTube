from PCA9685_smbus2 import PCA9685
import time

pwm = PCA9685.PCA9685(interface=5) # defaults to using i2c-3 and address 0x40
pwm.set_pwm_freq(50) # set frequency to 50 Hz for SG90 servos

print("Press Ctrl+C to quit...")
while True:
    # set servo to 0 degrees
    pwm.set_pwm(0, 0, 307) # channel 0, off time 307 (1.5 ms)
    time.sleep(1)
