/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_VL53L0X.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);  // PAGE BUFFER MODE (LOW RAM)
Adafruit_VL53L0X lox;

bool tofReady = false;
int16_t lastDistance = -1;

void TCA9548A(uint8_t b) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << b);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Serial.begin(115200);

  // Init display 1
  TCA9548A(2);
  u8g2.begin();

  // Init display 2
  TCA9548A(6);
  u8g2.begin();

  // Init TOF
  if (!lox.begin()) {
    Serial.println("VL53L0X missing");
  } else {
    tofReady = true;
#if defined(ADAFRUIT_VL53L0X_H)
    lox.startRangeContinuous(30);
#endif
  }
}

void loop() {

  // Read TOF (lightweight)
  if (tofReady) {
    uint16_t mm = lox.readRange();
    if (lox.timeoutOccurred()) lastDistance = -1;
    else lastDistance = mm / 10;
  }

  // ---------------- DISPLAY 1 ----------------
  TCA9548A(2);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tr);

    const char *msg = "Sensor data on";
    const char *msg2 = "other display";

    // Get text widths for centering
    int w1 = u8g2.getStrWidth(msg);
    int w2 = u8g2.getStrWidth(msg2);

    int x1 = (128 - w1) / 2;
    int x2 = (128 - w2) / 2;

    // Vertical centering: 64px tall → put two lines near middle
    u8g2.drawStr(x1, 28, msg);
    u8g2.drawStr(x2, 46, msg2);

  } while (u8g2.nextPage());

  // ---------------- DISPLAY 2 ----------------
  TCA9548A(6);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_8x13B_tr);

    // ---------- box size ----------
    const int boxW = 90;
    const int boxH = 24;

    // center the box on a 128×64 display
    int boxX = (128 - boxW) / 2;
    int boxY = (64 - boxH) / 2;

    // draw white box
    u8g2.setDrawColor(1);
    u8g2.drawBox(boxX, boxY, boxW, boxH);

    // prepare text
    char text[20];
    if (lastDistance < 0)
      strcpy(text, "DIST ---");
    else {
      // manual formatting to avoid snprintf
      // "DIST 12cm"
      strcpy(text, "DIST ");
      itoa(lastDistance, text + strlen(text), 10);
      strcat(text, "cm");
    }

    // center text inside box
    int textW = u8g2.getStrWidth(text);
    int textX = boxX + (boxW - textW) / 2;
    int textY = boxY + (boxH / 2) + 3;

    // black text inside the white box
    u8g2.setDrawColor(0);
    u8g2.drawStr(textX, textY, text);

    // restore draw color
    u8g2.setDrawColor(1);

  } while (u8g2.nextPage());
}
