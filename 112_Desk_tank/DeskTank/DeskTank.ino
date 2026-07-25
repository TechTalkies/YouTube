/*
  ESP32-C3 Web Gamepad — 2x N20 Motor Controller (DRV8833) + OLED Face
  ---------------------------------------------------------------------
  NOTE: This is an Arduino sketch. Saved with a .c extension for easy viewing —
  rename to ESP32_N20_Gamepad.ino before opening it in Arduino IDE.

  - Hosts its own WiFi Access Point + web page (no router/internet needed)
  - ONE joystick (left) does everything:
      AUTO OFF -> normal 2D drive (forward/back + turn), speed slider caps max throttle
      AUTO ON  -> robot drives forward automatically at the slider's speed,
                  and the joystick LOCKS to horizontal-only movement,
                  becoming a pure left/right steering stick
  - Flip L/R and Flip F/B switches fix reversed wiring/motors in software.
  - Failsafe: motors stop if no command received for 400ms (lost connection)
  - SH1106 128x64 I2C OLED shows a simple face that blinks on its own,
    fully non-blocking (no delay()) so it never affects control latency.

  LIBRARIES NEEDED (Library Manager): "U8g2"
  
  	Copyright (c)
	Arduino project by Tech Talkies YouTube Channel.
	https://www.youtube.com/@techtalkies1
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <U8g2lib.h>

// ---------------- USER SETTINGS ----------------
const char* AP_SSID = "TT Desk Tank";
const char* AP_PASSWORD = "TechTalkies";  // min 8 chars, or set to "" for open AP

// Motor pins (DRV8833 — both pins per motor are PWM inputs)
#define L_IN1 6   // left motor forward PWM
#define L_IN2 7   // left motor reverse PWM
#define R_IN1 10  // right motor forward PWM
#define R_IN2 20  // right motor reverse PWM

// PWM config
const int PWM_FREQ = 1000;
const int PWM_RES = 8;  // 8-bit -> 0-255

const unsigned long FAILSAFE_MS = 400;  // stop motors if no packet in this time

// OLED (SH1106, 128x64, I2C)
#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_W 128
#define OLED_H 64
// ------------------------------------------------

WebServer server(80);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
bool oledOk = false;

volatile int lx = 0, ly = 0;  // joystick, -100..100
volatile bool autoMode = false;
volatile int autoSpeed = 50;  // 0..100 — universal: cruise speed (AUTO) / max throttle cap (MANUAL)
volatile unsigned long lastCmdMs = 0;

volatile bool flipLR = false;
volatile bool flipFB = false;

// ---------------- Face / blink animation ----------------
// Fully non-blocking: no delay() anywhere. Idle state costs one millis()
// compare per loop() and ZERO I2C traffic. A blink only redraws ~10 frames
// over ~250ms, so average load on the bus/CPU is negligible.
const int EYE_OPEN_H = 34;
const int EYE_CLOSED_H = 4;
const int EYE_STEP = 6;   // px per frame -> fluid but few frames
const int FRAME_MS = 20;  // frame pacing during a blink

int eyeH = EYE_OPEN_H;
int8_t blinkDir = 0;  // 0 idle, -1 closing, +1 opening
unsigned long lastFrameMs = 0;
unsigned long nextBlinkMs = 0;

void drawFace() {
  if (!oledOk) return;
  u8g2.clearBuffer();
  int eyeW = 26;
  int cx1 = OLED_W / 2 - 24, cx2 = OLED_W / 2 + 24;
  int cy = OLED_H / 2 - 4;

  // Corner radius must shrink with the eye or drawRBox glitches once the
  // box gets smaller than the radius.
  int r = eyeH / 2;
  if (r > 6) r = 6;
  if (r < 1) r = 1;

  u8g2.drawRBox(cx1 - eyeW / 2, cy - eyeH / 2, eyeW, eyeH, r);
  u8g2.drawRBox(cx2 - eyeW / 2, cy - eyeH / 2, eyeW, eyeH, r);

  int mouthW = 34, mouthH = 6;
  u8g2.drawRBox(OLED_W / 2 - mouthW / 2, OLED_H - 16, mouthW, mouthH, 3);

  u8g2.sendBuffer();
}

void updateFace() {
  if (!oledOk) return;
  unsigned long now = millis();

  if (blinkDir == 0) {
    if (now >= nextBlinkMs) {
      blinkDir = -1;
      lastFrameMs = now;
    }
    return;  // idle -> no drawing, no I2C traffic
  }
  if (now - lastFrameMs < (unsigned long)FRAME_MS) return;  // pace the frames
  lastFrameMs = now;

  eyeH += blinkDir * EYE_STEP;
  if (blinkDir < 0 && eyeH <= EYE_CLOSED_H) {
    eyeH = EYE_CLOSED_H;
    blinkDir = 1;
  } else if (blinkDir > 0 && eyeH >= EYE_OPEN_H) {
    eyeH = EYE_OPEN_H;
    blinkDir = 0;
    nextBlinkMs = now + random(2000, 5000);  // next blink in 2-5s
  }
  drawFace();
}

// ---------------- Motor helpers ----------------
// DRV8833: PWM the forward pin for +speed, PWM the reverse pin for -speed,
// the other pin held at 0 duty.
void setMotor(int speedPercent, int pinFwd, int pinRev) {
  speedPercent = constrain(speedPercent, -100, 100);
  int duty = map(abs(speedPercent), 0, 100, 0, 255);
  if (speedPercent >= 0) {
    ledcWrite(pinFwd, duty);
    ledcWrite(pinRev, 0);
  } else {
    ledcWrite(pinFwd, 0);
    ledcWrite(pinRev, duty);
  }
}

void stopMotors() {
  setMotor(0, L_IN1, L_IN2);
  setMotor(0, R_IN1, R_IN2);
}

void applyDrive() {
  int left, right;

  if (autoMode) {
    // Cruise speed = slider, FIXED. Turning only ever slows the inner wheel
    // — the outer wheel always stays pinned at "forward", so the set speed
    // never changes no matter how hard you steer.
    int forward = autoSpeed;
    int turn = lx;  // joystick X only (Y is locked by UI in auto mode)
    if (turn >= 0) {
      left = forward;
      right = forward - turn;
    } else {
      right = forward;
      left = forward + turn;  // turn is negative here, so this subtracts
    }
    // Naturally bounded to [-100, 100] already — no clamp/normalize needed.
  } else {
    // MANUAL: speed slider now caps max throttle here too (universal).
    float scale = autoSpeed / 100.0f;
    int fwd = (int)(ly * scale);
    int turn = (int)(lx * scale);
    left = fwd + turn;
    right = fwd - turn;

    int maxMag = max(abs(left), abs(right));
    if (maxMag > 100) {
      left = (left * 100) / maxMag;
      right = (right * 100) / maxMag;
    }
  }

  if (flipLR) {
    int t = left;
    left = right;
    right = t;
  }

  if (flipFB) {
    left = -left;
    right = -right;
  }

  setMotor(left, L_IN1, L_IN2);
  setMotor(right, R_IN1, R_IN2);
}

// ---------------- Web page ----------------
const char PAGE_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>TT Desk Tank</title>
<style>
  html,body{margin:0;height:100%;color:#eee;font-family:'Segoe UI',sans-serif;
    display:flex;flex-direction:column;align-items:center;overflow:hidden;touch-action:none;user-select:none;
    background:linear-gradient(160deg,#1b2340 0%,#0d1526 55%,#090c16 100%);}
  h3{margin:10px 0 4px;font-weight:600;letter-spacing:1px;color:#7fd4ff;
    text-shadow:0 0 12px #4a90d955;}
  .row{display:flex;justify-content:space-around;align-items:center;width:100%;flex:1;gap:10px;}
  .stickWrap{display:flex;flex-direction:column;align-items:center;}
  .base{width:180px;height:180px;border-radius:50%;
    background:radial-gradient(circle at 35% 30%,#2a3355,#161c30);
    border:2px solid #3a4570;box-shadow:inset 0 0 14px #00000088, 0 0 10px #4a90d922;
    position:relative;touch-action:none;}
  .knob{width:70px;height:70px;border-radius:50%;
    background:radial-gradient(circle at 35% 30%,#6ab0ff,#3568b8);
    position:absolute;left:55px;top:55px;box-shadow:0 0 14px #4a90d9cc;}
  .panel{display:flex;flex-direction:column;align-items:center;gap:16px;
    padding:22px 20px;border-radius:16px;
    background:linear-gradient(160deg,#1e2745,#141a30);
    border:1px solid #3a4570;box-shadow:0 4px 18px #00000066, inset 0 0 0 1px #ffffff08;}
  .switchWrap{display:flex;align-items:center;gap:10px;}
  .switch{position:relative;width:60px;height:30px;background:#2a3150;border-radius:15px;cursor:pointer;
    border:1px solid #3a4570;}
  .switch .dot{position:absolute;width:24px;height:24px;top:2px;left:2px;border-radius:50%;background:#8892b8;transition:.15s;}
  .switch.on{background:#1f8a54;}
  .switch.on .dot{left:32px;background:#fff;}
  input[type=range]{width:140px;accent-color:#4a90d9;}
  .label{font-size:13px;color:#9aa6c8;margin-bottom:6px;letter-spacing:.5px;}
  #speedVal{color:#7fd4ff;font-weight:bold;}
  .divider{width:100%;height:1px;background:#3a4570;margin:2px 0;}
</style>
</head>
<body>
<h3>TT DESK TANK</h3>
<div class="row">
  <div class="stickWrap">
    <div class="label" id="stickLabel">Manual drive</div>
    <div class="base" id="base"><div class="knob" id="knob"></div></div>
  </div>

  <div class="panel">
    <div class="switchWrap">
      <span class="label">AUTO FWD</span>
      <div class="switch" id="autoSw"><div class="dot"></div></div>
    </div>
    <div>
      <div class="label">Speed: <span id="speedVal">50</span>%</div>
      <input type="range" id="speed" min="0" max="100" value="50">
    </div>
    <div class="divider"></div>
    <div class="switchWrap">
      <span class="label">Flip L/R</span>
      <div class="switch" id="flipLRSw"><div class="dot"></div></div>
    </div>
    <div class="switchWrap">
      <span class="label">Flip F/B</span>
      <div class="switch" id="flipFBSw"><div class="dot"></div></div>
    </div>
  </div>
</div>

<script>
let auto = false;
let speed = 50;
let flipLR = false;
let flipFB = false;
let state = {lx:0, ly:0};

function makeStick(baseId, knobId, lockHorizontal) {
  const base = document.getElementById(baseId);
  const knob = document.getElementById(knobId);
  const R = 55; // max travel radius (px)
  let dragging = false;

  function setKnob(dx, dy) {
    knob.style.left = (55 + dx) + "px";
    knob.style.top  = (55 + dy) + "px";
  }
  function reset() {
    setKnob(0,0);
    return {x:0, y:0};
  }
  function handle(clientX, clientY) {
    const rect = base.getBoundingClientRect();
    let dx = clientX - (rect.left + rect.width/2);
    let dy = clientY - (rect.top + rect.height/2);
    if (lockHorizontal && lockHorizontal()) dy = 0;
    let dist = Math.sqrt(dx*dx + dy*dy);
    if (dist > R) { dx = dx*R/dist; dy = dy*R/dist; }
    setKnob(dx, dy);
    return { x: Math.round((dx/R)*100), y: Math.round((-dy/R)*100) };
  }

  function start(e){ dragging = true; move(e); }
  function move(e){
    if(!dragging) return;
    const t = e.touches ? e.touches[0] : e;
    const v = handle(t.clientX, t.clientY);
    onMove(v);
    e.preventDefault();
  }
  function end(e){
    dragging = false;
    const v = reset();
    onMove(v);
  }

  let onMove = ()=>{};
  base.addEventListener('mousedown', start);
  base.addEventListener('touchstart', start, {passive:false});
  window.addEventListener('mousemove', move);
  window.addEventListener('touchmove', move, {passive:false});
  window.addEventListener('mouseup', end);
  window.addEventListener('touchend', end);

  return { onMove: (fn)=>{ onMove = fn; } };
}

// Single stick: horizontal-locked whenever AUTO is on
const stick = makeStick('base','knob', () => auto);
stick.onMove(v => { state.lx = v.x; state.ly = v.y; });

const autoSw = document.getElementById('autoSw');
autoSw.addEventListener('click', () => {
  auto = !auto;
  autoSw.classList.toggle('on', auto);
  document.getElementById('stickLabel').textContent = auto ? "Steer only (auto)" : "Manual drive";
});

const speedSlider = document.getElementById('speed');
speedSlider.addEventListener('input', () => {
  speed = speedSlider.value;
  document.getElementById('speedVal').textContent = speed;
});

const flipLRSw = document.getElementById('flipLRSw');
flipLRSw.addEventListener('click', () => {
  flipLR = !flipLR;
  flipLRSw.classList.toggle('on', flipLR);
});

const flipFBSw = document.getElementById('flipFBSw');
flipFBSw.addEventListener('click', () => {
  flipFB = !flipFB;
  flipFBSw.classList.toggle('on', flipFB);
});

// Send state to ESP32 continuously
setInterval(() => {
  const url = `/control?lx=${state.lx}&ly=${state.ly}&auto=${auto?1:0}&speed=${speed}&flipLR=${flipLR?1:0}&flipFB=${flipFB?1:0}`;
  fetch(url).catch(()=>{});
}, 100);
</script>
</body>
</html>
)HTML";

// ---------------- HTTP handlers ----------------
void handleRoot() {
  server.send_P(200, "text/html", PAGE_HTML);
}

void handleControl() {
  if (server.hasArg("lx")) lx = server.arg("lx").toInt();
  if (server.hasArg("ly")) ly = server.arg("ly").toInt();
  if (server.hasArg("auto")) autoMode = server.arg("auto").toInt() == 1;
  if (server.hasArg("speed")) autoSpeed = server.arg("speed").toInt();

  if (server.hasArg("flipLR"))
    flipLR = server.arg("flipLR").toInt();

  if (server.hasArg("flipFB"))
    flipFB = server.arg("flipFB").toInt();

  lastCmdMs = millis();
  applyDrive();
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  ledcAttach(L_IN1, PWM_FREQ, PWM_RES);
  ledcAttach(L_IN2, PWM_FREQ, PWM_RES);
  ledcAttach(R_IN1, PWM_FREQ, PWM_RES);
  ledcAttach(R_IN2, PWM_FREQ, PWM_RES);

  stopMotors();

  Wire.begin(OLED_SDA, OLED_SCL);
  oledOk = u8g2.begin();
  if (oledOk) {
    drawFace();  // initial open-eyes frame
    nextBlinkMs = millis() + random(1000, 3000);
    Serial.println("OLED Init!");
  } else {
    Serial.println("OLED not found - check wiring/address");
  }

  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  //esp_bt_controller_disable();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());  // usually 192.168.4.1

  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.begin();

  lastCmdMs = millis();
}

void loop() {
  server.handleClient();
  updateFace();  // non-blocking; no-op most of the time

  // Failsafe: stop if link goes quiet
  if (millis() - lastCmdMs > FAILSAFE_MS) {
    stopMotors();
  }
}