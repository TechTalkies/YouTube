#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <SPI.h>
#include "TFT_eSPI.h"

EPaper epaper;
WebServer server(80);

// ---------------- CONFIG ----------------
#define BTN_NEXT 5
#define BTN_PREV 6

#define MAX_FILE_SIZE (1500 * 1024)  // ~1.5MB

// ---------------- STATE ----------------
File book;
size_t fileSize = 0;
uint32_t pagePos = 0;

unsigned long lastActivity = 0;

// ---------------- HTML ----------------
const char* uploadPage = R"rawliteral(
<!DOCTYPE html>
<html>
<body>
<h2>Upload TXT</h2>
<form method="POST" action="/upload" enctype="multipart/form-data">
<input type="file" name="file">
<input type="submit">
</form>
</body>
</html>
)rawliteral";

// ---------------- WIFI ----------------
void setupAP() {
  Serial.println("[WIFI] Starting AP...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("PocketReader", "12345678");  // password required for reliability

  delay(500);  // give time to start

  Serial.print("[WIFI] AP IP: ");
  Serial.println(WiFi.softAPIP());
}

// ---------------- WEB ----------------
void handleRoot() {
  server.send(200, "text/html", uploadPage);
}

void handleUpload() {
  HTTPUpload& upload = server.upload();

  static File f;

  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("Upload Start");

    if (LittleFS.exists("/book.txt"))
      LittleFS.remove("/book.txt");

    f = LittleFS.open("/book.txt", "w");
  }

  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (f) {
      if (f.size() + upload.currentSize > MAX_FILE_SIZE) {
        Serial.println("File too large");
        f.close();
        LittleFS.remove("/book.txt");
        return;
      }
      f.write(upload.buf, upload.currentSize);
    }
  }

  else if (upload.status == UPLOAD_FILE_END) {
    if (f) f.close();
    Serial.println("Upload Complete");
    server.send(200, "text/plain", "OK");
  }
}

// ---------------- STORAGE ----------------
void loadBook() {
  if (!LittleFS.exists("/book.txt")) return;

  book = LittleFS.open("/book.txt", "r");
  fileSize = book.size();

  // restore position
  File posFile = LittleFS.open("/pos.txt", "r");
  if (posFile) {
    pagePos = posFile.parseInt();
    posFile.close();
  }

  book.seek(pagePos);
}

void savePosition() {
  File posFile = LittleFS.open("/pos.txt", "w");
  posFile.print(pagePos);
  posFile.close();
}

// ---------------- DISPLAY ----------------
void renderPage() {
  epaper.fillScreen(TFT_WHITE);

  int y = 0;
  String line = "";

  while (book.available() && y < 120) {
    char c = book.read();

    if (c == '\n' || line.length() > 35) {
      epaper.drawString(line, 5, y, 2);
      line = "";
      y += 16;
    } else {
      line += c;
    }
  }

  pagePos = book.position();
  savePosition();

  epaper.update();
}

// ---------------- BUTTONS ----------------
bool isPressed(int pin) {
  return digitalRead(pin) == LOW;
}

void handleButtons() {
  static unsigned long pressStart = 0;

  if (isPressed(BTN_NEXT)) {
    if (pressStart == 0) pressStart = millis();

    if (millis() - pressStart > 800) {
      Serial.println("NEXT HOLD");
      // future menu
    }
  } else {
    if (pressStart != 0 && millis() - pressStart < 800) {
      Serial.println("NEXT TAP");
      renderPage();
    }
    pressStart = 0;
  }

  if (isPressed(BTN_PREV)) {
    Serial.println("PREV TAP");

    if (pagePos > 200) {
      pagePos -= 200;
      book.seek(pagePos);
      renderPage();
    }
  }
}

// ---------------- SLEEP ----------------
void maybeSleep() {
  if (millis() - lastActivity > 30000) {
    Serial.println("Light sleep");
    esp_light_sleep_start();
    lastActivity = millis();
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);

  if (!LittleFS.begin()) {
    Serial.println("FS FAIL");
    while (1)
      ;
  }

  setupAP();

  server.on("/", handleRoot);
  server.on(
    "/upload", HTTP_POST, []() {}, handleUpload);
  server.begin();

  epaper.begin();
  epaper.setRotation(1);

  loadBook();
  renderPage();

  lastActivity = millis();
}

// ---------------- LOOP ----------------
void loop() {
  server.handleClient();
  handleButtons();
  maybeSleep();
}