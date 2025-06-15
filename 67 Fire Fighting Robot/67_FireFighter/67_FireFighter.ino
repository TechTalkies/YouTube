/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <AFMotor.h>
#include <Servo.h>

// Motors
AF_DCMotor leftMotor(3);    // M3
AF_DCMotor rightMotor(2);   // M2
AF_DCMotor pumpMotor(4);    // M4 for water pump

Servo sprayServo;

// Flame sensors A0–A4
const int flamePins[5] = { A0, A1, A2, A3, A4 };
const int flameThreshold = 300;  // Below this = no fire

void setup() {
  Serial.begin(9600);
  sprayServo.attach(9);
  sprayServo.write(90);  // Center the servo

  leftMotor.setSpeed(100);
  rightMotor.setSpeed(100);
  pumpMotor.setSpeed(200);
}

void loop() {
  int values[5];
  int maxValue = 0;
  int flameIndex = -1;

  // Read sensors and find the one with strongest flame (max value)
  for (int i = 0; i < 5; i++) {
    values[i] = analogRead(flamePins[i]);
    Serial.print(values[i]);
    if (i < 4) Serial.print(", ");
    if (values[i] > maxValue) {
      maxValue = values[i];
      flameIndex = i;
    }
  }
  Serial.println();

  if (maxValue > flameThreshold) {
    Serial.println("Flame Detected");

    if (flameIndex < 2) {
      turnLeft();
    } else if (flameIndex > 2) {
      turnRight();
    } else {
      // Flame centered
      if (values[2] > 800) {  // Close enough
        stopMotors();
        extinguish();
      } else {
        moveForward();
      }
    }
  } else {
    Serial.println("No Flame");
    stopMotors();
    pumpMotor.run(RELEASE);
  }

  delay(200);
}

void turnLeft() {
  leftMotor.run(BACKWARD);
  rightMotor.run(FORWARD);
}

void turnRight() {
  leftMotor.run(FORWARD);
  rightMotor.run(BACKWARD);
}

void moveForward() {
  leftMotor.run(BACKWARD);
  rightMotor.run(BACKWARD);
}

void stopMotors() {
  leftMotor.run(RELEASE);
  rightMotor.run(RELEASE);
}

void extinguish() {
  pumpMotor.run(FORWARD);

  for (int pos = 80; pos <= 100; pos += 5) {
    sprayServo.write(pos);
    delay(100);
  }
  for (int pos = 100; pos >= 80; pos -= 5) {
    sprayServo.write(pos);
    delay(100);
  }

  sprayServo.write(90);
  pumpMotor.run(RELEASE);
}
