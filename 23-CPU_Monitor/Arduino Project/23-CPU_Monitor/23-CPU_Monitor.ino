#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>
#include "ExtFont.h"

#define CLK_PIN 14   //D5
#define DATA_PIN 13  //D7
#define CS_PIN 15    //D8

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

int incomingByte = 0;  // for incoming serial data
String teststr;

void setup(void) {
  teststr = "CPU:00";
  Serial.begin(9600);  // opens serial port, sets data rate to 9600 bps
  P.begin();
  P.setIntensity(1);
  P.setFont(ExtFont);
}

void loop(void) {
  if (Serial.available() > 0) {
    teststr = Serial.readString();  //read until timeout
    teststr.trim();                 // remove any \r \n whitespace at the end of the String
    Serial.print(teststr);
  }
  if (P.displayAnimate()) {
    P.displayText(teststr.c_str(), PA_LEFT, P.getSpeed(), P.getPause(), PA_NO_EFFECT);
  }
}
