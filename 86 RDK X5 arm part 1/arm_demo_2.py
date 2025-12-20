from PCA9685_smbus2 import PCA9685
import time

# ---------------- CONFIG ----------------

I2C_BUS = 5
PWM_FREQ = 50

SERVO_CHANNELS = {
    "base": 0,
    "arm1": 1,
    "arm2": 2,
    "claw": 3
}

ANGLE_MIN = -90
ANGLE_MAX = 90

PWM_MIN = 102   # -90 deg
PWM_MID = 307   # 0 deg
PWM_MAX = 512   # +90 deg

STEP_DELAY = 0.02   # 20 ms per step (~50 Hz)
SPEED_FACTOR = 0.4  # 40% speed (lower = slower)

# ----------------------------------------

pwm = PCA9685.PCA9685(interface=I2C_BUS)
pwm.set_pwm_freq(PWM_FREQ)

# Track current angles
current_angles = {
    "base": 0,
    "arm1": 0,
    "arm2": 0,
    "claw": 0
}

# ----------------------------------------

def angle_to_pwm(angle):
    angle = max(ANGLE_MIN, min(ANGLE_MAX, angle))
    return int(PWM_MID + (angle / 90.0) * (PWM_MAX - PWM_MID))

def set_servo_angle(name, angle):
    ch = SERVO_CHANNELS[name]
    pwm_val = angle_to_pwm(angle)
    pwm.set_pwm(ch, 0, pwm_val)

# ----------------------------------------

def move_servos_smooth(targets, speed=SPEED_FACTOR):
    """
    targets = dict {servo_name: target_angle}
    """
    max_delta = max(abs(targets[s] - current_angles[s]) for s in targets)
    steps = max(1, int(max_delta * speed))

    for step in range(steps + 1):
        for s in targets:
            start = current_angles[s]
            end = targets[s]
            interp = start + (end - start) * (step / steps)
            set_servo_angle(s, interp)

        time.sleep(STEP_DELAY)

    for s in targets:
        current_angles[s] = targets[s]

# ----------------------------------------
# TEST SEQUENCE
# ----------------------------------------

def test_sequence():
    print("Moving all servos to 0")
    move_servos_smooth({
        "base": 0,
        "arm1": 0,
        "arm2": 0,
        "claw": 0
    })

    print("Base sweep")
    move_servos_smooth({"base": 80})
    move_servos_smooth({"base": -80})
    move_servos_smooth({"base": 0})

    print("Arm1 sweep")
    move_servos_smooth({"arm1": 70})
    move_servos_smooth({"arm1": -70})
    move_servos_smooth({"arm1": 0})

    print("Arm2 sweep")
    move_servos_smooth({"arm2": 70})
    move_servos_smooth({"arm2": -70})
    move_servos_smooth({"arm2": 0})

    print("Claw test")
    move_servos_smooth({"claw": 60})
    move_servos_smooth({"claw": 0})

# ----------------------------------------
# DANCE DEMO ??
# ----------------------------------------

def dance_demo():
    print("Dance mode!")

    moves = [
        {"base": 40, "arm1": 20},
        {"base": -40, "arm2": 30},
        {"arm1": -30, "arm2": -30},
        {"claw": 50},
        {"base": 0, "arm1": 0, "arm2": 0, "claw": 0}
    ]

    for _ in range(3):
        for m in moves:
            move_servos_smooth(m, speed=0.5)

# ----------------------------------------

if __name__ == "__main__":
    try:
        test_sequence()
        dance_demo()
    except KeyboardInterrupt:
        print("Exiting...")
