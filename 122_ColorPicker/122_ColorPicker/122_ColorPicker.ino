/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_TCS34725.h>

// ============================================================
// PIN DEFINITIONS
// ============================================================

#define TFT_CS 8
#define TFT_DC 9
#define TFT_RST 4
#define TFT_MOSI 44
#define TFT_SCLK 7

#define TCS_SDA 5
#define TCS_SCL 6
#define PIN_NUM_TCS_LED 4

// Physical Pin Numbers
#define PIN_BTN_L 1
#define PIN_BTN_C 2
#define PIN_BTN_R 43

// Logical Button IDs
#define BTN_L 0
#define BTN_C 1
#define BTN_R 2
#define BTN_COUNT 3

// ============================================================
// SENSOR CONFIGURATION
// ============================================================

bool swapSensorGB = false;

// ============================================================
// DISPLAY & CANVAS BUFFERING
// ============================================================

#define TFT_WIDTH 172
#define TFT_HEIGHT 320
#define SCREEN_W 320
#define SCREEN_H 172

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI, TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(SCREEN_W, SCREEN_H);

// ============================================================
// COLOR SENSOR & USB HID
// ============================================================

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_4X);
USBHIDKeyboard Keyboard;

// ============================================================
// STORAGE & SETTINGS
// ============================================================

Preferences preferences;

#define MAX_SAVED_COLORS 50
uint32_t savedColors[MAX_SAVED_COLORS];
int savedCount = 0;

#define LED_MODE_AUTO 0
#define LED_MODE_ON 1
#define LED_MODE_OFF 2

struct AppSettings {
  uint8_t led_mode;
  uint8_t auto_overwrite;  // 0 = OFF, 1 = ON
};

AppSettings g_settings = { LED_MODE_AUTO, 0 };

// ============================================================
// STATE VARIABLES
// ============================================================

uint8_t currentR = 0;
uint8_t currentG = 0;
uint8_t currentB = 0;

enum UIState {
  UI_BOOT_MENU,
  UI_SCAN_IDLE,
  UI_SCAN_LIVE,
  UI_SCAN_RESULT,
  UI_SCAN_SAVED,
  UI_SCAN_FULL,
  UI_BROWSE,
  UI_BROWSE_ACTION,
  UI_BROWSE_TYPED,
  UI_SETTINGS_MENU,
  UI_SETTINGS_LED,
  UI_SETTINGS_OVERWRITE
};

UIState uiState = UI_BOOT_MENU;
int menuSelection = 0;      // 0=Scan, 1=Browse, 2=Settings
int settingsSelection = 0;  // 0=LED Mode, 1=Auto Overwrite
int browseIndex = 0;
bool redraw = true;

unsigned long stateStartTime = 0;
unsigned long lastSensorRead = 0;
#define SENSOR_INTERVAL 150
#define MESSAGE_TIMEOUT 1200
#define LONG_PRESS_MS 600

// ============================================================
// BUTTON SYSTEM
// ============================================================

enum ButtonEvent { BUTTON_PRESS_SHORT,
                   BUTTON_PRESS_LONG };
struct ButtonMessage {
  uint8_t button;
  ButtonEvent event;
};

struct ButtonState {
  uint8_t pin;
  bool rawState;
  bool stableState;
  uint8_t stableCounter;
  unsigned long pressStartTime;
  bool longPressFired;
};

ButtonState buttonStates[BTN_COUNT];
QueueHandle_t buttonQueue;

// ============================================================
// HELPER FUNCTIONS
// ============================================================

uint16_t getTextColor(uint8_t r, uint8_t g, uint8_t b) {
  float luminance = 0.299f * r + 0.587f * g + 0.114f * b;
  return (luminance > 140.0f) ? tft.color565(0, 0, 0) : tft.color565(255, 255, 255);
}

uint32_t packColor(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void unpackColor(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (color >> 16) & 0xFF;
  g = (color >> 8) & 0xFF;
  b = color & 0xFF;
}

// ============================================================
// STORAGE FUNCTIONS
// ============================================================

void loadData() {
  preferences.begin("irlcolors", false);
  savedCount = preferences.getInt("count", 0);
  if (savedCount < 0 || savedCount > MAX_SAVED_COLORS) savedCount = 0;

  for (int i = 0; i < savedCount; i++) {
    char key[8];
    snprintf(key, sizeof(key), "c%d", i);
    savedColors[i] = preferences.getUInt(key, 0);
  }

  size_t settingsSize = preferences.getBytesLength("settings");
  if (settingsSize == sizeof(AppSettings)) {
    preferences.getBytes("settings", &g_settings, sizeof(AppSettings));
  }
}

void saveData() {
  preferences.putInt("count", savedCount);
  for (int i = 0; i < savedCount; i++) {
    char key[8];
    snprintf(key, sizeof(key), "c%d", i);
    preferences.putUInt(key, savedColors[i]);
  }
  preferences.putBytes("settings", &g_settings, sizeof(AppSettings));
}

bool saveCurrentColor() {
  if (savedCount >= MAX_SAVED_COLORS) {
    if (g_settings.auto_overwrite == 1) {
      // Shift array left to delete the oldest color
      for (int i = 0; i < MAX_SAVED_COLORS - 1; i++) {
        savedColors[i] = savedColors[i + 1];
      }
      savedColors[MAX_SAVED_COLORS - 1] = packColor(currentR, currentG, currentB);
      saveData();
      return true;
    } else {
      return false;  // Storage full, overwrite disabled
    }
  }

  savedColors[savedCount++] = packColor(currentR, currentG, currentB);
  saveData();
  return true;
}

void deleteCurrentColor() {
  if (savedCount == 0) return;
  for (int i = browseIndex; i < savedCount - 1; i++) {
    savedColors[i] = savedColors[i + 1];
  }
  savedCount--;
  if (savedCount == 0) browseIndex = 0;
  else if (browseIndex >= savedCount) browseIndex = savedCount - 1;
  saveData();
}

// ============================================================
// HARDWARE CONTROL
// ============================================================

void updateSensorLED(UIState state) {
  if (g_settings.led_mode == LED_MODE_ON) {
    digitalWrite(PIN_NUM_TCS_LED, HIGH);
  } else if (g_settings.led_mode == LED_MODE_OFF) {
    digitalWrite(PIN_NUM_TCS_LED, LOW);
  } else {
    digitalWrite(PIN_NUM_TCS_LED, (state == UI_SCAN_LIVE) ? HIGH : LOW);
  }
}

void enterState(UIState newState) {
  uiState = newState;
  stateStartTime = millis();
  updateSensorLED(newState);
  redraw = true;
}

// ============================================================
// COLOR PROCESSING
// ============================================================

void readColor() {
  uint16_t r, g, b, c;
  const float BRIGHTNESS_BOOST = 1.30f;
  const float SATURATION_BOOST = 1.40f;

  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {
    currentR = currentG = currentB = 0;
    return;
  }

  float normR = ((float)r / (float)c) * 255.0f;
  float normG = ((float)g / (float)c) * 255.0f;
  float normB = ((float)b / (float)c) * 255.0f;

  normR *= BRIGHTNESS_BOOST;
  normG *= BRIGHTNESS_BOOST;
  normB *= BRIGHTNESS_BOOST;

  float lum = 0.299f * normR + 0.587f * normG + 0.114f * normB;

  normR = lum + (normR - lum) * SATURATION_BOOST;
  normG = lum + (normG - lum) * SATURATION_BOOST;
  normB = lum + (normB - lum) * SATURATION_BOOST;

  if (normR < 0.0f) normR = 0.0f;
  else if (normR > 255.0f) normR = 255.0f;
  if (normG < 0.0f) normG = 0.0f;
  else if (normG > 255.0f) normG = 255.0f;
  if (normB < 0.0f) normB = 0.0f;
  else if (normB > 255.0f) normB = 255.0f;

  if (swapSensorGB) {
    currentR = (uint8_t)normR;
    currentG = (uint8_t)normB;
    currentB = (uint8_t)normG;
  } else {
    currentR = (uint8_t)normR;
    currentG = (uint8_t)normG;
    currentB = (uint8_t)normB;
  }
}

// ============================================================
// DRAWING ROUTINES (USING CANVAS)
// ============================================================

void drawCenteredText(const char *text, int y, uint16_t color, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;
  canvas.setTextSize(size);
  canvas.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  int x = (SCREEN_W - w) / 2;
  if (x < 0) x = 0;
  canvas.setCursor(x, y);
  canvas.setTextColor(color);
  canvas.print(text);
}

void drawRightText(const char *text, int y, uint16_t color, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;
  canvas.setTextSize(size);
  canvas.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  int x = SCREEN_W - w - 10;
  if (x < 0) x = 0;
  canvas.setCursor(x, y);
  canvas.setTextColor(color);
  canvas.print(text);
}

void drawBottomLabels(const char *l_lbl, const char *c_lbl, const char *r_lbl, uint16_t color) {
  int y = SCREEN_H - 24;
  canvas.setTextSize(2);
  canvas.setTextColor(color);

  if (l_lbl && strlen(l_lbl) > 0) {
    canvas.setCursor(10, y);
    canvas.print(l_lbl);
  }
  if (c_lbl && strlen(c_lbl) > 0) {
    drawCenteredText(c_lbl, y, color, 2);
  }
  if (r_lbl && strlen(r_lbl) > 0) {
    drawRightText(r_lbl, y, color, 2);
  }
}

void renderUI() {
  uint16_t colorMenuBg = tft.color565(50, 50, 50);
  uint16_t colorFg = tft.color565(255, 255, 255);

  switch (uiState) {
    case UI_BOOT_MENU:
      {
        canvas.fillScreen(colorMenuBg);
        const char *menus[] = { "SCAN", "BROWSE", "SETTINGS" };
        for (int i = 0; i < 3; i++) {
          char buf[20];
          snprintf(buf, sizeof(buf), "%s%s", menuSelection == i ? ">" : " ", menus[i]);
          drawCenteredText(buf, 30 + i * 35, colorFg, 3);
        }
        drawBottomLabels("", "SELECT", "NEXT", colorFg);
        break;
      }

    case UI_SCAN_IDLE:
      {
        canvas.fillScreen(colorMenuBg);
        drawCenteredText("READY", 60, colorFg, 3);
        drawCenteredText("PRESS TO START", 95, colorFg, 2);
        drawBottomLabels("BACK", "", "START", colorFg);
        break;
      }

    case UI_SCAN_LIVE:
      {
        uint16_t bg = tft.color565(currentR, currentG, currentB);
        uint16_t fg = getTextColor(currentR, currentG, currentB);
        canvas.fillScreen(bg);
        char hex[10];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", currentR, currentG, currentB);
        drawCenteredText(hex, 60, fg, 3);
        drawBottomLabels("", "", "STOP", fg);
        break;
      }

    case UI_SCAN_RESULT:
      {
        uint16_t bg = tft.color565(currentR, currentG, currentB);
        uint16_t fg = getTextColor(currentR, currentG, currentB);
        canvas.fillScreen(bg);
        char hex[10];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", currentR, currentG, currentB);
        drawCenteredText(hex, 50, fg, 3);
        drawBottomLabels("DISCARD", "SAVE", "", fg);
        break;
      }

    case UI_SCAN_SAVED:
      {
        uint16_t bg = tft.color565(currentR, currentG, currentB);
        uint16_t fg = getTextColor(currentR, currentG, currentB);
        canvas.fillScreen(bg);
        drawCenteredText("SAVED.", 75, fg, 3);
        break;
      }

    case UI_SCAN_FULL:
      {
        canvas.fillScreen(tft.color565(200, 0, 0));  // Red error screen
        drawCenteredText("STORAGE", 50, colorFg, 3);
        drawCenteredText("FULL!", 90, colorFg, 3);
        break;
      }

    case UI_BROWSE:
      {
        if (savedCount == 0) {
          canvas.fillScreen(colorMenuBg);
          drawCenteredText("NO COLORS", 75, colorFg, 3);
          drawBottomLabels("BACK", "", "", colorFg);
          break;
        }
        uint8_t r, g, b;
        unpackColor(savedColors[browseIndex], r, g, b);
        uint16_t bg = tft.color565(r, g, b);
        uint16_t fg = getTextColor(r, g, b);
        canvas.fillScreen(bg);

        char hex[10], idx[16];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
        snprintf(idx, sizeof(idx), "%d/%d", browseIndex + 1, savedCount);

        drawCenteredText(hex, 40, fg, 3);
        drawCenteredText(idx, 80, fg, 2);

        drawBottomLabels("PREV", "ACTION", "NEXT", fg);
        break;
      }

    case UI_BROWSE_ACTION:
      {
        uint8_t r, g, b;
        unpackColor(savedColors[browseIndex], r, g, b);
        uint16_t bg = tft.color565(r, g, b);
        uint16_t fg = getTextColor(r, g, b);
        canvas.fillScreen(bg);

        char hex[10];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);
        drawCenteredText(hex, 40, fg, 3);
        drawCenteredText("OPTIONS", 80, fg, 2);

        drawBottomLabels("CANCEL", "TYPE", "DELETE", fg);
        break;
      }

    case UI_BROWSE_TYPED:
      {
        uint8_t r, g, b;
        unpackColor(savedColors[browseIndex], r, g, b);
        uint16_t bg = tft.color565(r, g, b);
        uint16_t fg = getTextColor(r, g, b);
        canvas.fillScreen(bg);

        char hex[10];
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);

        drawCenteredText(hex, 40, fg, 3);
        drawCenteredText("SENT TO PC!", 80, fg, 2);
        break;
      }

    case UI_SETTINGS_MENU:
      {
        canvas.fillScreen(colorMenuBg);
        const char *menus[] = { "LED MODE", "AUTO OVERWRITE" };
        for (int i = 0; i < 2; i++) {
          char buf[20];
          snprintf(buf, sizeof(buf), "%s%s", settingsSelection == i ? ">" : " ", menus[i]);
          drawCenteredText(buf, 40 + i * 40, colorFg, 2);
        }
        drawBottomLabels("BACK", "SELECT", "NEXT", colorFg);
        break;
      }

    case UI_SETTINGS_LED:
      {
        canvas.fillScreen(colorMenuBg);
        const char *mode = (g_settings.led_mode == LED_MODE_AUTO) ? "AUTO" : (g_settings.led_mode == LED_MODE_ON) ? "ON"
                                                                                                                  : "OFF";
        drawCenteredText("SENSOR LED", 40, colorFg, 2);
        drawCenteredText(mode, 80, colorFg, 3);
        drawBottomLabels("BACK", "SAVE", "TOGGLE", colorFg);
        break;
      }

    case UI_SETTINGS_OVERWRITE:
      {
        canvas.fillScreen(colorMenuBg);
        const char *mode = (g_settings.auto_overwrite == 1) ? "ON" : "OFF";
        drawCenteredText("AUTO OVERWRITE", 40, colorFg, 2);
        drawCenteredText(mode, 80, colorFg, 3);
        drawBottomLabels("BACK", "SAVE", "TOGGLE", colorFg);
        break;
      }
  }

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_W, SCREEN_H);
}

// ============================================================
// BUTTON TASK & HANDLING
// ============================================================

void buttonTask(void *parameter) {
  while (true) {
    for (int i = 0; i < BTN_COUNT; i++) {
      ButtonState &btn = buttonStates[i];
      bool raw = digitalRead(btn.pin) == LOW;

      // Debounce logic
      if (raw == btn.rawState) {
        if (btn.stableCounter < 2) btn.stableCounter++;
      } else {
        btn.rawState = raw;
        btn.stableCounter = 0;
      }

      // State changes
      if (btn.stableCounter >= 2 && raw != btn.stableState) {
        btn.stableState = raw;
        if (raw) {
          // Button just pressed
          btn.pressStartTime = millis();
          btn.longPressFired = false;
        } else {
          // Button just released
          if (!btn.longPressFired) {
            ButtonMessage message = { (uint8_t)i, BUTTON_PRESS_SHORT };
            xQueueSend(buttonQueue, &message, 0);
          }
        }
      } else if (raw && btn.stableState) {
        // Button is being held down
        if (!btn.longPressFired && (millis() - btn.pressStartTime > LONG_PRESS_MS)) {
          btn.longPressFired = true;
          ButtonMessage message = { (uint8_t)i, BUTTON_PRESS_LONG };
          xQueueSend(buttonQueue, &message, 0);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void handleButton(const ButtonMessage &message) {

  // GLOBAL LONG PRESS HANDLER
  // A long press on the Center button from almost anywhere dumps you back to the Main Menu
  if (message.button == BTN_C && message.event == BUTTON_PRESS_LONG) {
    if (uiState != UI_BOOT_MENU && uiState != UI_SCAN_LIVE) {
      enterState(UI_BOOT_MENU);
      return;
    }
  }

  if (message.event != BUTTON_PRESS_SHORT) return;  // All normal navigation is on short press

  switch (uiState) {
    case UI_BOOT_MENU:
      if (message.button == BTN_R) menuSelection = (menuSelection + 1) % 3;
      else if (message.button == BTN_C) {
        if (menuSelection == 0) enterState(UI_SCAN_IDLE);
        else if (menuSelection == 1) {
          browseIndex = 0;
          enterState(UI_BROWSE);
        } else if (menuSelection == 2) enterState(UI_SETTINGS_MENU);
      }
      redraw = true;
      break;

    case UI_SCAN_IDLE:
      if (message.button == BTN_L) enterState(UI_BOOT_MENU);
      else if (message.button == BTN_R) {
        enterState(UI_SCAN_LIVE);
        readColor();
        lastSensorRead = millis();
      }
      break;

    case UI_SCAN_LIVE:
      if (message.button == BTN_R) enterState(UI_SCAN_RESULT);
      break;

    case UI_SCAN_RESULT:
      if (message.button == BTN_L) enterState(UI_SCAN_IDLE);
      else if (message.button == BTN_C) {
        if (saveCurrentColor()) {
          enterState(UI_SCAN_SAVED);
        } else {
          enterState(UI_SCAN_FULL);
        }
      }
      break;

    case UI_BROWSE:
      if (message.button == BTN_L) {
        if (savedCount > 0) browseIndex = (browseIndex - 1 < 0) ? savedCount - 1 : browseIndex - 1;
        else enterState(UI_BOOT_MENU);
        redraw = true;
      } else if (savedCount > 0) {
        if (message.button == BTN_R) {
          browseIndex = (browseIndex + 1) % savedCount;
          redraw = true;
        } else if (message.button == BTN_C) enterState(UI_BROWSE_ACTION);
      }
      break;

    case UI_BROWSE_ACTION:
      if (message.button == BTN_L) enterState(UI_BROWSE);
      else if (message.button == BTN_R) {
        deleteCurrentColor();
        enterState(UI_BROWSE);
      } else if (message.button == BTN_C) {
        char hex[10];
        uint8_t r, g, b;
        unpackColor(savedColors[browseIndex], r, g, b);
        snprintf(hex, sizeof(hex), "#%02X%02X%02X", r, g, b);

        Keyboard.print(hex);
        enterState(UI_BROWSE_TYPED);
      }
      break;

    case UI_SETTINGS_MENU:
      if (message.button == BTN_L) enterState(UI_BOOT_MENU);
      else if (message.button == BTN_R) settingsSelection = (settingsSelection + 1) % 2;
      else if (message.button == BTN_C) {
        if (settingsSelection == 0) enterState(UI_SETTINGS_LED);
        else if (settingsSelection == 1) enterState(UI_SETTINGS_OVERWRITE);
      }
      redraw = true;
      break;

    case UI_SETTINGS_LED:
      if (message.button == BTN_L) enterState(UI_SETTINGS_MENU);
      else if (message.button == BTN_R) {
        g_settings.led_mode = (g_settings.led_mode + 1) % 3;
        updateSensorLED(uiState);
        redraw = true;
      } else if (message.button == BTN_C) {
        saveData();
        enterState(UI_SETTINGS_MENU);
      }
      break;

    case UI_SETTINGS_OVERWRITE:
      if (message.button == BTN_L) enterState(UI_SETTINGS_MENU);
      else if (message.button == BTN_R) {
        g_settings.auto_overwrite = (g_settings.auto_overwrite == 1) ? 0 : 1;
        redraw = true;
      } else if (message.button == BTN_C) {
        saveData();
        enterState(UI_SETTINGS_MENU);
      }
      break;

    default:
      break;
  }
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);

  pinMode(PIN_NUM_TCS_LED, OUTPUT);
  digitalWrite(PIN_NUM_TCS_LED, LOW);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(3);
  tft.setSPISpeed(40000000);

  tft.fillScreen(ST77XX_BLACK);
  canvas.fillScreen(0x0000);

  Wire.begin(TCS_SDA, TCS_SCL);
  Wire.setClock(400000);

  if (!tcs.begin()) {
    Serial.println("TCS34725 NOT FOUND");
    drawCenteredText("SENSOR ERROR", 70, tft.color565(255, 0, 0), 2);
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_W, SCREEN_H);
    while (true) delay(1000);
  }

  loadData();

  uint8_t pins[BTN_COUNT] = { PIN_BTN_L, PIN_BTN_C, PIN_BTN_R };
  for (int i = 0; i < BTN_COUNT; i++) {
    buttonStates[i].pin = pins[i];
    pinMode(pins[i], INPUT_PULLUP);
    bool state = digitalRead(pins[i]) == LOW;
    buttonStates[i].rawState = state;
    buttonStates[i].stableState = state;
    buttonStates[i].stableCounter = 0;
    buttonStates[i].longPressFired = false;
  }

  buttonQueue = xQueueCreate(8, sizeof(ButtonMessage));
  xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 4096, NULL, 5, NULL, 0);

  // Initialize Composite USB (Serial + Keyboard)
  Keyboard.begin();
  USB.begin();

  enterState(UI_BOOT_MENU);
}

// ============================================================
// LOOP
// ============================================================

void loop() {
  ButtonMessage message;
  if (xQueueReceive(buttonQueue, &message, pdMS_TO_TICKS(5)) == pdTRUE) {
    handleButton(message);
  }

  if (uiState == UI_SCAN_LIVE) {
    if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
      readColor();
      lastSensorRead = millis();
      redraw = true;
    }
  }

  if (uiState == UI_SCAN_SAVED || uiState == UI_SCAN_FULL) {
    if (millis() - stateStartTime >= MESSAGE_TIMEOUT) {
      enterState(UI_SCAN_IDLE);
    }
  }

  if (uiState == UI_BROWSE_TYPED) {
    if (millis() - stateStartTime >= MESSAGE_TIMEOUT) {
      enterState(UI_BROWSE);
    }
  }

  if (redraw) {
    renderUI();
    redraw = false;
  }

  vTaskDelay(pdMS_TO_TICKS(5));
}