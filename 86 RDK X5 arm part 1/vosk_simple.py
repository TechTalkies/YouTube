import time
import json
import queue
import sounddevice as sd
from vosk import Model, KaldiRecognizer
from PCA9685_smbus2 import PCA9685

# ================== VOSK SETUP ==================
MODEL_PATH = "/home/sunrise/Downloads/vosk-model-small-en-us-0.15/"
SAMPLE_RATE = 16000

audio_q = queue.Queue()

def audio_callback(indata, frames, time_info, status):
    if status:
        print(status)
    audio_q.put(bytes(indata))

print("Loading Vosk model...")
model = Model(MODEL_PATH)
rec = KaldiRecognizer(model, SAMPLE_RATE)

# ================== PCA9685 SETUP ==================
pwm = PCA9685.PCA9685(interface=5)
pwm.set_pwm_freq(50)

# ================== SERVO CONFIG ==================
SERVOS = {
    "base": 0,
    "arm1": 1,
    "arm2": 2,
    "claw": 3
}

MIN_PULSE = 102
MAX_PULSE = 512

current_angle = {
    "base": 0,
    "arm1": 0,
    "arm2": 0,
    "claw": 0
}

# ================== SERVO UTILS ==================
def angle_to_pwm(angle):
    angle = max(-90, min(90, angle))
    span = MAX_PULSE - MIN_PULSE
    return int(MIN_PULSE + ((angle + 90) / 180.0) * span)

def move_servos(targets, duration=0.8, steps=50):
    start_angles = {k: current_angle[k] for k in targets}
    sleep_time = duration / steps

    for step in range(steps + 1):
        t = step / steps
        for name, target in targets.items():
            start = start_angles[name]
            angle = start + (target - start) * t
            pwm.set_pwm(SERVOS[name], 0, angle_to_pwm(angle))
        time.sleep(sleep_time)

    for name, target in targets.items():
        current_angle[name] = target

# ================== COMMAND LOGIC ==================
STEP = 15  # degrees per voice command

def clamp(a):
    return max(-90, min(90, a))

def handle_command(text):
    text = text.lower()
    print(">>", text)

    if "reset" in text:
        move_servos(
            {"base": 0, "arm1": 0, "arm2": 0, "claw": 0},
            duration=1.0
        )
        return

    if "left" in text:
        move_servos({"base": clamp(current_angle["base"] + STEP)})
    elif "right" in text:
        move_servos({"base": clamp(current_angle["base"] - STEP)})

    elif "up" in text:
        move_servos({"arm2": clamp(current_angle["arm2"] - STEP)})
    elif "down" in text:
        move_servos({"arm2": clamp(current_angle["arm2"] + STEP)})

    elif "forward" in text:
        move_servos({"arm1": clamp(current_angle["arm1"] - STEP)})
    elif "back" in text or "backward" in text:
        move_servos({"arm1": clamp(current_angle["arm1"] + STEP)})

    elif "open" in text:
        move_servos({"claw": 60})
    elif "close" in text:
        move_servos({"claw": 0})

# ================== MAIN LOOP ==================
print("Voice control ready.")
print("Commands: left right up down forward backward open close reset")
print("Press Ctrl+C to exit.")

with sd.RawInputStream(
    samplerate=SAMPLE_RATE,
    blocksize=8000,
    dtype="int16",
    channels=1,
    callback=audio_callback
):
    try:
        while True:
            data = audio_q.get()
            if rec.AcceptWaveform(data):
                result = json.loads(rec.Result())
                if result.get("text"):
                    handle_command(result["text"])
    except KeyboardInterrupt:
        print("\nExiting safely...")
        move_servos({"base": 0, "arm1": 0, "arm2": 0, "claw": 0}, duration=1.0)
