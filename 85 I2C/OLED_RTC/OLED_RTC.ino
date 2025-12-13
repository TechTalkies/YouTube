/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS1307RTC.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float ahtValue;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  delay(100);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Ready");
  display.display();
}

void loop() {

  tmElements_t tm;
  char buf[16];
  char dateBuf[16];

  if (RTC.read(tm)) {
    sprintf(buf, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
    snprintf(dateBuf, sizeof(dateBuf),
             "%02d/%02d/%04d", tm.Day, tm.Month, tmYearToCalendar(tm.Year));

    display.clearDisplay();

    display.setCursor(10, 0);

    display.println(buf);
    display.println("");
    display.println(dateBuf);
    //display.print("T: ");
    //display.println(temp.temperature);

    display.display();
  }
  delay(1000);
}
