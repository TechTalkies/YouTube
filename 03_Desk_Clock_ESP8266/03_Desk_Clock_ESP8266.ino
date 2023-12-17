#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "Fonts.h"
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15

char* ssid = "wifi";              //wifi ssid
char* password = "pass";      //wifi password
const long utcOffsetInSeconds = 19800;  // Time zone offset

WiFiUDP ntpUDP;

MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds, 60000);

char Time[] = "00:00";
char Seconds[] = "00";
char Date[] = "00-00-2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

const char* DaysWeek[] = { "SUN", "MON", "TUES", "WED", "THUR", "FRI", "SAT" };

void setup() {
  Display.begin(2);

  Display.setZone(0, 1, 3);
  Display.setZone(1, 0, 0);

  /*If making changes to the display, don't use these fonts
      as they only have numbers and do not support characters*/
  Display.setFont(0, SmallDigits);
  Display.setFont(1, SmallerDigits);

  Display.setIntensity(2);
  Display.setCharSpacing(0);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Display.displayZoneText(0, ".", PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
  }

  timeClient.begin();

  Serial.begin(115200);
}

void loop() {
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();  // Get Unix epoch time from the NTP server
  int Day = timeClient.getDay();
  second_ = second(unix_epoch);
  if (last_second != second_) {
    minute_ = minute(unix_epoch);
    hour_ = hour(unix_epoch);
    day_ = day(unix_epoch);
    month_ = month(unix_epoch);
    year_ = year(unix_epoch);

    Seconds[1] = second_ % 10 + 48;
    Seconds[0] = second_ / 10 + 48;
    Time[4] = minute_ % 10 + 48;
    Time[3] = minute_ / 10 + 48;
    Time[1] = hour_ % 10 + 48;
    Time[0] = hour_ / 10 + 48;

    Date[0] = day_ / 10 + 48;
    Date[1] = day_ % 10 + 48;
    Date[3] = month_ / 10 + 48;
    Date[4] = month_ % 10 + 48;
    Date[8] = (year_ / 10) % 10 + 48;
    Date[9] = year_ % 10 % 10 + 48;

    if (second_ < 7) {
      Display.displayZoneText(0, DaysWeek[Day], PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      Display.displayZoneText(1, Date, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
    } else {
      Display.displayZoneText(0, Time, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
      Display.displayZoneText(1, Seconds, PA_LEFT, Display.getSpeed(), Display.getPause(), PA_NO_EFFECT);
    }
    Display.displayAnimate();

    last_second = second_;
  }
  delay(500);
}