/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <WiFi.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_wifi.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BTN_PIN 0
#define LED_PIN 2
#define MAX_CH 13
#define EEPROM_ADDR 2000
#define PACKET_RATE 5
#define LINE_Y 12
#define LINE_VAL 47

unsigned long prevTime = 0, curTime = 0;
unsigned long pkts = 0, no_deauths = 0, deauths = 0;
int curChannel = 1;
unsigned long maxVal = 0;
double multiplicator = 0.0;
bool canBtnPress = true;
bool ssid_locked = false;

unsigned int val[128];
char ssid[33] = "unknown";

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  const uint8_t *payload = pkt->payload;

  pkts++;

  if (payload[0] == 0xC0 || payload[0] == 0xA0) {
    deauths++;
  }

  if (!ssid_locked && payload[0] == 0x80) {
    uint8_t len = payload[37];
    if (len > 0 && len < 33) {
      memcpy(ssid, &payload[38], len);
      ssid[len] = 0;
      ssid_locked = true;
    }
  }
}

void getMultiplicator() {
  maxVal = val[0];
  for (int i = 1; i < SCREEN_WIDTH; i++) {
    if (val[i] > maxVal) maxVal = val[i];
  }
  if (maxVal < 1) maxVal = 1;
  multiplicator = maxVal > LINE_VAL ? (double)LINE_VAL / maxVal : 1;
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  EEPROM.begin(4096);
  curChannel = EEPROM.read(EEPROM_ADDR);
  if (curChannel < 1 || curChannel > MAX_CH) {
    curChannel = 1;
    EEPROM.write(EEPROM_ADDR, curChannel);
    EEPROM.commit();
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (1)
      ;
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  delay(2000);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
}

void loop() {
  curTime = millis();

  if (digitalRead(BTN_PIN) == LOW) {
    if (canBtnPress) canBtnPress = false;
  } else if (!canBtnPress) {
    canBtnPress = true;
    curChannel++;
    if (curChannel > MAX_CH) curChannel = 1;
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);

    for (int i = 0; i < SCREEN_WIDTH; i++) val[i] = 0;
    pkts = 0;
    multiplicator = 1;
    ssid_locked = false;
    strncpy(ssid, "unknown", sizeof(ssid));
    EEPROM.write(EEPROM_ADDR, curChannel);
    EEPROM.commit();
  }

  if (curTime - prevTime >= 1000) {
    prevTime = curTime;

    for (int i = 0; i < SCREEN_WIDTH - 1; i++) {
      val[i] = val[i + 1];
    }
    val[SCREEN_WIDTH - 1] = pkts;

    getMultiplicator();

    digitalWrite(LED_PIN, deauths > PACKET_RATE ? LOW : HIGH);

    if (pkts == 0) pkts = deauths;
    no_deauths = pkts - deauths;

    display.clearDisplay();

    // Display format: "Ch: X SSID_NAME Pkts"
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Ch:");
    display.print(curChannel);
    display.print(" ");
    display.print(ssid);
    display.print(" ");
    display.print(no_deauths);

    display.drawLine(0, LINE_Y, SCREEN_WIDTH - 1, LINE_Y, SSD1306_WHITE);

    for (int i = 0; i < SCREEN_WIDTH; i++) {
      int bar = val[i] * multiplicator;
      display.drawLine(i, SCREEN_HEIGHT - 1, i, SCREEN_HEIGHT - 1 - bar, SSD1306_WHITE);
    }

    display.display();

    deauths = 0;
    pkts = 0;
  }
}