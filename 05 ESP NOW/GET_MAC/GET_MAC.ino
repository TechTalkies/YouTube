#include <ESP8266WiFi.h>
 
void setup(){
  Serial.begin(74880);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}