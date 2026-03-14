/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <ESP32Servo.h>

Servo myservo, myservo2;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(13);
  myservo2.attach(12);
}

void loop() {
  // put your main code here, to run repeatedly:
  myservo.write(0);
  myservo2.write(0);
  delay(1000);
  myservo.write(90);
  myservo2.write(90);
  delay(1000);
}