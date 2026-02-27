/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Arduino.h>
#include <Wire.h>
#include <Seeed_Arduino_SSCMA.h>

#define LEDPIN 8

SSCMA AI;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(1000);
  Wire.begin();
  AI.begin(&Wire);

  pinMode(LEDPIN, OUTPUT);
}

void loop() {
  if (!AI.invoke(1, false, false)) {
    Serial.println("Invoked");
    if (AI.boxes().size() > 0) {
      Serial.println("Detected");
      digitalWrite(LEDPIN, HIGH);
    } else
      digitalWrite(LEDPIN, LOW);
  }
}