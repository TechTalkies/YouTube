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

// Global position coordinates constrained to screen size
static int32_t posX = SCREEN_WIDTH / 2;
static int32_t posY = SCREEN_HEIGHT / 2;

// Connection state status marker
static bool isConnected = false;
static char statusMsg[32] = "Initializing...";

// Coordinate and click trackers for visual overlay representation
struct ClickIndicator {
  int16_t x;
  int16_t y;
  uint16_t color;
  uint32_t timer;
};

#define MAX_CLICKS 8
static ClickIndicator clicks[MAX_CLICKS];
static uint8_t clickIdx = 0;

// CUSTOM BACKGROUND IMAGE PLACEHOLDER (RGB565 format)
// You can replace this placeholder with an exported raw image byte array from Image2Lcd or LCD Image Converter.
// 20480 elements represent 160*128 pixels.
const uint16_t background_placeholder[SCREEN_WIDTH * SCREEN_HEIGHT] PROGMEM = {
  // Simple dark matrix color gradient filler
  0x0821, 0x0821, 0x0821,  // ... Added placeholder background pixels
};

// Simple background rendering function that supports fallback or image decoding
void drawBackground() {
  sprite.fillScreen(ST7735_BLACK);

  // Uncomment the line below once your exported image array 'background_placeholder' contains valid data!
  // sprite.drawRGBBitmap(0, 0, (const uint16_t*)background_placeholder, SCREEN_WIDTH, SCREEN_HEIGHT);

  // Custom futuristic board aesthetics (Grid drawing overlay)
  for (int16_t x = 0; x < SCREEN_WIDTH; x += 16) {
    sprite.drawFastVLine(x, 0, SCREEN_HEIGHT, 0x10A2);  // Subtle cyan grids
  }
  for (int16_t y = 0; y < SCREEN_HEIGHT; y += 16) {
    sprite.drawFastHLine(0, y, SCREEN_WIDTH, 0x10A2);
  }
}

// Triggers rendering sequence
void renderFrame() {
  // 1. Draw Background Grid or Custom image
  drawBackground();

  // 2. Clear expired mouse clicks
  uint32_t now = millis();
  for (int i = 0; i < MAX_CLICKS; i++) {
    if (clicks[i].timer > 0 && (now - clicks[i].timer < 1000)) {  // Display click circles for 1 second
      sprite.fillCircle(clicks[i].x, clicks[i].y, 6, clicks[i].color);
      sprite.drawCircle(clicks[i].x, clicks[i].y, 10, ST7735_WHITE);
    }
  }

  // 3. Draw Cursor representation (simple classic pointer)
  if (isConnected) {
    // Draw cursor center dot
    sprite.fillCircle(posX, posY, 3, ST7735_WHITE);
    // Draw crosshairs
    sprite.drawFastHLine(posX - 8, posY, 16, ST7735_CYAN);
    sprite.drawFastVLine(posX, posY - 8, 16, ST7735_CYAN);
  }

  // 4. Render Top Bar Status Indicators
  sprite.fillRect(0, 0, SCREEN_WIDTH, 14, 0x18C3);  // Soft slate header block
  sprite.drawFastHLine(0, 14, SCREEN_WIDTH, ST7735_WHITE);

  // Draw Connection LED indicator
  uint16_t ledColor = isConnected ? ST7735_GREEN : ST7735_RED;
  sprite.fillCircle(7, 7, 3, ledColor);

  // Text statuses
  sprite.setTextSize(1);
  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(16, 4);
  sprite.print(isConnected ? "USB READY" : "NO MOUSE");

  if (isConnected) {
    sprite.setCursor(95, 4);
    sprite.printf("X:%d Y:%d", posX, posY);
  } else {
    sprite.setCursor(75, 4);
    sprite.print("DISCONNECTED");
  }

  // 5. Transfer double-buffered GFX Canvas frame to physical ST7735 display instantly
  tft.drawRGBBitmap(0, 0, sprite.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
}

void addNewClick(int16_t cx, int16_t cy, uint16_t color) {
  clicks[clickIdx].x = cx;
  clicks[clickIdx].y = cy;
  clicks[clickIdx].color = color;
  clicks[clickIdx].timer = millis();
  clickIdx = (clickIdx + 1) % MAX_CLICKS;
}

void setup() {
  Serial.begin(115200);

  // Initiate custom connections
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

  usb.onMouse([](const EspUsbHostMouseEvent &event) {
    if (event.buttonsChanged) {
      uint8_t pressed = event.buttons & ~event.previousButtons;

      // Left Button Press -> Red Circle overlay
      if (pressed & 0x01) {
        addNewClick(posX, posY, ST7735_RED);
        Serial.println("Click: LEFT pressed");
      }
      // Right Button Press -> Green Circle overlay
      if (pressed & 0x02) {
        addNewClick(posX, posY, ST7735_GREEN);
        Serial.println("Click: RIGHT pressed");
      }
      // Middle Button Press -> Blue Circle overlay
      if (pressed & 0x04) {
        addNewClick(posX, posY, ST7735_BLUE);
        Serial.println("Click: MIDDLE pressed");
      }
    }

    if (event.moved) {
      // Accumulate relative mouse displacement variables
      posX += event.x;
      posY += event.y;

      // Keep position clamped exactly within screen resolution bounds safely (leaving margin for status-bar)
      if (posX < 2) posX = 2;
      if (posX >= SCREEN_WIDTH - 2) posX = SCREEN_WIDTH - 2;
      if (posY < 16) posY = 16;  // Avoid overlapping status bar top header
      if (posY >= SCREEN_HEIGHT - 2) posY = SCREEN_HEIGHT - 2;

      Serial.printf("cursor: x=%d, y=%d\n", posX, posY);
    }
  });

  if (!usb.begin()) {
    Serial.println("USB Host initialization crashed!");
  }
}

void loop() {
  renderFrame();  // Periodically repaint sprite buffer on the display
  delay(16);      // Cap target framerate close to 60fps
}