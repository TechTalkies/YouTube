from PCA9685_smbus2 import PCA9685
import time

# ---------- PCA9685 SETUP ----------
pwm = PCA9685.PCA9685(interface=5)
pwm.set_pwm_freq(50)

# ---------- SERVO CONFIG ----------
SERVOS = {
    "base": 0,
    "arm1": 1,
    "arm2": 2,
    "claw": 3
}

# SG90 approx pulse range (adjust if needed)
MIN_PULSE = 102   # ~1ms
MAX_PULSE = 512   # ~2ms

# Track current angles
current_angle = {
    "base": 0,
    "arm1": 0,
    "arm2": 0,
    "claw": 0
}

# ---------- UTILS ----------
def angle_to_pwm(angle):
    """
    Map -90..+90 degrees to PWM pulse
    """
    angle = max(-90, min(90, angle))
    span = MAX_PULSE - MIN_PULSE
    return int(MIN_PULSE + ((angle + 90) / 180.0) * span)

def set_servo(name, angle):
    ch = SERVOS[name]
    pwm.set_pwm(ch, 0, angle_to_pwm(angle))
    current_angle[name] = angle

# ---------- LINEAR EASING MOVE ----------
def move_servos(targets, duration=1.0, steps=60):
    """
    targets = dict {servo_name: target_angle}
    duration = total move time (seconds)
    steps = smoothness (more = smoother)
    """
    start_angles = {k: current_angle[k] for k in targets}
    sleep_time = duration / steps

    for step in range(steps + 1):
        t = step / steps  # 0.0 ? 1.0 (linear easing)
        for name, target in targets.items():
            start = start_angles[name]
            angle = start + (target - start) * t
            pwm.set_pwm(SERVOS[name], 0, angle_to_pwm(angle))
        time.sleep(sleep_time)

    # finalize
    for name, target in targets.items():
        current_angle[name] = target

# ---------- TEST SEQUENCE ----------
def test_arm():
    print("Moving all servos to 0")
    move_servos(
        {"base": 0, "arm1": 0, "arm2": 0, "claw": 0},
        duration=1.2
    )

    print("Base left/right")
    move_servos({"base": 80}, duration=1.0)
    move_servos({"base": -80}, duration=1.0)
    move_servos({"base": 0}, duration=0.8)

    print("Arm1 up/down")
    move_servos({"arm1": 70}, duration=1.0)
    move_servos({"arm1": -50}, duration=1.0)
    move_servos({"arm1": 0}, duration=0.8)

    print("Arm2 up/down")
    move_servos({"arm2": 70}, duration=1.0)
    move_servos({"arm2": -50}, duration=1.0)
    move_servos({"arm2": 0}, duration=0.8)

    print("Claw open/close")
    move_servos({"claw": 60}, duration=0.8)
    move_servos({"claw": 0}, duration=0.8)

# ---------- MAIN ----------
if __name__ == "__main__":
    print("Starting smooth servo test. Ctrl+C to exit.")
    test_arm()
