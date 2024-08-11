/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
Include a link if re-using.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  delay(500);
}

void loop() {

  // Make it scroll!
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(80);

  // add the text
  const char text[] = "  Tech Talkies!  ";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}