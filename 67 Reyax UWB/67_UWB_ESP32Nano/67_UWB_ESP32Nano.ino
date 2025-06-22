/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <math.h>  // for sin, cos

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Pick alternate I2C pins (since 21/22 are not exposed)
#define OLED_SDA A4
#define OLED_SCL A5

// UWB Module on Serial2
#define UWB_RX D5
#define UWB_TX D4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
HardwareSerial UWB(2);  // UART2

float distance = -1;

String sendCmd = "AT+ANCHOR_SEND=TAG1,2,HI";
unsigned long lastSend = 0;
const unsigned long sendInterval = 700;

void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1)
      ;
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  UWB.begin(115200, SERIAL_8N1, UWB_RX, UWB_TX);

  delay(300);
  Serial.println("Starting...");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSend > sendInterval) {
    UWB.println(sendCmd);
    Serial.println("Sent: " + sendCmd);
    lastSend = now;
  }

  if (UWB.available()) {
    String line = UWB.readStringUntil('\n');
    Serial.print("UWB: ");
    Serial.println(line);
    parseDistance(line);
  }

  display.clearDisplay();

  display.setTextSize(2);
  String distText;

  if (distance < 0) {
    distText = "N/A";
  } else {
    distText = String(distance, 2) + " m";
  }

  int16_t x, y;
  uint16_t w, h;
  display.getTextBounds(distText, 0, 0, &x, &y, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2);
  display.print(distText);

  display.display();
}

// Parse "+ANCHOR_RCV=TAG1,0,,26 cm"
void parseDistance(String line) {
  line.trim();
  if (line.startsWith("+ANCHOR_RCV=")) {
    int cmIndex = line.indexOf("cm");
    if (cmIndex > 0) {
      int lastComma = line.lastIndexOf(',', cmIndex);
      if (lastComma > 0) {
        String distStr = line.substring(lastComma + 1, cmIndex);
        distStr.trim();
        distance = distStr.toFloat() / 100.0;
      }
    }
  }
}
