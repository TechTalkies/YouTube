/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include "esp_camera.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "webpage.h"

// ===========================
// CAMERA PINS (XIAO ESP32-S3)
// ===========================
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 10
#define SIOD_GPIO_NUM 40
#define SIOC_GPIO_NUM 39

#define Y9_GPIO_NUM 48
#define Y8_GPIO_NUM 11
#define Y7_GPIO_NUM 12
#define Y6_GPIO_NUM 14
#define Y5_GPIO_NUM 16
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 17
#define Y2_GPIO_NUM 15

#define VSYNC_GPIO_NUM 38
#define HREF_GPIO_NUM 47
#define PCLK_GPIO_NUM 13

// left motor
#define l_in1 D6
#define l_in2 D5

// right motor
#define r_in1 D2
#define r_in2 D1

#define LED_LEFT D4   // D4
#define LED_RIGHT D8  // D8

// ===========================
// WIFI
// ===========================
const char *ssid = "SSID";
const char *password = "Password";

// ===========================
// SERVER
// ===========================
AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

void motor_init() {
  ledcAttach(l_in1, 20000, 8);  // channel 0, 20khz, 8-bit
  ledcAttach(l_in2, 20000, 8);
  ledcAttach(r_in1, 20000, 8);
  ledcAttach(r_in2, 20000, 8);
  ledcAttach(LED_LEFT, 20000, 8);
  ledcAttach(LED_RIGHT, 20000, 8);
}

// ==============
// ROVER CONTROL
// ==============
void moveCar(int left, int right) {
  // clamp incoming values safely
  left = constrain(left, -255, 255);
  right = constrain(right, -255, 255);

  left = applyMinPWM(left);
  right = applyMinPWM(right);

  // LEFT MOTOR
  if (left > 0) {
    ledcWrite(l_in1, left);  // IN1 PWM
    ledcWrite(l_in2, 0);     // IN2 LOW
  } else if (left < 0) {
    ledcWrite(l_in1, 0);
    ledcWrite(l_in2, -left);  // reverse
  } else {
    ledcWrite(l_in1, 0);
    ledcWrite(l_in2, 0);
  }

  // RIGHT MOTOR
  if (right > 0) {
    ledcWrite(r_in1, right);
    ledcWrite(r_in2, 0);
  } else if (right < 0) {
    ledcWrite(r_in1, 0);
    ledcWrite(r_in2, -right);
  } else {
    ledcWrite(r_in1, 0);
    ledcWrite(r_in2, 0);
  }
}

int applyMinPWM(int v) {
  if (v == 0) return 0;
  int s = (v > 0) ? 1 : -1;
  v = abs(v);
  if (v < 40) v = 40;
  return s * v;
}



// ===========================
// WEBSOCKET CONTROL
// ===========================
void onCarInputWebSocketEvent(AsyncWebSocket *server,
                              AsyncWebSocketClient *client,
                              AwsEventType type,
                              void *arg,
                              uint8_t *data,
                              size_t len) {

  if (type != WS_EVT_DATA) return;

  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (!info->final || info->opcode != WS_TEXT) return;

  data[len] = 0;
  String msg = (char *)data;

  if (msg.startsWith("MoveCar")) {
    int firstComma = msg.indexOf(',');
    int secondComma = msg.indexOf(',', firstComma + 1);

    if (firstComma < 0 || secondComma < 0) return;

    int left = msg.substring(firstComma + 1, secondComma).toInt();
    int right = msg.substring(secondComma + 1).toInt();

    moveCar(left, right);
  }
}

// ===========================
// MJPEG STREAM HANDLER
// ===========================
void handleStream(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response =
    request->beginChunkedResponse(
      "multipart/x-mixed-replace; boundary=frame",
      [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
        static camera_fb_t *fb = nullptr;
        static size_t fb_pos = 0;

        if (!fb) {
          fb = esp_camera_fb_get();
          if (!fb) return 0;

          String header = "--frame\r\nContent-Type: image/jpeg\r\n\r\n";
          memcpy(buffer, header.c_str(), header.length());
          return header.length();
        }

        size_t remaining = fb->len - fb_pos;
        size_t toCopy = min(remaining, maxLen);
        memcpy(buffer, fb->buf + fb_pos, toCopy);
        fb_pos += toCopy;

        if (fb_pos >= fb->len) {
          esp_camera_fb_return(fb);
          fb = nullptr;
          fb_pos = 0;
          return toCopy;
        }

        return toCopy;
      });

  response->addHeader("Cache-Control", "no-cache");
  response->addHeader("Connection", "keep-alive");
  request->send(response);
}

void updateLights() {
  static uint32_t lastUpdate = 0;
  static int brightness = 0;
  static int direction = 5;

  if (millis() - lastUpdate >= 10) {
    lastUpdate = millis();
    brightness += direction;

    if (brightness >= 255 || brightness <= 0) {
      direction = -direction;
    }

    ledcWrite(LED_LEFT, brightness);        // LEFT
    ledcWrite(LED_RIGHT, 255 - brightness);  // RIGHT
  }
}

// ===========================
// SETUP
// ===========================
void setup() {
  Serial.begin(115200);
  Serial.println();

  // ---------- CAMERA CONFIG ----------
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // 🔥 LOW LATENCY SETTINGS
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 8;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;

  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);

  // ---------- WIFI ----------
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  // ---------- SERVER ----------
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r) {
    r->send_P(200, "text/html", htmlHomePage);
  });

  server.on("/stream", HTTP_GET, handleStream);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.println("Server started");

  motor_init();
}

void loop() {
  wsCarInput.cleanupClients();
  updateLights();
}