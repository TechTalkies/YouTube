/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266SAM.h>
#include "AudioOutputI2SNoDAC.h"

//Data for the web server. It is simple HTML code
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Tech Talkies</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; 
    background: linear-gradient(to right, red,orange,yellow,green,blue,indigo,violet, red);
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
	   .button { background-color: #195B6A; border: none; color: white; padding: 16px 20px;
    text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}
    .textarea {width:90%;}
  </style>
</head>

<body>
  <h2>Tech Talkies TTS</h2>
  <form>
		<label for="logText" maxlength="20">Enter text to speak:</label><br>

		<textarea class="textarea" id="logText" name="logText" rows="4"></textarea>
		<button class="button" type="submit" onclick="sendData()">Send</button> 
    </form><br>
	
<script>
function sendData(element) {
  var textValue = document.getElementById("logText").value;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/update?value="+textValue, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

AudioOutputI2SNoDAC* out;
AsyncWebServer server(80);
bool printDebug = false;
bool speak = false;
String message;

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
    else {
      message = WiFi.localIP().toString();
      speak = true;
    }
  }
}

void setup() {
  if (printDebug)
    Serial.begin(115200);

  connectWifi();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (request->hasParam("value")) {
      message = request->getParam("value")->value();
      if (printDebug)
        Serial.print(message);
      speak = true;
    }
  });

  server.begin();

  out = new AudioOutputI2SNoDAC();
  out->SetOutputModeMono(true);
  out->begin();
}

void loop() {
  if (speak) {
    ESP8266SAM* sam = new ESP8266SAM;
    sam->Say(out, message.c_str());
    delete sam;
    speak = false;
  }
}
