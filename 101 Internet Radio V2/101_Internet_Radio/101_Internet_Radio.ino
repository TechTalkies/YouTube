/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "secrets.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#define I2S_DOUT 7
#define I2S_BCLK 8
#define I2S_LRCK 9

#define TFT_SCK 1
#define TFT_MOSI 2
#define TFT_DC 3
#define TFT_RST 4
#define TFT_CS 5

#define ENC_CLK 43
#define ENC_DT 44
#define ENC_SW 6

#define RB_HOST "http://de1.api.radio-browser.info"
#define RB_RESULT_LIMIT 40
#define VOL_MAX 21
#define INITIAL_VOLUME 18

// ── Editable country list — add/remove as needed ──────────
// Format: {"CODE", "Display Name"}
// Codes must match Radio Browser countrycode field (ISO 3166-1 alpha-2)
struct CountryEntry {
  const char* code;
  const char* name;
};
static const CountryEntry COUNTRIES[] = {
  { "all", "All" },
  { "US", "USA" },
  { "IN", "India" },
  { "GB", "UK" },
  { "DE", "Germany" },
  { "FR", "France" },
  { "JP", "Japan" },
  { "CA", "Canada" },
  { "AU", "Australia" },
  { "IT", "Italy" },
  { "ES", "Spain" },
  { "BR", "Brazil" },
  { "MX", "Mexico" },
  { "NL", "Netherlands" },
  { "SE", "Sweden" },
  { "NO", "Norway" },
  { "ZA", "South Africa" },
  { "SG", "Singapore" },
  { "AE", "UAE" },
};
static const int COUNTRY_COUNT = sizeof(COUNTRIES) / sizeof(COUNTRIES[0]);

// ── Editable genre list ────────────────────────────────────
struct GenreEntry {
  const char* tag;
  const char* label;
};
static const GenreEntry GENRES[] = {
  { "all", "All" },
  { "music", "Music" },
  { "news", "News" },
  { "jazz", "Jazz" },
  { "classical", "Classical" },
  { "rock", "Rock" },
};
static const int GENRE_COUNT = sizeof(GENRES) / sizeof(GENRES[0]);

Audio audio;
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(160, 128);
QueueHandle_t encQueue;

enum EncEvent { EV_CW,
                EV_CCW,
                EV_PRESS,
                EV_LONG };
enum UiMode { MODE_NORMAL,
              MODE_BROWSE,
              MODE_EDIT };
enum FocusItem { F_NOWPLAYING,
                 F_COUNTRY,
                 F_TYPE };

struct Station {
  String name;
  String url;
  int bitrate;
  String country;
};

Station stations[RB_RESULT_LIMIT];
int stationCount = 0;
int currentStation = 0;
int focusIndex = 0;
UiMode uiMode = MODE_NORMAL;
bool uiDirty = true;
String streamTitle = "Loading...";
int previewStation = 0;
String previewTag = "all";
String previewCountry = "";
String searchTag = "all";
String selectedGenre = "all";
String selectedCountry = "all";
int bitrateCap = 96;  //TODO 96

bool muted = false;
int lastVol = INITIAL_VOLUME;
uint32_t browseLastAction = 0;
volatile uint32_t holdStartMs = 0;
volatile bool buttonHolding = false;

bool fetchStations(String tag) {
  HTTPClient http;
  String url = String(RB_HOST) + "/json/stations/search?limit=20&hidebroken=true&order=clickcount&reverse=true&codec=AAC&bitrateMax=" + String(bitrateCap);
  if (tag != "all") url += "&tag=" + tag;
  if (selectedCountry != "all") url += "&countrycode=" + selectedCountry;
  http.begin(url);
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }

  DynamicJsonDocument doc(10000);
  if (deserializeJson(doc, http.getStream())) {
    http.end();
    return false;
  }
  http.end();

  stationCount = 0;
  for (JsonObject s : doc.as<JsonArray>()) {
    String u = s["url_resolved"] | "";
    if (!u.startsWith("http://")) continue;
    stations[stationCount].name = s["name"].as<String>();
    stations[stationCount].url = u;
    stations[stationCount].bitrate = s["bitrate"] | 0;
    stations[stationCount].country = s["countrycode"].as<String>();
    stationCount++;
    if (stationCount >= RB_RESULT_LIMIT) break;
  }
  return stationCount > 0;
}

void playStation(int i) {
  if (i < 0 || i >= stationCount) return;
  currentStation = i;
  streamTitle = stations[i].name;
  audio.connecttohost(stations[i].url.c_str());
  uiDirty = true;
}

String countryName(String c) {
  c.toUpperCase();
  if (c == "US") return "USA";
  if (c == "GB") return "UK";
  if (c == "IN") return "India";
  if (c == "DE") return "Germany";
  if (c == "FR") return "France";
  if (c == "JP") return "Japan";
  if (c == "CA") return "Canada";
  if (c == "AU") return "Australia";
  if (c == "IT") return "Italy";
  if (c == "ES") return "Spain";
  if (c == "BR") return "Brazil";
  if (c == "MX") return "Mexico";
  if (c == "NL") return "Netherlands";
  if (c == "SE") return "Sweden";
  if (c == "NO") return "Norway";
  if (c == "ZA") return "South Africa";
  if (c == "SG") return "Singapore";
  if (c == "AE") return "UAE";
  return c;
}

String typeName() {
  for (int i = 0; i < GENRE_COUNT; i++)
    if (selectedGenre == GENRES[i].tag)
      return String(GENRES[i].label);
  return String(selectedGenre);  // fallback: show raw tag
}

#define DISP canvas

void chip(int x, int y, String txt, uint16_t col) {
  int w = txt.length() * 6 + 10;
  bool activeEdit = (uiMode == MODE_EDIT && ((focusIndex == 1 && x < 60) || (focusIndex == 2 && x > 60)));
  if (activeEdit) {
    DISP.fillRoundRect(x, y, w, 14, 3, col);
    DISP.setTextColor(ST77XX_BLACK);
  } else {
    DISP.drawRoundRect(x, y, w, 14, 3, col);
    DISP.setTextColor(col);
  }
  DISP.setCursor(x + 5, y + 4);
  DISP.print(txt);
}

void drawUI() {
  DISP.fillScreen(0x0000);

  uint16_t topBg = tft.color565(50, 0, 60);
  uint16_t cardBg = tft.color565(32, 73, 93);
  uint16_t botBg = ST77XX_BLACK;
  uint16_t border = ST77XX_WHITE;
  uint16_t accent = ST77XX_GREEN;
  uint16_t warn = ST77XX_YELLOW;

  // top bar
  DISP.fillRoundRect(2, 2, 156, 16, 3, topBg);
  DISP.drawRoundRect(2, 2, 156, 16, 3, border);

  // center card
  DISP.fillRoundRect(2, 22, 156, 80, 5, cardBg);
  DISP.drawRoundRect(2, 22, 156, 80, 5, border);

  // bottom bar
  DISP.fillRoundRect(2, 106, 156, 20, 4, botBg);
  DISP.drawRoundRect(2, 106, 156, 20, 4,
                     (uiMode == MODE_NORMAL) ? border : accent);

  // logo
  DISP.setCursor(8, 6);
  DISP.setTextColor(accent);
  DISP.print("Tech Talkies");

  // volume bars
  uint16_t volCol = (uiMode == MODE_NORMAL) ? accent : 0x39E7;
  for (int i = 0; i < 12; i++) {
    uint16_t c = (i < audio.getVolume() * 12 / 21) ? volCol : 0x2104;
    DISP.fillRoundRect(90 + i * 5, 6, 4, 6, 1, c);
  }

  // "Now Playing" label
  uint16_t npCol = (focusIndex == F_NOWPLAYING && uiMode != MODE_NORMAL)
                     ? warn
                     : ST77XX_WHITE;
  DISP.setTextColor(npCol);
  DISP.setCursor((160 - 11 * 6) / 2, 30);
  DISP.print("Now Playing");

  // station name / browse preview
  String np = (uiMode == MODE_EDIT && focusIndex == F_NOWPLAYING)
                ? (String("< ") + stations[previewStation].name.substring(0, 18) + " >")
                : streamTitle.substring(0, 24);
  int npX = (160 - (int)np.length() * 6) / 2;

  if (uiMode == MODE_EDIT && focusIndex == F_NOWPLAYING) {
    DISP.fillRoundRect(npX - 4, 44, np.length() * 6 + 8, 16, 3, warn);
    DISP.setTextColor(ST77XX_BLACK);
    DISP.setCursor(npX, 48);
    DISP.print(np);
    // counter  e.g. "3/15"
    String countTxt = String(previewStation + 1) + "/" + String(stationCount);
    DISP.setTextColor(warn);
    DISP.setCursor((160 - (int)countTxt.length() * 6) / 2, 66);
    DISP.print(countTxt);
  } else {
    DISP.setTextColor(ST77XX_WHITE);
    DISP.setCursor(npX, 48);
    DISP.print(np);
  }

  // ── Country chip ─────────────────────────────────────────
  // Show previewCountry while editing, otherwise selectedCountry
  // (falling back to the playing station's country when "all")
  String cCode = (uiMode == MODE_EDIT && focusIndex == F_COUNTRY)
                   ? String(previewCountry)
                   : (selectedCountry == "all"
                        ? stations[currentStation].country
                        : String(selectedCountry));
  chip(4, 80, countryName(cCode), focusIndex == F_COUNTRY ? warn : accent);

  // ── Genre chip ───────────────────────────────────────────
  // Show previewTag label while editing, otherwise selectedGenre label
  String gLabel;
  if (uiMode == MODE_EDIT && focusIndex == F_TYPE) {
    // find label for previewTag
    gLabel = previewTag;  // fallback
    for (int i = 0; i < GENRE_COUNT; i++)
      if (previewTag == GENRES[i].tag) {
        gLabel = GENRES[i].label;
        break;
      }
  } else {
    gLabel = typeName();
  }
  chip(92, 80, gLabel, focusIndex == F_TYPE ? warn : ST77XX_CYAN);

  // hold-progress circle
  DISP.drawCircle(149, 116, 8, ST77XX_WHITE);
  if (buttonHolding) {
    int prog = min(8, (int)((millis() - holdStartMs) / 87));
    for (int i = 0; i < prog; i++)
      DISP.fillCircle(149, 116, i, accent);
  }

  // bottom hint
  DISP.setCursor(8, 112);
  DISP.setTextColor(ST77XX_WHITE);
  if (uiMode == MODE_NORMAL) DISP.print("Volume");
  else if (uiMode == MODE_BROWSE) DISP.print("Rotate Select");
  else DISP.print("Rotate Change");

  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 160, 128);
  uiDirty = false;
}

void applyChange() {
  fetchStations(selectedGenre);
  playStation(0);
}

void handleEvent(uint8_t ev) {
  browseLastAction = millis();

  if (uiMode == MODE_NORMAL) {

    if (ev == EV_CW) audio.setVolume(min(VOL_MAX, audio.getVolume() + 1));
    else if (ev == EV_CCW) audio.setVolume(max(0, audio.getVolume() - 1));
    else if (ev == EV_PRESS) {
      muted = !muted;
      if (muted) {
        lastVol = audio.getVolume();
        audio.setVolume(0);
      } else audio.setVolume(lastVol);
    } else if (ev == EV_LONG) {
      uiMode = MODE_BROWSE;
      previewStation = currentStation;
      previewTag = selectedGenre;
      previewCountry = selectedCountry;
    }

  } else if (uiMode == MODE_BROWSE) {

    if (ev == EV_CW) focusIndex = (focusIndex + 1) % 3;
    else if (ev == EV_CCW) focusIndex = (focusIndex + 2) % 3;
    else if (ev == EV_PRESS) uiMode = MODE_EDIT;
    else if (ev == EV_LONG) uiMode = MODE_NORMAL;

  } else if (uiMode == MODE_EDIT) {

    // ── F_NOWPLAYING — scroll through loaded stations ─────
    if (focusIndex == F_NOWPLAYING) {
      if (ev == EV_CW && stationCount > 0)
        previewStation = (previewStation + 1) % stationCount;
      else if (ev == EV_CCW && stationCount > 0)
        previewStation = (previewStation - 1 + stationCount) % stationCount;
      else if (ev == EV_PRESS) {
        playStation(previewStation);
        uiMode = MODE_BROWSE;
      } else if (ev == EV_LONG) uiMode = MODE_BROWSE;
    }

    // ── F_COUNTRY — cycles through COUNTRIES[] table ──────
    else if (focusIndex == F_COUNTRY) {
      if (ev == EV_CW || ev == EV_CCW) {
        int ci = 0;
        for (int i = 0; i < COUNTRY_COUNT; i++) {
          if (previewCountry == COUNTRIES[i].code) {
            ci = i;
            break;
          }
        }
        int dir = (ev == EV_CW) ? 1 : -1;
        ci = (ci + dir + COUNTRY_COUNT) % COUNTRY_COUNT;
        previewCountry = COUNTRIES[ci].code;
      } else if (ev == EV_PRESS) {
        selectedCountry = previewCountry;
        applyChange();
        uiMode = MODE_BROWSE;
      } else if (ev == EV_LONG) uiMode = MODE_BROWSE;
    }

    // ── F_TYPE — cycles through GENRES[] table ────────────
    else if (focusIndex == F_TYPE) {
      if (ev == EV_CW || ev == EV_CCW) {
        int gi = 0;
        for (int i = 0; i < GENRE_COUNT; i++) {
          if (previewTag == GENRES[i].tag) {
            gi = i;
            break;
          }
        }
        int dir = (ev == EV_CW) ? 1 : -1;
        gi = (gi + dir + GENRE_COUNT) % GENRE_COUNT;
        previewTag = GENRES[gi].tag;
      } else if (ev == EV_PRESS) {
        selectedGenre = previewTag;
        searchTag = selectedGenre;
        applyChange();
        uiMode = MODE_BROWSE;
      } else if (ev == EV_LONG) uiMode = MODE_BROWSE;
    }
  }

  uiDirty = true;
}

void taskRotary(void* p) {
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  int lastClk = digitalRead(ENC_CLK);
  bool lastBtn = HIGH;
  uint32_t pressAt = 0;

  for (;;) {
    int clk = digitalRead(ENC_CLK);
    if (clk != lastClk && clk == LOW) {
      uint8_t e = digitalRead(ENC_DT) ? EV_CW : EV_CCW;
      xQueueSend(encQueue, &e, 0);
    }
    lastClk = clk;

    bool btn = digitalRead(ENC_SW);
    if (btn == LOW && lastBtn == HIGH) {
      pressAt = millis();
      holdStartMs = pressAt;
      buttonHolding = true;
    }
    if (btn == HIGH && lastBtn == LOW) {
      buttonHolding = false;
      uint8_t e = (millis() - pressAt > 700) ? EV_LONG : EV_PRESS;
      xQueueSend(encQueue, &e, 0);
    }
    lastBtn = btn;
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

void setup() {
  SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
  audio.setVolume(INITIAL_VOLUME);

  encQueue = xQueueCreate(16, 1);
  xTaskCreatePinnedToCore(taskRotary, "rotary", 4096, nullptr, 1, nullptr, 1);

  fetchStations(selectedGenre);
  playStation(0);
}

void loop() {
  audio.loop();
  uint8_t ev;
  while (xQueueReceive(encQueue, &ev, 0) == pdTRUE) handleEvent(ev);

  if ((uiMode == MODE_BROWSE || uiMode == MODE_EDIT) && millis() - browseLastAction > 10000) {
    uiMode = MODE_NORMAL;
    uiDirty = true;
  }

  if (uiDirty) drawUI();
}

void audio_showstreamtitle(const char* info) {
  streamTitle = String(info);
  uiDirty = true;
}