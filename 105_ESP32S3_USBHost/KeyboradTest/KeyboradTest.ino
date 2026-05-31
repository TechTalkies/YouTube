/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include "EspUsbHost.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

EspUsbHost usb;

// Screen size definitions
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128

// Pin definitions
#define TFT_SCK 11
#define TFT_MOSI 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 7

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// Arduino canvas buffer to avoid flicker (Sprite allocation)
GFXcanvas16 sprite(SCREEN_WIDTH, SCREEN_HEIGHT);

// Connection state status marker
static bool isConnected = false;
static uint8_t lastKeyCode = 0;
static char lastAscii = 0;

// Visual buffer for typed text size tracker
static char charBuffer[128] = "";
static uint8_t cursorIndex = 0;

// CUSTOM BACKGROUND IMAGE PLACEHOLDER (RGB565 format)
const uint16_t background_placeholder[SCREEN_WIDTH * SCREEN_HEIGHT] PROGMEM = {
  // Simple dark matrix color gradient filler
  0x0821,
  0x0821,
  0x0821,
};

// Simple background rendering function that supports fallback or image decoding
void drawBackground() {
  sprite.fillScreen(ST7735_BLACK);

  // Custom futuristic board aesthetics (Grid drawing overlay)
  for (int16_t x = 0; x < SCREEN_WIDTH; x += 16) {
    sprite.drawFastVLine(x, 0, SCREEN_HEIGHT, 0x0986);  // Subtle blue-gray grids
  }
  for (int16_t y = 0; y < SCREEN_HEIGHT; y += 16) {
    sprite.drawFastHLine(0, y, SCREEN_WIDTH, 0x0986);
  }
}

// Triggers rendering sequence
void renderFrame() {
  // 1. Draw Background Grid
  drawBackground();

  // 2. Draw Typed Buffer Panel
  sprite.fillRect(10, 24, SCREEN_WIDTH - 20, 48, 0x0104);  // Dark background panel
  sprite.drawRect(10, 24, SCREEN_WIDTH - 20, 48, 0x3ACF);  // Border outline

  sprite.setTextSize(1);
  sprite.setTextColor(0x57FF);  // Cyan
  sprite.setCursor(16, 28);
  sprite.print("KEYBOARD BUFFER:");

  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(16, 42);
  if (strlen(charBuffer) > 0) {
    sprite.print(charBuffer);
    // Draw an elegant blinking prompt cursor
    if ((millis() / 400) % 2 == 0) {
      sprite.print("_");
    }
  } else {
    sprite.setTextColor(0x5AEB);  // Muted gray
    sprite.print("Type on hardware...");
  }

  // 3. Draw Event Details Indicators
  sprite.fillRect(10, 78, SCREEN_WIDTH - 20, 42, 0x10A2);  // Darker panel
  sprite.drawRect(10, 78, SCREEN_WIDTH - 20, 42, 0x2124);

  sprite.setTextColor(0xFBE0);  // Orange
  sprite.setCursor(16, 84);
  sprite.printf("Keycode: 0x%02X (%d)", lastKeyCode, lastKeyCode);

  sprite.setCursor(16, 96);
  if (lastAscii >= 32 && lastAscii <= 126) {
    sprite.printf("ASCII Char: '%c' (0x%02X)", lastAscii, (int)lastAscii);
  } else {
    sprite.printf("ASCII Char: [Special 0x%02X]", (int)lastAscii);
  }

  // 4. Render Top Bar Status Indicators
  sprite.fillRect(0, 0, SCREEN_WIDTH, 14, 0x18C3);  // Soft slate header block
  sprite.drawFastHLine(0, 14, SCREEN_WIDTH, ST7735_WHITE);

  // Draw Connection LED indicator
  uint16_t ledColor = isConnected ? ST7735_GREEN : ST7735_RED;
  sprite.fillCircle(7, 7, 3, ledColor);

  // Text statuses
  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(16, 4);
  sprite.print(isConnected ? "KEYBOARD READY" : "NO KEYBOARD");

  if (isConnected) {
    sprite.setCursor(110, 4);
    sprite.printf("LEN:%d", cursorIndex);
  } else {
    sprite.setCursor(85, 4);
    sprite.print("DISCONNECTED");
  }

  // 5. Transfer double-buffered GFX Canvas frame to physical ST7735 display instantly
  tft.drawRGBBitmap(0, 0, sprite.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
}

void setup() {
  Serial.begin(115200);

  // Initiate connections
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  // Initial frame drawing placeholder
  renderFrame();

  usb.onDeviceConnected([](const EspUsbHostDeviceInfo &device) {
    isConnected = true;
    Serial.println("USB Device Registered and Connected.");
    renderFrame();
  });

  usb.onDeviceDisconnected([](const EspUsbHostDeviceInfo &device) {
    isConnected = false;
    Serial.println("USB Device Detached.");
    renderFrame();
  });

  usb.onKeyboard([](const EspUsbHostKeyboardEvent &event) {
    if (event.pressed) {
      lastKeyCode = event.keycode;
      lastAscii = event.ascii;

      char key = event.ascii;
      if (key >= 32 && key <= 126) {  // Printable characters
        if (cursorIndex < 120) {
          charBuffer[cursorIndex++] = key;
          charBuffer[cursorIndex] = '\0';
        }
      } else if (event.keycode == 42) {  // Backspace keycode
        if (cursorIndex > 0) {
          charBuffer[--cursorIndex] = '\0';
        }
      } else if (event.keycode == 40 || key == '\r' || key == '\n') {  // Enter key
        Serial.printf("Typed sentence: %s\n", charBuffer);
        memset(charBuffer, 0, sizeof(charBuffer));
        cursorIndex = 0;
      }
    }
  });

  if (!usb.begin()) {
    Serial.println("USB Host initialization crashed!");
  }
}

void loop() {
  renderFrame();  // Periodically repaint sprite buffer on display
  delay(16);      // Cap target framerate close to 60fps
}