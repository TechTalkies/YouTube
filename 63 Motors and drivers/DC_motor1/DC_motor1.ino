/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
// For L298N & TB6612

// Motor A pins
const int in1A = 8;
const int in2A = 9;
const int ena = 10;  // must be PWM-capable

// Motor B pins
const int in1B = 6;
const int in2B = 7;
const int enb = 5;  // must be PWM-capable

void setup() {
  pinMode(in1A, OUTPUT);
  pinMode(in2A, OUTPUT);
  pinMode(ena, OUTPUT);

  pinMode(in1B, OUTPUT);
  pinMode(in2B, OUTPUT);
  pinMode(enb, OUTPUT);
}

void loop() {
  // Both motors forward
  digitalWrite(in1A, HIGH);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, HIGH);
  digitalWrite(in2B, LOW);
  analogWrite(ena, 175);
  analogWrite(enb, 175);
  delay(2000);

  // Both motors backward
  digitalWrite(in1A, LOW);
  digitalWrite(in2A, HIGH);
  digitalWrite(in1B, LOW);
  digitalWrite(in2B, HIGH);
  analogWrite(ena, 200);
  analogWrite(enb, 200);
  delay(2000);

  // Stop motors
  digitalWrite(in1A, LOW);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, LOW);
  digitalWrite(in2B, LOW);
  delay(2000);

  // Run forward at slow speed
  digitalWrite(in1A, HIGH);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, HIGH);
  digitalWrite(in2B, LOW);
  analogWrite(ena, 75);
  analogWrite(enb, 75);
  delay(2000);

  // Run forward at slow speed
  digitalWrite(in1A, HIGH);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, HIGH);
  digitalWrite(in2B, LOW);
  analogWrite(ena, 120);
  analogWrite(enb, 120);
  delay(1000);

  // Stop
  digitalWrite(in1A, LOW);
  digitalWrite(in2A, LOW);
  digitalWrite(in1B, LOW);
  digitalWrite(in2B, LOW);
  delay(2000);
}
