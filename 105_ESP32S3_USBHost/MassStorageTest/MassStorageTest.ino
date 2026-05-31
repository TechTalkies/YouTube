/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include "EspUsbHost.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

EspUsbHostMscFS usbMassStorage;
EspUsbHost usb;

// ── Display ────────────────────────────────────────────────────────────────────
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128
#define TFT_SCK 11
#define TFT_MOSI 10
#define TFT_DC 9
#define TFT_RST 8
#define TFT_CS 7

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 sprite(SCREEN_WIDTH, SCREEN_HEIGHT);

// ── State ──────────────────────────────────────────────────────────────────────
static bool isMscConnected = false;
static bool isMouseConnected = false;

static int32_t posX = SCREEN_WIDTH / 2;
static int32_t posY = SCREEN_HEIGHT / 2;

struct FileItem {
  char name[48];
  char fullPath[128];
  bool isDirectory;
  uint32_t size;
};

#define MAX_DIR_ITEMS 12
static FileItem currentDirItems[MAX_DIR_ITEMS];
static int currentDirItemsCount = 0;
static char currentPath[128] = "/";

static char activeFileViewContent[256] = "";
static uint32_t viewTimer = 0;
static uint32_t lastMountAttemptMs = 0;

// ── Path helpers ───────────────────────────────────────────────────────────────

// Build parent path of a given clean /-rooted path
static void parentOf(const char *path, char *out, size_t outSize) {
  strncpy(out, path, outSize - 1);
  out[outSize - 1] = '\0';

  // Remove trailing slash (unless root)
  size_t len = strlen(out);
  if (len > 1 && out[len - 1] == '/') out[--len] = '\0';

  char *slash = strrchr(out, '/');
  if (!slash || slash == out) {
    strcpy(out, "/");
  } else {
    *slash = '\0';
  }
}

// Build child path: join parent + name into out
static void joinPath(const char *parent, const char *name, char *out, size_t outSize) {
  if (strcmp(parent, "/") == 0) {
    snprintf(out, outSize, "/%s", name);
  } else {
    snprintf(out, outSize, "%s/%s", parent, name);
  }
}

// ── Directory loader ───────────────────────────────────────────────────────────
// Debug message shown on screen instead of Serial
static char debugMsg[128] = "Ready";

void showDebug(const char *msg) {
  strncpy(debugMsg, msg, sizeof(debugMsg) - 1);
  debugMsg[sizeof(debugMsg) - 1] = '\0';

  // Draw directly to sprite and push — called outside normal renderFrame
  sprite.fillRect(4, 116, SCREEN_WIDTH - 8, 8, 0xF800);  // red bar
  sprite.setTextSize(1);
  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(6, 117);
  sprite.print(debugMsg);
  tft.drawRGBBitmap(0, 0, sprite.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
  delay(1500);  // hold long enough to read
}

// Always open root as "/" — the FS wrapper handles the /usb mount point
// But subdirectory entries need the full /usb/ prefix to reopen
void loadDirectory(const char *path) {
  currentDirItemsCount = 0;
  strncpy(currentPath, path, sizeof(currentPath) - 1);
  currentPath[sizeof(currentPath) - 1] = '\0';

  bool isRoot = (strcmp(path, "/") == 0);

  if (!isRoot) {
    char parent[128];
    parentOf(path, parent, sizeof(parent));
    strcpy(currentDirItems[0].name, ".. [Back]");
    strcpy(currentDirItems[0].fullPath, parent);
    currentDirItems[0].isDirectory = true;
    currentDirItems[0].size = 0;
    currentDirItemsCount = 1;
  }

  if (!usbMassStorage.mounted()) {
    showDebug("ERR: not mounted");
    return;
  }

  File root = usbMassStorage.open(path);
  if (!root || !root.isDirectory()) {
    char msg[64];
    snprintf(msg, sizeof(msg), "ERR:'%.40s'", path);
    showDebug(msg);
    if (root) root.close();
    return;
  }
  root.rewindDirectory();

  while (currentDirItemsCount < MAX_DIR_ITEMS) {
    File entry = root.openNextFile();
    if (!entry) break;

    const char *rawName = entry.name();

    // ── Normalise: strip /usb prefix if present ──────────────────────────
    // entry.name() may return "/usb/FOLDER" or "/FOLDER" or "FOLDER"
    // We want a clean FS-relative path starting with /
    const char *normalized = rawName;
    if (strncmp(normalized, "/usb/", 5) == 0) normalized += 4;  // keep leading /
    else if (strncmp(normalized, "/usb", 4) == 0) normalized += 4;
    if (normalized[0] != '/') {
      // bare name — prepend parent path
      char tmp[128];
      joinPath(path, normalized, tmp, sizeof(tmp));
      // store via FileItem below
      const char *slash = strrchr(tmp, '/');
      const char *name = slash ? slash + 1 : tmp;
      if (!name || name[0] == '\0' || name[0] == '.') {
        entry.close();
        continue;
      }
      FileItem &item = currentDirItems[currentDirItemsCount];
      strncpy(item.name, name, sizeof(item.name) - 1);
      strncpy(item.fullPath, tmp, sizeof(item.fullPath) - 1);
      item.name[sizeof(item.name) - 1] = '\0';
      item.fullPath[sizeof(item.fullPath) - 1] = '\0';
      item.isDirectory = entry.isDirectory();
      item.size = entry.size();
      currentDirItemsCount++;
      entry.close();
      continue;
    }

    // normalized now starts with / and has no /usb prefix
    const char *slash = strrchr(normalized, '/');
    const char *name = slash ? slash + 1 : normalized;

    if (!name || name[0] == '\0' || name[0] == '.') {
      entry.close();
      continue;
    }

    // Build clean path relative to FS root
    char nextPath[128];
    joinPath(path, name, nextPath, sizeof(nextPath));

    FileItem &item = currentDirItems[currentDirItemsCount];
    strncpy(item.name, name, sizeof(item.name) - 1);
    strncpy(item.fullPath, nextPath, sizeof(item.fullPath) - 1);
    item.name[sizeof(item.name) - 1] = '\0';
    item.fullPath[sizeof(item.fullPath) - 1] = '\0';
    item.isDirectory = entry.isDirectory();
    item.size = entry.size();
    currentDirItemsCount++;
    entry.close();
  }
  root.close();
  renderFrame();
}

// ── Grid layout ────────────────────────────────────────────────────────────────
#define GRID_COLS 4
#define GRID_START_X 10
#define GRID_START_Y 45
#define GRID_SPACING_X 36
#define GRID_SPACING_Y 35
#define GRID_CELL_W 32
#define GRID_CELL_H 32

void getItemRect(int index, int &x, int &y, int &w, int &h) {
  x = GRID_START_X + (index % GRID_COLS) * GRID_SPACING_X;
  y = GRID_START_Y + (index / GRID_COLS) * GRID_SPACING_Y;
  w = GRID_CELL_W;
  h = GRID_CELL_H;
}

void drawWrappedName(const char *name, int ix, int iy, int iw) {
  const char *dn = (strcmp(name, ".. [Back]") == 0) ? "Up" : name;
  int len = strlen(dn);
  sprite.setTextColor(ST7735_BLACK);
  if (len <= 5) {
    sprite.setCursor(ix + (iw - len * 6) / 2, iy + 21);
    sprite.print(dn);
  } else {
    char l1[6] = {}, l2[6] = {};
    strncpy(l1, dn, 5);
    if (len <= 10) {
      strncpy(l2, dn + 5, 5);
    } else {
      strncpy(l2, dn + 5, 3);
      strcat(l2, "..");
    }
    sprite.setCursor(ix + (iw - strlen(l1) * 6) / 2, iy + 19);
    sprite.print(l1);
    sprite.setCursor(ix + (iw - strlen(l2) * 6) / 2, iy + 27);
    sprite.print(l2);
  }
}

// ── Render ─────────────────────────────────────────────────────────────────────
void drawBackground() {
  sprite.fillScreen(0x0410);
  sprite.fillRect(2, 2, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 4, 0xD69A);
  sprite.drawFastHLine(2, 2, SCREEN_WIDTH - 4, ST7735_WHITE);
  sprite.drawFastVLine(2, 2, SCREEN_HEIGHT - 4, ST7735_WHITE);
  sprite.drawFastHLine(2, SCREEN_HEIGHT - 3, SCREEN_WIDTH - 4, 0x4208);
  sprite.drawFastVLine(SCREEN_WIDTH - 3, 2, SCREEN_HEIGHT - 4, 0x4208);
  sprite.fillRect(4, 40, SCREEN_WIDTH - 8, 75, ST7735_WHITE);
  sprite.drawRect(4, 40, SCREEN_WIDTH - 8, 75, 0x8410);
  sprite.drawRect(3, 39, SCREEN_WIDTH - 6, 77, 0xBDD7);
}

void renderFrame() {
  drawBackground();

  // Title bar
  sprite.fillRect(4, 4, SCREEN_WIDTH - 8, 13, 0x0010);
  sprite.setTextSize(1);
  sprite.setTextColor(ST7735_WHITE);
  sprite.setCursor(8, 7);
  sprite.print("My Computer F:\\");
  sprite.fillRect(146, 6, 9, 9, 0xD69A);
  sprite.drawRect(146, 6, 9, 9, ST7735_WHITE);
  sprite.setTextColor(ST7735_BLACK);
  sprite.setCursor(148, 7);
  sprite.print("X");

  // Address bar
  sprite.fillRect(4, 17, SCREEN_WIDTH - 8, 22, 0xD69A);
  sprite.setTextColor(0x4208);
  sprite.setCursor(6, 25);
  sprite.print("Address: ");
  sprite.fillRect(52, 21, 101, 13, ST7735_WHITE);
  sprite.drawRect(52, 21, 101, 13, 0x8410);
  sprite.setTextColor(ST7735_BLACK);
  sprite.setCursor(55, 24);
  char winPath[64];
  strncpy(winPath, currentPath, sizeof(winPath) - 1);
  for (int i = 0; winPath[i]; i++)
    if (winPath[i] == '/') winPath[i] = '\\';
  sprite.printf("F:%s", winPath);

  // In renderFrame() address bar — strip /usb prefix for display:
  char displayPath[64];
  const char *dp = currentPath;
  if (strncmp(dp, "/usb", 4) == 0) dp += 4;
  if (dp[0] == '\0') dp = "/";
  strncpy(displayPath, dp, sizeof(displayPath) - 1);
  for (int i = 0; displayPath[i]; i++)
    if (displayPath[i] == '/') displayPath[i] = '\\';
  sprite.printf("F:%s", displayPath);

  // Main area
  if (!isMscConnected) {
    sprite.setTextColor(ST7735_RED);
    sprite.setCursor(14, 55);
    sprite.print("USB MSC DETACHED");
    sprite.setTextColor(ST7735_BLACK);
    sprite.setCursor(14, 70);
    sprite.print("Insert USB drive...");
  } else if (strlen(activeFileViewContent) > 0) {
    // File viewer
    sprite.fillRect(6, 42, SCREEN_WIDTH - 12, 71, ST7735_WHITE);
    sprite.drawRect(6, 42, SCREEN_WIDTH - 12, 71, 0x0010);
    sprite.fillRect(6, 42, SCREEN_WIDTH - 12, 10, 0x0010);
    sprite.setTextColor(ST7735_WHITE);
    sprite.setCursor(10, 44);
    sprite.print("FILE CONTENT");
    sprite.setTextColor(ST7735_BLACK);
    int startY = 56;
    int idx = 0;
    int total = strlen(activeFileViewContent);
    while (idx < total && startY < 100) {
      char line[22] = {};
      int c = 0;
      while (c < 21 && idx < total && activeFileViewContent[idx] != '\n')
        line[c++] = activeFileViewContent[idx++];
      if (idx < total && activeFileViewContent[idx] == '\n') idx++;
      sprite.setCursor(10, startY);
      sprite.print(line);
      startY += 9;
    }
    sprite.setTextColor(0x001F);
    sprite.setCursor(10, 103);
    sprite.print("Click anywhere to close");
    if (millis() - viewTimer > 8000)
      memset(activeFileViewContent, 0, sizeof(activeFileViewContent));
  } else {
    // Icon grid
    for (int i = 0; i < currentDirItemsCount; i++) {
      int ix, iy, iw, ih;
      getItemRect(i, ix, iy, iw, ih);
      bool hover = isMouseConnected && (posX >= ix && posX < ix + iw && posY >= iy && posY < iy + ih);
      if (hover) {
        sprite.fillRect(ix, iy, iw, ih, 0xCE79);
        sprite.drawRect(ix, iy, iw, ih, 0x0010);
      }
      if (strcmp(currentDirItems[i].name, ".. [Back]") == 0) {
        sprite.fillRect(ix + 9, iy + 4, 15, 9, 0xFFE0);
        sprite.fillRect(ix + 9, iy + 2, 5, 2, 0xFDC0);
        sprite.drawRect(ix + 9, iy + 3, 15, 10, 0x9240);
        sprite.fillRect(ix + 15, iy + 7, 2, 4, ST7735_GREEN);
        sprite.drawPixel(ix + 15, iy + 4, ST7735_GREEN);
        sprite.drawFastHLine(ix + 14, iy + 5, 4, ST7735_GREEN);
        sprite.drawFastHLine(ix + 13, iy + 6, 6, ST7735_GREEN);
      } else if (currentDirItems[i].isDirectory) {
        sprite.fillRect(ix + 9, iy + 4, 15, 9, 0xFFD0);
        sprite.fillRect(ix + 9, iy + 2, 5, 2, 0xFCA0);
        sprite.drawRect(ix + 9, iy + 3, 15, 10, 0x9200);
      } else {
        sprite.fillRect(ix + 11, iy + 2, 11, 13, ST7735_WHITE);
        sprite.drawRect(ix + 11, iy + 2, 11, 13, ST7735_BLACK);
        sprite.drawFastHLine(ix + 13, iy + 5, 7, 0x8410);
        sprite.drawFastHLine(ix + 13, iy + 8, 7, 0x8410);
        sprite.drawFastHLine(ix + 13, iy + 11, 5, 0x8410);
      }
      drawWrappedName(currentDirItems[i].name, ix, iy, iw);
    }
  }

  // Status bar
  sprite.fillRect(4, 116, SCREEN_WIDTH - 8, 8, 0xD69A);
  sprite.drawFastHLine(4, 115, SCREEN_WIDTH - 8, 0x8410);
  sprite.setTextColor(ST7735_BLACK);
  sprite.setCursor(8, 117);
  if (isMscConnected)
    sprite.printf("%d items", currentDirItemsCount);
  else
    sprite.print("No drive");

  // Mouse cursor
  if (isMouseConnected) {
    sprite.fillCircle(posX, posY, 1, ST7735_WHITE);
    sprite.drawLine(posX, posY, posX + 6, posY + 4, ST7735_BLACK);
    sprite.drawLine(posX, posY, posX + 2, posY + 7, ST7735_BLACK);
    sprite.drawLine(posX + 6, posY + 4, posX + 2, posY + 7, ST7735_BLACK);
    sprite.fillTriangle(posX + 1, posY + 1, posX + 4, posY + 3, posX + 2, posY + 5, ST7735_WHITE);
  }

  tft.drawRGBBitmap(0, 0, sprite.getBuffer(), SCREEN_WIDTH, SCREEN_HEIGHT);
}

// ── Click handler ──────────────────────────────────────────────────────────────
void handleMouseLeftClick() {
  if (!isMscConnected) return;

  if (strlen(activeFileViewContent) > 0) {
    memset(activeFileViewContent, 0, sizeof(activeFileViewContent));
    renderFrame();
    return;
  }

  for (int i = 0; i < currentDirItemsCount; i++) {
    int ix, iy, iw, ih;
    getItemRect(i, ix, iy, iw, ih);
    if (posX >= ix && posX < ix + iw && posY >= iy && posY < iy + ih) {
      Serial.printf("[Click] '%s' → '%s'\n", currentDirItems[i].name, currentDirItems[i].fullPath);
      if (currentDirItems[i].isDirectory) {
        loadDirectory(currentDirItems[i].fullPath);
      } else {
        File f = usbMassStorage.open(currentDirItems[i].fullPath, FILE_READ);
        if (f) {
          size_t n = f.readBytes(activeFileViewContent, sizeof(activeFileViewContent) - 1);
          activeFileViewContent[n] = '\0';
          viewTimer = millis();
          f.close();
          Serial.printf("[File] Read %u bytes\n", (unsigned)n);
        } else {
          Serial.printf("[File] open failed: '%s'\n", currentDirItems[i].fullPath);
        }
      }
      renderFrame();
      break;
    }
  }
}

// ── Setup / Loop ───────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);

  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  renderFrame();

  usb.onDeviceConnected([](const EspUsbHostDeviceInfo &device) {
    Serial.printf("[USB] Connected VID:%04X PID:%04X Class:%02X\n",
                  device.vid, device.pid, device.deviceClass);
    isMouseConnected = true;  // assume any connected device could be mouse
    renderFrame();
  });

  usb.onDeviceDisconnected([](const EspUsbHostDeviceInfo &device) {
    Serial.printf("[USB] Disconnected Class:%02X\n", device.deviceClass);
    isMouseConnected = false;
    renderFrame();
  });

  usb.onMouse([](const EspUsbHostMouseEvent &event) {
    if (event.moved) {
      posX = constrain(posX + event.x, 2, SCREEN_WIDTH - 3);
      posY = constrain(posY + event.y, 16, SCREEN_HEIGHT - 3);
    }
    if (event.buttonsChanged) {
      uint8_t pressed = event.buttons & ~event.previousButtons;
      if (pressed & 0x01) handleMouseLeftClick();
    }
    renderFrame();
  });

  if (!usb.begin()) {
    Serial.printf("[USB] begin() failed: %s\n", usb.lastErrorName());
  }
}

void loop() {
  if (!usbMassStorage.mounted()) {
    const uint32_t now = millis();
    if (now - lastMountAttemptMs >= 1000) {
      lastMountAttemptMs = now;
      if (usbMassStorage.begin(usb, "/usb")) {
        isMscConnected = true;
        Serial.println("[MSC] Mounted — loading root");
        loadDirectory("/");
        renderFrame();
      } else {
        if (isMscConnected) {
          // Was mounted, now gone
          isMscConnected = false;
          strcpy(currentPath, "/");
          currentDirItemsCount = 0;
          memset(activeFileViewContent, 0, sizeof(activeFileViewContent));
          renderFrame();
        }
      }
    }
  }
  delay(16);
}