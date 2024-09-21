#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino_JSON.h>
#include "webserver.h"

const char* ssid = "Your_SSID"; //Your_SSID
const char* password = "Your_Password"; //Your_Password

AsyncWebServer server(80);
AsyncEventSource events("/events");

JSONVar readings;
unsigned long previous_time = 0;  
unsigned long previous_time_temp = 0;
unsigned long previous_time_acceleration = 0;
unsigned long gyro_delay  = 10;
unsigned long temperature_delay  = 1000;
unsigned long accelerometer_delay  = 200;

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

float rotationX, rotationY, rotationZ;
float accelerationX, accelerationY, accelerationZ;
float temperature;

float rotationX_error = 0.05;
float rotationY_error = 0.02;
float rotationZ_error = 0.01;

String getGyroscopeReadings(){
  mpu.getEvent(&a, &g, &temp);

  float rotationX_temporary = g.gyro.x;
  if(abs(rotationX_temporary) > rotationX_error)  {
    rotationX += rotationX_temporary*0.01;
  }
  
  float rotationY_temporary = g.gyro.y;
  if(abs(rotationY_temporary) > rotationY_error) {
    rotationY += rotationY_temporary*0.01;
  }

  float rotationZ_temporary = g.gyro.z;
  if(abs(rotationZ_temporary) > rotationZ_error) {
    rotationZ += rotationZ_temporary*0.01;
  }

  readings["rotationX"] = String(rotationX);
  readings["rotationY"] = String(rotationY);
  readings["rotationZ"] = String(rotationZ);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getAccelerationReadings() {
  mpu.getEvent(&a, &g, &temp);
  accelerationX = a.acceleration.x;
  accelerationY = a.acceleration.y;
  accelerationZ = a.acceleration.z;
  readings["accelerationX"] = String(accelerationX);
  readings["accelerationY"] = String(accelerationY);
  readings["accelerationZ"] = String(accelerationZ);
  String accString = JSON.stringify (readings);
  return accString;
}

String getTemperatureReadings(){
  mpu.getEvent(&a, &g, &temp);
  temperature = temp.temperature;
  return String(temperature);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println(WiFi.localIP());

  // Initialize LittleFS
  /*
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  */
  
  if (!mpu.begin()) {
    Serial.println("MPU6050 is not properly connected. Check circuit!");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    //request->send(LittleFS, "/index.html", "text/html");
    request->send_P(200, "text/html", index_html);
  });

  //server.serveStatic("/",LittleFS, "/");

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationX=0;
    rotationY=0;
    rotationZ=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationX=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationY=0;
    request->send(200, "text/plain", "OK");
  });

  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request){
    rotationZ=0;
    request->send(200, "text/plain", "OK");
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  server.begin();
}

void loop() {
  if ((millis() - previous_time) > gyro_delay ) {
    events.send(getGyroscopeReadings().c_str(),"gyro_readings",millis());
    previous_time = millis();
  }
  if ((millis() - previous_time_acceleration) > accelerometer_delay ) {
    events.send(getAccelerationReadings().c_str(),"accelerometer_readings",millis());
    previous_time_acceleration = millis();
  }
  if ((millis() - previous_time_temp) > temperature_delay ) {
    events.send(getTemperatureReadings().c_str(),"temperature_reading",millis());
    previous_time_temp = millis();
  }
}