/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Wire.h>
#include <U8g2lib.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

// Animation variables
int x = 0;
int dx = 2;

bool inverted = false;
unsigned long lastInvert = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // ---- Init Display 1 ----
  TCA9548A(2);
  if (!u8g2.begin()) Serial.println("Display 1 failed");
  else Serial.println("Display 1 init");

  // ---- Init Display 2 ----
  TCA9548A(6);
  if (!u8g2.begin()) Serial.println("Display 2 failed");
  else Serial.println("Display 2 init");
}

void loop() {

  // DISPLAY 1: Scrolling text
  TCA9548A(2);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  x += dx;
  if (x < 0 || x > (SCREEN_WIDTH - 80)) {  
    dx = -dx;
    x += dx;
  }

  u8g2.drawStr(x, 32, "HELLO FROM BUS 2");
  u8g2.sendBuffer();

  // DISPLAY 2: Full-screen invert toggle
  TCA9548A(6);

  if (millis() - lastInvert > 1000) {
    inverted = !inverted;
    lastInvert = millis();
  }

  u8g2.clearBuffer();

  if (inverted) {
    // Fill screen black and draw white text (inverted look)
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(10, 30, "INVERTED SCREEN");
    u8g2.setDrawColor(1);
  } else {
    // Normal white-on-black frame
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(10, 30, "NORMAL SCREEN");
  }

  u8g2.sendBuffer();
}
