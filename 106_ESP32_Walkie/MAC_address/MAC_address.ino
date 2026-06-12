/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <WiFi.h>

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set Wi-Fi to Station mode
  WiFi.mode(WIFI_MODE_STA);
  WiFi.STA.begin();

  // Print the MAC address
  Serial.print("\nESP32 MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Nothing to do here
}
