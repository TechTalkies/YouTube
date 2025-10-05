/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1


Below versions used for this project. 
Try the exact versions if you have any compile errors.

LVGL - 8.4.0 (https://github.com/lvgl/lvgl)
ESP32 boards - 3.3.1 (https://github.com/espressif/arduino-esp32)
-------------------------------------------------*/

#include <lvgl.h>
#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

#include "AXS15231B.h"
#include <Arduino.h>
#include <Wire.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <Ticker.h>

#include <ArduinoJson.h>

/**********************User Data***********************************/

const char *ssid = "WiFi Name";
const char *password = "Password";

//Weather API key
const char *apiKey1 = "Weather API key";

//Weather location
float latitude = 64.612231;
float longitude = -18.546648;

//YouTube API key
const char *apiKey2 = "Youtube API key";
const char *channelID = "Channel ID";

// 300s = 5 minutes to not flooding
#define HTTPS_REQUEST_INTERVAL 300

/**********************User Data***********************************/


const char *baseUrl2 = "https://www.googleapis.com/youtube/v3/channels?part=statistics,snippet&id=";

char weatherURL[512];
char youtubeURL[512];
snprintf(urlBuf, sizeof(urlBuf),
         "https://api.openweathermap.org/data/2.5/weather?lat=%.6f&lon=%.6f&units=metric&APPID=%s",
         latitude, longitude, apiKey1);
snprintf(youtubeURL, sizeof(urlBuf), "%s%s&key=%s", baseUrl2, channelID, apiKey2);

uint8_t ALS_ADDRESS = 0x3B;
#define TOUCH_IICSCL 10
#define TOUCH_IICSDA 15
#define TOUCH_INT 11
#define TOUCH_RES 16

#define AXS_TOUCH_ONE_POINT_LEN 6
#define AXS_TOUCH_BUF_HEAD_LEN 2

#define AXS_TOUCH_GESTURE_POS 0
#define AXS_TOUCH_POINT_NUM 1
#define AXS_TOUCH_EVENT_POS 2
#define AXS_TOUCH_X_H_POS 2
#define AXS_TOUCH_X_L_POS 3
#define AXS_TOUCH_ID_POS 4
#define AXS_TOUCH_Y_H_POS 4
#define AXS_TOUCH_Y_L_POS 5
#define AXS_TOUCH_WEIGHT_POS 6
#define AXS_TOUCH_AREA_POS 7

#define AXS_GET_POINT_NUM(buf) buf[AXS_TOUCH_POINT_NUM]
#define AXS_GET_GESTURE_TYPE(buf) buf[AXS_TOUCH_GESTURE_POS]
#define AXS_GET_POINT_X(buf, point_index) (((uint16_t)(buf[AXS_TOUCH_ONE_POINT_LEN * point_index + AXS_TOUCH_X_H_POS] & 0x0F) << 8) + (uint16_t)buf[AXS_TOUCH_ONE_POINT_LEN * point_index + AXS_TOUCH_X_L_POS])
#define AXS_GET_POINT_Y(buf, point_index) (((uint16_t)(buf[AXS_TOUCH_ONE_POINT_LEN * point_index + AXS_TOUCH_Y_H_POS] & 0x0F) << 8) + (uint16_t)buf[AXS_TOUCH_ONE_POINT_LEN * point_index + AXS_TOUCH_Y_L_POS])
#define AXS_GET_POINT_EVENT(buf, point_index) (buf[AXS_TOUCH_ONE_POINT_LEN * point_index + AXS_TOUCH_EVENT_POS] >> 6)

static const uint16_t screenWidth = 180;
static const uint16_t screenHeight = 640;
#define COLOR_DEPTH 2  // bytes per pixel (16-bit)

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;
static lv_color_t *buf1;

Ticker ticker;
JsonObject jsonYT;
JsonDocument docYT, docWeather;

String subscribers = "Updating..";
String weather = "Updating..";
String temperature = "00";
String iconCode;

String title = "Updating..";
String subs = "Updating..";
String views = "Updating..";
String videos = "Updating..";

String viewCount, videoCount;

int32_t timer_sec;
int32_t timer_mins;

unsigned long lastMillis = 0;  // Timestamp of data refresh
bool timerRunning = false;
unsigned long timerStartTime;
bool timer_countdown;          //Timer mode
unsigned long lastUpdate = 0;  // When was the refresh time updated

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

// counter that lcd_PushColors already increments when it queues a DMA chunk.
// make it visible to the flush_cb. mark volatile because it's changed by different contexts.
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area,
                   lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#ifdef LCD_SPI_DMA
  char i = 0;
  while (get_lcd_spi_dma_write()) {
    i = i >> 1;
    lcd_PushColors(0, 0, 0, 0, NULL);
  }
#endif
  lcd_PushColors(area->x1, area->y1, w, h, (uint16_t *)&color_p->full);

#ifdef LCD_SPI_DMA

#else
  lv_disp_flush_ready(disp);
#endif
}

uint8_t read_touchpad_cmd[11] = { 0xb5, 0xab, 0xa5, 0x5a, 0x0, 0x0, 0x0, 0x8 };

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint8_t buff[20] = { 0 };

  Wire.beginTransmission(0x3B);
  Wire.write(read_touchpad_cmd, 8);
  Wire.endTransmission();
  Wire.requestFrom(0x3B, 8);
  while (!Wire.available())
    ;
  Wire.readBytes(buff, 8);

  uint16_t pointX;
  uint16_t pointY;
  uint16_t type = 0;

  type = AXS_GET_GESTURE_TYPE(buff);
  pointX = AXS_GET_POINT_X(buff, 0);
  pointY = AXS_GET_POINT_Y(buff, 0);

  if (!type && (pointX || pointY)) {
    pointX = (640 - pointX);
    if (pointX > 640) pointX = 640;
    if (pointY > 180) pointY = 180;
    data->state = LV_INDEV_STATE_PR;
    data->point.x = pointY;
    data->point.y = pointX;

    Serial.print("x:");
    Serial.print(pointX);
    Serial.print(", y:");
    Serial.println(pointY);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}


void setup() {
  Serial.begin(115200); /* prepare for possible serial debug */

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  pinMode(TOUCH_RES, OUTPUT);
  digitalWrite(TOUCH_RES, HIGH);
  delay(2);
  digitalWrite(TOUCH_RES, LOW);
  delay(10);
  digitalWrite(TOUCH_RES, HIGH);
  delay(2);

  Wire.begin(TOUCH_IICSDA, TOUCH_IICSCL);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  axs15231_init();

  lv_init();

  size_t buffer_size =
    sizeof(lv_color_t) * screenWidth * screenHeight;

  buf = (lv_color_t *)ps_malloc(buffer_size);
  if (buf == NULL) {
    while (1) {
      Serial.println("buf NULL");
      delay(500);
    }
  }

  buf1 = (lv_color_t *)ps_malloc(buffer_size);
  if (buf1 == NULL) {
    while (1) {
      Serial.println("buf NULL");
      delay(500);
    }
  }

  if (!buf || !buf1) {
    Serial.println("PSRAM allocation failed!");
    while (1) delay(1000);
  }

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  lv_disp_draw_buf_init(&draw_buf, buf, buf1, screenWidth * screenHeight);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.full_refresh = 1;
  //disp_drv.direct_mode = 1;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();

  Serial.println("Setup done");
  ticker.attach(HTTPS_REQUEST_INTERVAL, sendRequest);
  sendRequest();
}

extern uint32_t transfer_num;
extern size_t lcd_PushColors_len;

void loop() {
  //lv_timer_handler(); /* let the GUI do its work */

  if (transfer_num <= 0 && lcd_PushColors_len <= 0)
    lv_timer_handler();

  if (transfer_num <= 1 && lcd_PushColors_len > 0) {
    lcd_PushColors(0, 0, 0, 0, NULL);
  }

  ui_tick();

  if (timerRunning) {
    unsigned long elapsed = millis() - timerStartTime;
    unsigned int seconds = (elapsed / 1000) % 60;
    unsigned int minutes = (elapsed / 1000) / 60;
    Serial.printf("%02u:%02u\n", minutes, seconds);

    char buf[4];

    sprintf(buf, "%02u", minutes);
    lv_label_set_text(objects.timer_label_min, buf);
    sprintf(buf, "%02u", seconds);
    lv_label_set_text(objects.timer_label_sec, buf);
  }

  unsigned long now = millis();

  if (now - lastUpdate >= 60000) {
    lastUpdate = now;  // reset the timer
    updateStats();     // call your function
  }

  delay(15);
}

void sendRequest() {
  Serial.print("Send request fired ");
  Serial.println(millis());

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http1, http2;
    http1.begin(weatherURL);  // Weather API
    http2.begin(youtubeURL);  // Google API

    int httpResponseCode = http1.GET();
    int httpResponseCode2 = http2.GET();

    String payload1, payload2;

    if (httpResponseCode > 0) {
      payload1 = http1.getString();
      Serial.println(payload1);
      decodeWeatherData(payload1);
    } else {
      Serial.print("Error on HTTP GET request: ");
      Serial.println(httpResponseCode);
    }
    http1.end();

    if (httpResponseCode2 > 0) {
      payload2 = http2.getString();
      decodeYoutubeData(payload2);
      Serial.println(payload2);
    } else {
      Serial.print("Error on HTTP GET request: ");
      Serial.println(httpResponseCode2);
    }
    http2.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void decodeWeatherData(String payload) {
  // Allocate the JSON document
  DeserializationError error = deserializeJson(docWeather, payload);

  if (error) {
    Serial.println(F("Parsing failed!"));
    Serial.println(error.c_str());
    return;
  }

  // keep entire JSON in "doc" for later use
  JsonObject root = docWeather.as<JsonObject>();

  // extract once
  const char *mainWeather = root["weather"][0]["main"];
  const char *description = root["weather"][0]["description"];
  double main_temp = root["main"]["temp"];
  int humidity = root["main"]["humidity"];
  const char *icon = root["weather"][0]["icon"];

  double wind_speed = root["wind"]["speed"];
  double wind_gust = root["wind"]["gust"];
  int wind_deg = root["wind"]["deg"];

  char weather_buf[32];
  sprintf(weather_buf, "%s, %d °C", mainWeather, (int)main_temp);

  iconCode = (String)icon;

  Serial.print("weather,");
  Serial.print(weather);
  Serial.print(",");
  Serial.println(temperature);

  char buf[30];
  sprintf(buf, "%s", weather);

  lv_label_set_text(objects.label_main_weather, weather_buf);
  lv_img_set_src(objects.img_main_weather, getIcon(icon));
  lv_img_set_src(objects.img_weather_icon, getIcon(icon));

  lv_label_set_text(objects.label_weather_desc, description);
  lv_label_set_text(objects.label_weather_title, weather_buf);

  sprintf(buf, "%.0f °C", main_temp);
  lv_label_set_text(objects.label_weather_temp, buf);

  sprintf(buf, "%d %%", humidity);
  lv_label_set_text(objects.label_weather_hum, buf);

  sprintf(buf, "%.0f m/s", wind_speed);
  lv_label_set_text(objects.label_weather_wind, buf);

  updateStats();
}

void decodeYoutubeData(String payload) {
  // Allocate the JSON document
  DeserializationError error = deserializeJson(docYT, payload);

  if (error) {
    Serial.println(F("Parsing failed!"));
    Serial.println(error.c_str());
    return;
  }

  // Navigate into items[0]
  jsonYT = docYT["items"][0];

  // Snippet -> title
  const char *title = jsonYT["snippet"]["title"];

  // Statistics
  const char *subs = jsonYT["statistics"]["subscriberCount"];
  const char *views = jsonYT["statistics"]["viewCount"];
  const char *videos = jsonYT["statistics"]["videoCount"];

  subscribers = (String)subs;
  viewCount = (String)views;
  videoCount = (String)videos;

  lv_label_set_text(objects.label_main_yt, subs);
  lv_label_set_text(objects.label_yt_title, title);
  lv_label_set_text(objects.label_yt_subs, subs);
  lv_label_set_text(objects.label_yt_views, views);
  lv_label_set_text(objects.label_yt_videos, videos);

  updateStats();
}

// return pointer to image descriptor
const lv_img_dsc_t *getIcon(const char *iconCode) {
  if (strcmp(iconCode, "01d") == 0 || strcmp(iconCode, "01n") == 0) return &img_01d;
  if (strcmp(iconCode, "02d") == 0 || strcmp(iconCode, "02n") == 0) return &img_02d;
  if (strcmp(iconCode, "03d") == 0 || strcmp(iconCode, "03n") == 0) return &img_03d;
  if (strcmp(iconCode, "04d") == 0 || strcmp(iconCode, "04n") == 0) return &img_03d;
  if (strcmp(iconCode, "09d") == 0 || strcmp(iconCode, "09n") == 0) return &img_09d;
  if (strcmp(iconCode, "10d") == 0 || strcmp(iconCode, "10n") == 0) return &img_09d;
  if (strcmp(iconCode, "11d") == 0 || strcmp(iconCode, "11n") == 0) return &img_11d;
  if (strcmp(iconCode, "13d") == 0 || strcmp(iconCode, "13n") == 0) return &img_13d;
  if (strcmp(iconCode, "50d") == 0 || strcmp(iconCode, "50n") == 0) return &img_50d;
  return &img_01d;
}

void updateStats() {
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - lastMillis;

  unsigned long elapsedMinutes = elapsedMillis / 60000;  // integer minutes

  lastMillis = millis();  // initialize

  char buffer[40];
  sprintf(buffer, "%lu mins ago", elapsedMinutes);  // use %lu for unsigned long

  lv_label_set_text(objects.main_label_refreshed, buffer);

  if (WiFi.status() == WL_CONNECTED)
    lv_label_set_text(objects.main_label_wifi, WIFI_SSID);
  else
    lv_label_set_text(objects.main_label_wifi, "No Wi-Fi");
}

void action_timer_change_mins(lv_event_t *e) {
  char buf[4];
  sprintf(buf, "%02u", get_var_timer_mins());
  lv_label_set_text(objects.timer_label_min, buf);
}

void action_timer_change_sec(lv_event_t *e) {
  char buf[4];
  sprintf(buf, "%02u", get_var_timer_sec());
  lv_label_set_text(objects.timer_label_sec, buf);
}

void action_main_refresh(lv_event_t *e) {
  sendRequest();
}

void action_timer_start(lv_event_t *e) {
  if (timerRunning) {
    lv_label_set_text(objects.timer_label_start, "Start");
  } else {
    lv_label_set_text(objects.timer_label_start, "Stop");
    timerStartTime = millis();
  }
  timerRunning = !timerRunning;
}


int32_t get_var_timer_sec() {
  return timer_sec;
}
void set_var_timer_sec(int32_t value) {
  timer_sec = value;
}
int32_t get_var_timer_mins() {
  return timer_mins;
}
void set_var_timer_mins(int32_t value) {
  timer_mins = value;
}

bool get_var_timer_countdown() {
  return timer_countdown;
}
void set_var_timer_countdown(bool value) {
  timer_countdown = value;
}
