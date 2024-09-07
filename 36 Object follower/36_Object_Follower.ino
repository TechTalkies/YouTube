/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <Servo.h>

int pinIR1 = 4;
int pinIR2 = 5;
int pinServo = 9;
int currentPos = 0;
Servo myservo;

int leftIR;
int rightIR;

void setup() {
  Serial.begin(9600);

  pinMode(pinIR1, INPUT);
  pinMode(pinIR2, INPUT);
  myservo.attach(pinServo);

  Serial.println("Detect IR Sensor");
  delay(1000);
}

void loop() {

  if (digitalRead(pinIR1) == HIGH & digitalRead(pinIR2) == LOW) {
    //Turn right
    currentPos = currentPos - 2;
    if (currentPos < 0) {
      currentPos = 0;
    }
    myservo.write(currentPos);
    delay(20);
  } else if (digitalRead(pinIR2) == HIGH & digitalRead(pinIR1) == LOW) {
    //Turn left
    currentPos = currentPos + 2;
    if (currentPos >= 180) {
      currentPos = 180;
    }
    myservo.write(currentPos);
    delay(20);
  }
}