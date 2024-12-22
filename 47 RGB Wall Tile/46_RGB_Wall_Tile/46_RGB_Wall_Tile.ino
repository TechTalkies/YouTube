#include "DHT.h"
#include "LedControl.h"
#include <Adafruit_NeoPixel.h>

//DO NOT change these pins
#define DIN 11
#define CLK 13

//These pins can be changed
#define DHTPIN 2
#define CS 12
#define LEDPIN 6

#define NUMPIXELS 64
#define HOTLIMIT 27  //Temperature at which hot & cold colors are swapped

uint16_t hotColor1 = 0;
uint16_t hotColor2 = 6000;
uint16_t coldColor1 = 20000;
uint16_t coldColor2 = 44000;

// Uncomment whatever type you're using!
#define DHTTYPE DHT11  // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// DataIn, CLK, CS, No. of MAX72XX
LedControl lc = LedControl(DIN, CLK, CS, 1);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 1000;
int t = 0;

static const uint8_t digitCodeMap[] = {
  // Flipped byte code DEFABCG
  // DEFABCG  Segments      7-segment map:
  0b01111110,  // 0               AAA
  0b00000110,  // 1              F   B
  0b01101101,  // 2              F   B
  0b01001111,  // 3               GGG
  0b00010111,  // 4              E   C
  0b01011011,  // 5              E   C
  0b01110011,  // 6               DDD
  0b00001110,  // 7
  0b01111111,  // 8
  0b00011111,  // 9
  0b00011101,  // Degree
  0b01111000,  // C
  0b00110111,  // H
  0b00000000,  // Space
};

void setup() {
  dht.begin();
  pixels.begin();
  pixels.setBrightness(50);
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
}

void loop() {
  int h = dht.readHumidity();
  // Comment below line to test colors
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    return;
  }

  //Code for protyping and testing colors. Uncomment to test colors
  /*
  t++;
  if (t < 10) t = 50;
  if (t > 50) t = 10;
  */

  int Tdigit1 = (t / 10) % 10;
  int Tdigit2 = t % 10;
  int Hdigit1 = (h / 10) % 10;
  int Hdigit2 = h % 10;

  lc.clearDisplay(0);
  lc.setRow(0, 0, digitCodeMap[Tdigit1]);
  lc.setRow(0, 1, digitCodeMap[Tdigit2]);
  lc.setRow(0, 2, digitCodeMap[10]);  // Degree
  lc.setRow(0, 3, digitCodeMap[11]);  // C
  lc.setRow(0, 4, digitCodeMap[13]);  // Space
  lc.setRow(0, 5, digitCodeMap[12]);  // H
  lc.setRow(0, 6, digitCodeMap[Hdigit1]);
  lc.setRow(0, 7, digitCodeMap[Hdigit2]);

  uint16_t hue;
  if (t < HOTLIMIT) {
    hue = map(t, 0, HOTLIMIT, coldColor2, coldColor1);
  } else {
    hue = map(t, HOTLIMIT, 50, hotColor2, hotColor1);
  }

  pixels.fill(pixels.ColorHSV(hue));
  pixels.show();
  delay(250);
}
