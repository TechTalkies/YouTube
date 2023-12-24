#include <ESP8266WiFi.h>
#include <espnow.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address and change
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String myData = "Ready";
bool displayInit = false;
bool updateDisplay = false;

// callback function that will be executed when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  Serial.print("Bytes received: ");
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(myData);
  Serial.println();

  if (displayInit)
    updateDisplay = true;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(74880);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocated"));
    displayInit = true;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  display.setTextColor(SSD1306_WHITE);
  display.cp437(true);  // Use full 256 char 'Code Page 437' font

  displayUpdater();
}

void displayUpdater() {
  display.clearDisplay();
  display.setCursor(10, 0);  // Start at top-left corner
  display.setTextSize(2);   // Normal 1:1 pixel scale
  display.println(myData);
  display.println("");
  display.display();
  updateDisplay = false;
}

void loop() {
  if (updateDisplay)
    displayUpdater();
}
