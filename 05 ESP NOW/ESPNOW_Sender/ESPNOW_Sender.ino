#include <ESP8266WiFi.h>
#include <espnow.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

String myData = "Test";

// Callback function called when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == 0 ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Init Serial Monitor
  Serial.begin(74880);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("Initialized ESP-NOW");
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Add peer
  if (esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 0, NULL, 0) != 0) {
    Serial.println("Failed to add peer");
    return;
  } else {
    Serial.println("Added peer");
  }

  if (esp_now_is_peer_exist(broadcastAddress)) {
    Serial.println("Peer exists");
  } else
    Serial.println("No exists");
}

void loop() {
  if (Serial.available() > 0) {
    myData = Serial.readString();  //read until timeout
    myData.trim();                 // remove any \r \n whitespace at the end of the String
    Serial.println(myData);

    // Send message via ESP-NOW
    int result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

    if (result == 0) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  }

  delay(1000);
}
