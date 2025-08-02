/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Rotary encoder pins
#define ENC_R_CLK 5
#define ENC_R_DT 4
#define BTN_RIGHT 3

#define ENC_L_CLK 8
#define ENC_L_DT 7
#define BTN_LEFT 6

// Brush thickness
int brushSize = 2;  // Change this to 1, 2, 3, etc.

int x = SCREEN_WIDTH / 2;
int y = SCREEN_HEIGHT / 2;

bool inResetMenu = false;

int lastEncR_CLK = HIGH;
int lastEncL_CLK = HIGH;

void drawHeader(const char* text) {
  display.fillRect(0, 0, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor((SCREEN_WIDTH - strlen(text) * 6) / 2, 1);
  display.setTextSize(1);
  display.print(text);
  display.setTextColor(SSD1306_WHITE);
}

void drawBrush(int cx, int cy, int size) {
  int half = size / 2;
  for (int dx = -half; dx <= half; dx++) {
    for (int dy = -half; dy <= half; dy++) {
      int px = constrain(cx + dx, 0, SCREEN_WIDTH - 1);
      int py = constrain(cy + dy, 11, SCREEN_HEIGHT - 1); // avoid header
      display.drawPixel(px, py, SSD1306_WHITE);
    }
  }
}

void setup() {
  pinMode(ENC_R_CLK, INPUT_PULLUP);
  pinMode(ENC_R_DT, INPUT_PULLUP);
  pinMode(ENC_L_CLK, INPUT_PULLUP);
  pinMode(ENC_L_DT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  drawHeader("ARDU-SKETCH");
  display.display();
}

void loop() {
  static bool prevBtnRight = HIGH;
  static bool prevBtnLeft = HIGH;

  bool btnRight = digitalRead(BTN_RIGHT);
  bool btnLeft = digitalRead(BTN_LEFT);

  if (inResetMenu) {
    if (btnLeft == LOW && prevBtnLeft == HIGH) {
      display.clearDisplay();
      drawHeader("ARDU-SKETCH");
      display.display();
      inResetMenu = false;
      delay(300);
    } else if (btnRight == LOW && prevBtnRight == HIGH) {
      drawHeader("ARDU-SKETCH");
      display.display();
      inResetMenu = false;
      delay(300);
    }
    prevBtnRight = btnRight;
    prevBtnLeft = btnLeft;
    return;
  }

  if (btnLeft == LOW && prevBtnLeft == HIGH) {
    inResetMenu = true;
    drawHeader("-Yes-   Reset?   -No-");
    display.display();
    delay(300);
    prevBtnLeft = btnLeft;
    return;
  }

  int encR_CLK = digitalRead(ENC_R_CLK);
  if (encR_CLK != lastEncR_CLK && encR_CLK == LOW) {
    if (digitalRead(ENC_R_DT) != encR_CLK) {
      x--;
    } else {
      x++;
    }
    x = constrain(x, 0, SCREEN_WIDTH - 1);
    drawBrush(x, y, brushSize);
    display.display();
  }
  lastEncR_CLK = encR_CLK;

  int encL_CLK = digitalRead(ENC_L_CLK);
  if (encL_CLK != lastEncL_CLK && encL_CLK == LOW) {
    if (digitalRead(ENC_L_DT) != encL_CLK) {
      y++;
    } else {
      y--;
    }
    y = constrain(y, 11, SCREEN_HEIGHT - 1);
    drawBrush(x, y, brushSize);
    display.display();
  }
  lastEncL_CLK = encL_CLK;

  prevBtnRight = btnRight;
  prevBtnLeft = btnLeft;
}
