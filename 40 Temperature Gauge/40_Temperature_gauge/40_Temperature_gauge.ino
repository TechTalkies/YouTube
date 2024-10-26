/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <SPI.h>
#include <Wire.h>
#include "DHT.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 2      // Digital pin connected to the DHT sensor
#define BUTTON_PIN 6  // Digital pin connected to the button/ touch sensor

// Uncomment whatever type you're using!
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

DHT dht(DHTPIN, DHTTYPE);
Servo myservo;

bool fahrenheitMode = false;
bool useSerial = true;
bool valueSwitch = true;
int pos = 0;
unsigned long previousMillis = 0;

void setup() {
  //Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }      //else Serial.println(F("SSD1306 success"));

  dht.begin();
  myservo.attach(9);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  delay(500);
  myservo.write(pos);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    {
      if (digitalRead(BUTTON_PIN) == HIGH) {
        valueSwitch = !valueSwitch;
        delay(250);
      }

      previousMillis = currentMillis;  // Save the last time you read the sensor
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      int h = int(dht.readHumidity());
      int t = int(dht.readTemperature());

      if (fahrenheitMode)
        t = dht.readTemperature(true);

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
      }

      if (valueSwitch)
        pos = map(t, 0, 50, 0, 180);
      else
        pos = map(h, 0, 100, 0, 180);

      //Servos usually go anti-clockwise. We need to invert it.
      pos = 180 - pos;

      myservo.write(pos);

      display.clearDisplay();

      if (valueSwitch)
        display.fillRect(0, 0, 64, 64, SSD1306_WHITE);
      else
        display.fillRect(63, 0, 64, 64, SSD1306_WHITE);

      if (valueSwitch)
        display.setTextColor(SSD1306_BLACK);
      else
        display.setTextColor(SSD1306_WHITE);
      display.setCursor(7, 0);
      display.print("Temp");
      display.setCursor(7, 30);
      display.print(t);
      display.print("`C");

      if (valueSwitch)
        display.setTextColor(SSD1306_WHITE);
      else
        display.setTextColor(SSD1306_BLACK);
      display.setCursor(75, 0);
      display.print("Hum");
      display.setCursor(75, 30);
      display.print(h);
      display.print("%");

      display.display();
    }
  }
}
