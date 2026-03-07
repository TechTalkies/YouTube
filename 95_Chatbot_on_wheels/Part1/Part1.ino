/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define TOUCH_PIN 6

/* -------- DRV8833 -------- */

#define M1_IN1 0
#define M1_IN2 1
#define M2_IN1 3
#define M2_IN2 4

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/* -------- STATES -------- */

#define STATE_ROAM 0
#define STATE_PET 1
#define STATE_ATTN 2

int state = STATE_ATTN;
int turnSpeed = 40;
int motorSpeed = 40;   // 0–255 speed control

/* -------- FACE PHYSICS -------- */

float faceX = 0;
float faceY = 0;

float targetFaceX = 0;
float targetFaceY = 0;

float velX = 0;
float velY = 0;

float spring = 0.08;
float damping = 0.75;

/* -------- EYES -------- */

float eyeW = 28;
float eyeH = 28;

float targetEyeW = 28;
float targetEyeH = 28;

/* -------- BLINK -------- */

bool blinking = false;
unsigned long lastBlink = 0;
unsigned long nextBlink = 0;

/* -------- TOUCH TIMER -------- */

bool touchActive = false;
unsigned long touchStartTime = 0;
unsigned long touchDuration = 0;
unsigned long lastTouchPrint = 0;

/* -------- PET TIMER -------- */

unsigned long lastPet = 0;

/* -------- MOTOR -------- */

unsigned long lastMove = 0;
unsigned long moveInterval = 3000;
int moveState = 0;

/* -------- MOTOR CONTROL -------- */

void motorStop() {
  analogWrite(M1_IN1, 0);
  analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, 0);
  analogWrite(M2_IN2, 0);
}

void motorForward() {
  analogWrite(M1_IN1, motorSpeed);
  analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, motorSpeed);
  analogWrite(M2_IN2, 0);
}

void rotateCW() {
  analogWrite(M1_IN1, turnSpeed);
  analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, 0);
  analogWrite(M2_IN2, turnSpeed);
}

void rotateCCW() {
  analogWrite(M1_IN1, 0);
  analogWrite(M1_IN2, turnSpeed);
  analogWrite(M2_IN1, turnSpeed);
  analogWrite(M2_IN2, 0);
}

/* -------- TOUCH TRACKING -------- */

void updateTouchTimer() {

  bool touched = digitalRead(TOUCH_PIN);

  /* Touch started */

  if (touched && !touchActive) {

    touchActive = true;
    touchStartTime = millis();

    Serial.println("Touch start");

    state = STATE_PET;
    motorStop();
  }

  /* Touch ongoing */

  if (touched && touchActive) {

    touchDuration = millis() - touchStartTime;

    lastPet = millis();

    if (millis() - lastTouchPrint > 100) {

      Serial.print("Touch duration: ");
      Serial.print(touchDuration);
      Serial.println(" ms");

      lastTouchPrint = millis();
    }
  }

  /* Touch released */

  if (!touched && touchActive) {

    touchActive = false;

    touchDuration = millis() - touchStartTime;

    Serial.print("Touch released after ");
    Serial.print(touchDuration);
    Serial.println(" ms");
  }
}

/* -------- ROAM BEHAVIOR -------- */

void updateRoam() {

  if (state != STATE_ROAM) return;

  if (millis() - lastMove > moveInterval) {

    lastMove = millis();
    moveInterval = random(2000, 5000);

    moveState = random(0, 4);

    if (moveState == 0) motorForward();
    if (moveState == 1) rotateCW();
    if (moveState == 2) rotateCCW();
    if (moveState == 3) motorStop();  // short pause
  }
}

/* -------- STATE MACHINE -------- */

void updateState() {

  unsigned long now = millis();

  if (state == STATE_PET) {

    if (!touchActive && now - lastPet > 2000) {

      state = STATE_ROAM;
    }
  }

  if (state == STATE_ROAM) {

    if (now - lastPet > 150000) {

      state = STATE_ATTN;
      motorStop();
    }
  }

  if (state == STATE_ATTN) {

    if (touchActive) {

      state = STATE_PET;
    }
  }
}

/* -------- FACE PHYSICS -------- */

void updateFacePhysics() {

  float ax = (targetFaceX - faceX) * spring;
  float ay = (targetFaceY - faceY) * spring;

  velX = (velX + ax) * damping;
  velY = (velY + ay) * damping;

  faceX += velX;
  faceY += velY;
}

/* -------- RANDOM LOOK -------- */

unsigned long lastLook = 0;

void updateLook() {

  if (millis() - lastLook > 2000) {

    lastLook = millis();

    targetFaceX = random(-6, 7);
    targetFaceY = random(-4, 5);
  }
}

/* -------- BLINK -------- */

void updateBlink() {

  unsigned long now = millis();

  if (now > nextBlink) {

    blinking = true;
    lastBlink = now;

    nextBlink = now + random(3000, 7000);
  }

  if (blinking) {

    targetEyeH = 2;

    if (now - lastBlink > 120)
      blinking = false;
  }
}

/* -------- FACE SHAPE -------- */

void updateFaceShape() {

  switch (state) {

    case STATE_ROAM:

      targetEyeW = 36;
      targetEyeH = 36;

      break;

    case STATE_PET:

      targetEyeW = 42;
      targetEyeH = 24;

      break;

    case STATE_ATTN:

      targetEyeW = 28;
      targetEyeH = 42;

      break;
  }

  eyeW += (targetEyeW - eyeW) * 0.2;
  eyeH += (targetEyeH - eyeH) * 0.2;
}

/* -------- DRAW FACE -------- */

void drawFace() {

  int cx = 64 + faceX;
  int cy = 32 + faceY;

  int leftEyeX = cx - 38;
  int rightEyeX = cx + 2;

  int eyeY = cy - 18;

  display.fillRoundRect(leftEyeX, eyeY, eyeW, eyeH, 8, SH110X_WHITE);
  display.fillRoundRect(rightEyeX, eyeY, eyeW, eyeH, 8, SH110X_WHITE);

  int mouthY = cy + 20;

  if (state == STATE_ROAM)
    display.fillRoundRect(cx - 12, mouthY, 24, 6, 4, SH110X_WHITE);

  if (state == STATE_PET)
    display.fillRoundRect(cx - 14, mouthY, 28, 10, 6, SH110X_WHITE);

  if (state == STATE_ATTN)
    display.fillRoundRect(cx - 6, mouthY, 12, 12, 6, SH110X_WHITE);
}

/* -------- SETUP -------- */

void setup() {

  Serial.begin(115200);
  Wire.begin();

  pinMode(TOUCH_PIN, INPUT);
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);

  display.begin(0x3C, true);

  nextBlink = millis() + random(2000, 4000);

  randomSeed(analogRead(0));

  lastPet = millis();
}

/* -------- LOOP -------- */

void loop() {
  updateTouchTimer();
  updateState();
  updateRoam();
  updateFaceShape();
  updateBlink();
  updateLook();
  updateFacePhysics();

  display.clearDisplay();
  drawFace();
  display.display();

  delay(20);
}