/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <ESP32Servo.h>

Servo myservo;

void setup() {
  // put your setup code here, to run once:
  myservo.attach(13);
}

void loop() {
  // put your main code here, to run repeatedly:
  myservo.write(0);
  delay(1000);
  myservo.write(90);
  delay(1000);
}