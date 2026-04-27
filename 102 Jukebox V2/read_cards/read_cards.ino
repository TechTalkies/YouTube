/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define SCK_PIN 6
#define MOSI_PIN 7
#define MISO_PIN 8
#define RST_PIN 9


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

void setup() {
  Serial.begin(9600);                              // Initiate a serial communication
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);  // Initiate  SPI bus
  mfrc522.PCD_Init();                              // Initiate MFRC522
  mfrc522.PCD_DumpVersionToSerial();
}

void loop() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  delay(2000);
}