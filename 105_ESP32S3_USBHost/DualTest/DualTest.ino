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
#define SCREEN_WIDTH  160
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

// Connection state status markers
static bool mouseConnected = false;
static bool keyboardConnected = false;

// Mouse cursor position constrained to screen size
static int32_t posX = SCREEN_WIDTH / 2;
static int32_t posY = SCREEN_HEIGHT / 2 + 10;

// Coordinate and click trackers for visual overlay representation
struct ClickIndicator {
  int16_t x;
  int16_t y;
  uint16_t color;
  uint32_t timer;
};

#define MAX_CLICKS 6
static ClickIndicator clicks[MAX_CLICKS];
static uint8_t clickIdx = 0;

// Keyboard buffer states
static char charBuffer[32] = "";
static uint8_t cursorIndex = 0;
static uint8_t lastKeyCode = 0;

// Simple background rendering with grid
void drawBackground() {
  sprite.fillScreen(ST7735_BLACK);
  
  // Custom hybrid futuristic grids (dark cyan/blue)
  for (int16_t x = 0; x < SCREEN_WIDTH; x += 16) {
    sprite.drawFastVLine(x, 0, SCREEN_HEIGHT, 0x0842);
  }
  for (int16_t y = 0; y < SCREEN_HEIGHT; y += 16) {
    sprite.drawFastHLine(0, y, SCREEN_WIDTH, 0x0842);
  }
}

void addNewClick(int16_t cx, int16_t cy, uint16_t color) {
  clicks[clickIdx].x = cx;
  clicks[clickIdx].y = cy;
  clicks[clickIdx].color = color;
  clicks[clickIdx].timer = millis();
  clickIdx = (clickIdx + 1) % MAX_CLICKS;
}

// Triggers rendering sequence
void renderFrame() {
  // 1. Draw Background Grid
  drawBackground();

  // 2. Clear expired mouse clicks
  uint32_t now = millis();
  for (int i = 0; i < MAX_CLICKS; i++) {
    if (clicks[i].timer > 0 && (now - clicks[i].timer < 1000)) { // Display click circles for 1 second
      sprite.fillCircle(clicks[i].x, clicks[i].y, 5, clicks[i].color);
      sprite.drawCircle(clicks[i].x, clicks[i].y, 8, ST7735_WHITE);
    }
  }

  // 3. Draw Keyboard interactive overlay panel
  sprite.fillRect(8, 20, SCREEN_WIDTH - 16, 36, 0x0882); // Soft dark navy panel
  sprite.drawRect(8, 20, SCREEN_WIDTH - 16, 36, 0x228B); // Muted blue border
  
  sprite.setTextSize(1);
  sprite.setTextColor(ST7735_YELLOW);
  sprite.setCursor(14, 24);
  sprite.print("KEYBOARD INPUT:");

  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(14, 38);
  if (strlen(charBuffer) > 0) {
    sprite.print(charBuffer);
    if ((millis() / 400) % 2 == 0) {
      sprite.print("_");
    }
  } else {
    sprite.setTextColor(0x5AEB); // Muted gray
    sprite.print("Type key on board...");
  }

  // 4. Draw Mouse stats summary panel at raw bottom
  sprite.fillRect(8, 100, SCREEN_WIDTH - 16, 22, 0x1042);
  sprite.drawRect(8, 100, SCREEN_WIDTH - 16, 22, 0x18EA);
  sprite.setTextColor(ST7735_CYAN);
  sprite.setCursor(14, 104);
  sprite.printf("Cursor: X:%d Y:%d KC:0x%02X", posX, posY, lastKeyCode);

  // 5. Draw active Mouse Cursor
  if (mouseConnected) {
    // Elegant crosshair with center dot
    sprite.fillCircle(posX, posY, 2, ST7735_WHITE);
    sprite.drawFastHLine(posX - 6, posY, 12, ST7735_MAGENTA);
    sprite.drawFastVLine(posX, posY - 6, 12, ST7735_MAGENTA);
  }

  // 6. Render Dual Device Status Header
  sprite.fillRect(0, 0, SCREEN_WIDTH, 14, 0x18C3); // Soft slate header block
  sprite.drawFastHLine(0, 14, SCREEN_WIDTH, ST7735_WHITE);
  
  // Connection LEDs
  uint16_t mouseLED = mouseConnected ? ST7735_GREEN : ST7735_RED;
  uint16_t kbLED = keyboardConnected ? ST7735_GREEN : ST7735_RED;
  
  sprite.fillCircle(8, 7, 3, mouseLED);
  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(16, 4);
  sprite.print("MOUSE");

  sprite.fillCircle(78, 7, 3, kbLED);
  sprite.setCursor(86, 4);
  sprite.print("KEYBOARD (HUB)");

  // 7. Transfer double-buffered GFX Canvas frame to physical ST7735 display instantly
  tft.drawRGBBitmap(0, 0, sprite.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
}

void setup() {
  Serial.begin(115200);
  
  // Initiate custom connections
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  
  renderFrame();

  usb.onDeviceConnected([](const EspUsbHostDeviceInfo &device) {
    Serial.print("connected: ");
    espUsbHostPrint(device);
    renderFrame();
  });

  usb.onDeviceDisconnected([](const EspUsbHostDeviceInfo &device) {
    Serial.print("disconnected: ");
    espUsbHostPrint(device);
    mouseConnected = false;
    keyboardConnected = false;
    renderFrame();
  });

  usb.onMouse([](const EspUsbHostMouseEvent &event) {
    mouseConnected = true; // Safe fallback connection flag
    if (event.buttonsChanged) {
      uint8_t pressed = event.buttons & ~event.previousButtons;
      if (pressed & 0x01) {
        addNewClick(posX, posY, ST7735_RED);
        Serial.println("Dual Hub: Left Click Event");
      }
      if (pressed & 0x02) {
        addNewClick(posX, posY, ST7735_GREEN);
        Serial.println("Dual Hub: Right Click Event");
      }
      if (pressed & 0x04) {
        addNewClick(posX, posY, ST7735_BLUE);
        Serial.println("Dual Hub: Middle Click Event");
      }
    }
    
    if (event.moved) {
      posX += event.x;
      posY += event.y;

      // Bound clamp
      if (posX < 2) posX = 2;
      if (posX >= SCREEN_WIDTH - 2) posX = SCREEN_WIDTH - 2;
      if (posY < 16) posY = 16;
      if (posY >= 98) posY = 98; // don't overlap mouse status panel at bottom

      Serial.printf("Hub Mouse: x=%d, y=%d\n", posX, posY);
    }
  });

  usb.onKeyboard([](const EspUsbHostKeyboardEvent &event) {
    keyboardConnected = true; // Safe fallback connection flag
    if (event.pressed) {
      lastKeyCode = event.keycode;
      char key = event.ascii;
      
      if (key >= 32 && key <= 126) {
        if (cursorIndex < 24) { // keep typing buffer clean and single-line readable
          charBuffer[cursorIndex++] = key;
          charBuffer[cursorIndex] = '\0';
        }
      } else if (event.keycode == 42) { // Backspace scancode
        if (cursorIndex > 0) {
          charBuffer[--cursorIndex] = '\0';
        }
      } else if (event.keycode == 40 || key == '\r' || key == '\n') { // Enter keys
        Serial.printf("Hub Keyboard sentence: %s\n", charBuffer);
        memset(charBuffer, 0, sizeof(charBuffer));
        cursorIndex = 0;
      }
    }
  });

  if (!usb.begin()) {
    Serial.println("Hub Dual OTG Stack Initialization failed!");
  }
}

void loop() {
  renderFrame();
  delay(15); // ~60fps target speed
}
