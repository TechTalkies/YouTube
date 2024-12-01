/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include "webpage.h"

#define PIN 2  //Pin for the Neopixel ring

// Replace with your network credentials
const char* ssid = "WiFi Name";
const char* password = "Password";

const char* input_parameter = "value";

bool LEDstates[16];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer localServer(80);
//AsyncWebServerRequest request;

uint32_t ledColor;
int currentLED = 0;
uint8_t LEDmode = 0;

String processor(const String& var) {
  if (var == "STARTCOLOR") {
    return "#00ff4c";
  }
  return String();
}

void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.setBrightness(100);

  connectWifi();

  localServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  localServer.on("/setting", HTTP_GET, [](AsyncWebServerRequest* request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();

      if (message == "all") {
        LEDmode = 3;
      } else if (message == "single") {
        LEDmode = 2;
      } else if (message == "select") {
        LEDmode = 1;
      }
      updateLEDs();
    } else {
      message = "No message sent";
    }
    Serial.println(message);
    request->send(200, "text/plain", "OK");
  });

  localServer.on("/LED", HTTP_GET, [](AsyncWebServerRequest* request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();
      bool LEDon = !LEDstates[message.toInt()];
      LEDstates[message.toInt()] = LEDon;
      if (LEDon)
        strip.setPixelColor(message.toInt(), ledColor);
      else
        strip.setPixelColor(message.toInt(), 0);
      strip.show();
    } else {
      message = "No message sent";
    }
    Serial.println(message);
    request->send(200, "text/plain", "OK");
  });

  localServer.on("/single", HTTP_GET, [](AsyncWebServerRequest* request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();
      currentLED = message.toInt();
    } else {
      message = "No message sent";
    }
    Serial.println(message);
    request->send(200, "text/plain", "OK");
  });

  localServer.on("/color", HTTP_GET, [](AsyncWebServerRequest* request) {
    String message;
    if (request->hasParam(input_parameter)) {
      message = request->getParam(input_parameter)->value();
      ledColor = hexToRGB("#" + message);
      updateLEDs();
    } else {
      message = "No message sent";
    }
    Serial.println(message);
    request->send(200, "text/plain", "OK");
  });

  localServer.begin();
}

void updateLEDs() {
  switch (LEDmode) {
    case 1:
    default: setSelectedPixels(); break;
    case 2: strip.setPixelColor(currentLED, ledColor); break;
    case 3: strip.fill(ledColor); break;
  }
  strip.show();
}

void setSelectedPixels() {
  strip.fill(0);
  for (int x = 0; x < 16; x++) {
    if (LEDstates[x])
      strip.setPixelColor(x, ledColor);
  }
  strip.show();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void connectWifi() {
  if (WiFi.status() != WL_CONNECTED) {  // Connect to Wi-Fi network with SSID and password
    //Serial.print("Connecting to ");
    //Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

uint32_t hexToRGB(String hexstring) {
  // Convert hex string (e.g., "#RRGGBB") to a long integer, removing the "#" at the start
  unsigned long number = strtoul(hexstring.substring(1).c_str(), NULL, 16);

  // Split into R, G, B values
  unsigned long r = (number >> 16) & 0xFF;  // Extract the Red component
  unsigned long g = (number >> 8) & 0xFF;   // Extract the Green component
  unsigned long b = number & 0xFF;          // Extract the Blue component

  // Return the color in RGB format using strip.Color() for NeoPixel
  return strip.Color(r, g, b);
}