/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Change to invert joystick axis
#define INVERT_Y false
#define INVERT_X true
#define SWAP_XY true

#define eyeXL 32    //Center X of Left eye
#define eyeXR 84    //Center X of Right eye
#define eyeY 32     //Center Y of both eyes
#define eyeRad 16   //Radius of pupil
#define pupilRad 7  //Radius of reflection

int16_t lookXL, lookXR, lookY;
int buttonPin = A0;
int xPin = A1;
int yPin = A2;
bool blink = false;

void setup() {
  Serial.begin(9600);

  pinMode(xPin, INPUT);
  pinMode(yPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(500);  // Pause for 2 seconds

  // Clear the buffer.
  display.clearDisplay();

  lookXL = eyeXL;
  lookXR = eyeXR;
  lookY = eyeY;
}

void loop() {
  // Clear the buffer.
  display.clearDisplay();

  read_key();

  if (!blink) {
    //Eyebrows
    display.fillTriangle(16, 14, 48, 14, 48, 12, SSD1306_WHITE);
    display.fillTriangle(68, 14, 100, 14, 68, 12, SSD1306_WHITE);

    //Eyes outer circle
    display.fillCircle(eyeXL, eyeY, eyeRad, SSD1306_WHITE);
    display.fillCircle(eyeXR, eyeY, eyeRad, SSD1306_WHITE);

    //Eyes inner circle
    display.fillCircle(lookXL, lookY, pupilRad, SSD1306_BLACK);
    display.fillCircle(lookXR, lookY, pupilRad, SSD1306_BLACK);

    //Reflection
    display.fillCircle(lookXL - 2, lookY - 2, pupilRad / 3, SSD1306_WHITE);
    display.fillCircle(lookXR - 2, lookY - 2, pupilRad / 3, SSD1306_WHITE);
  } else {
    display.drawFastHLine(eyeXL - eyeRad, eyeY, eyeRad * 2, SSD1306_WHITE);
    display.drawFastHLine(eyeXR - eyeRad, eyeY, eyeRad * 2, SSD1306_WHITE);
  }

  display.display();  // Update screen with all the above
  delay(1);
}

void read_key() {
  int offsetX = 12;
  int offsetY = 12;
  int xPosition = 0;
  int yPosition = 0;
  blink = !digitalRead(buttonPin);

  if (!SWAP_XY) {
    xPosition = analogRead(xPin);
    yPosition = analogRead(yPin);
  } else {
    xPosition = analogRead(yPin);
    yPosition = analogRead(xPin);
  }

  lookXL = eyeXL;
  lookXR = eyeXR;
  lookY = eyeY;

  if (INVERT_X) offsetX = -12;
  if (INVERT_Y) offsetY = -12;

  if (yPosition < 30) { //Serial.println("UP");
    lookY = eyeY - offsetY;
  }
  if (yPosition > 600) { //Serial.println("DOWN");
    lookY = eyeY + offsetY;
  }
  if (xPosition < 300) { //Serial.println("LEFT");
    lookXL = eyeXL - offsetX;
    lookXR = eyeXR - offsetX;
  }
  if (xPosition > 600) { //Serial.println("RIGHT");
    lookXL = eyeXL + offsetX;
    lookXR = eyeXR + offsetX;
  }
}