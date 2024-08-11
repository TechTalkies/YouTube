/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
Include a link if re-using.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include "Arduino_LED_Matrix.h"   //Include the LED_Matrix library
#include "animation.h"            //Include animation.h header file

// Create an instance of the ArduinoLEDMatrix class
ArduinoLEDMatrix matrix;  

void setup() {
  Serial.begin(115200);
  // you can also load frames at runtime, without stopping the refresh
  matrix.loadSequence(animation);
  matrix.begin();
  // turn on autoscroll to avoid calling next() to show the next frame; the paramenter is in milliseconds
  // matrix.autoscroll(300);
  matrix.play(true);
}

void loop() {
  delay(500);
  Serial.println(millis());
}
