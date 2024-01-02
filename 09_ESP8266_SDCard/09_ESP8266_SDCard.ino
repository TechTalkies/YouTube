/*
   SD card attached to SPI bus as follows:
 ** MOSI - pin D7
 ** MISO - pin D6
 ** CLK - pin D5
 ** CS - pin D8

For Tech Talkies Youtube channel.
This example code is in the public domain.
*/

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <SD.h>
#include "webserver.h"

File myFile;
String fileName = "data.txt";

bool printDebug = true;

AsyncWebServer server(80);

const char* ssid = "wifi name";
const char* password = "password";

String processor(const String& var) {
  return String();
}

void connectWifi() {
  if (WiFi.status() != WL_CONNECTED) {  // Connect to Wi-Fi network with SSID and password
    //Serial.print("Connecting to ");
    //Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if (printDebug)
        Serial.print(".");
    }
    Serial.println("IP address: ");
    if (printDebug)
      Serial.println(WiFi.localIP());
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(4)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  connectWifi();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
    String message = "";
    if (request->hasParam("value")) {
      message = request->getParam("value")->value();
    }
    Serial.print(message);

    myFile = SD.open(fileName, FILE_WRITE);
    if (myFile) {
      myFile.println(message);
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening file");
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/getlog", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(SDFS, "/data.txt", "text/plain");
    request->send(response);
  });

  server.begin();
}

void loop() {
}
