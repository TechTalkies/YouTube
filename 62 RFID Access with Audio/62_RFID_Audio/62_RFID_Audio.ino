/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <SPI.h>
#include <MFRC522.h>
#include "DFRobotDFPlayerMini.h"

#define SS_PIN 5   //D4
#define RST_PIN 0  //D3
#define RXD2 16
#define TXD2 17

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

// Create the Player object
DFRobotDFPlayerMini player;

String secureIDS[] = {
  "8A 0C 84 96",  //0001
  "2A 94 3A 96",  //0002
  "D8 F6 F3 99"   //0003
};

int len = sizeof(secureIDS) / sizeof(secureIDS[0]);
int i;
void setup() {

  // Init USB serial port for debugging
  Serial.begin(9600);

  SPI.begin();         // Initiate  SPI bus
  mfrc522.PCD_Init();  // Initiate MFRC522
  mfrc522.PCD_DumpVersionToSerial();

  delay(1000);

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // Init serial port for DFPlayer Mini

  Serial.print("DFMini init");

  // Start communication with DFPlayer Mini
  while (!player.begin(Serial2)) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("DFMini OK");
  // Set volume to maximum (0 to 30).
  player.volume(30);
  // Play the first MP3 file on the SD card
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
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();

  bool accessGranted = false;

  for (i = 0; i < len; ++i) {
    if (content.substring(1) == secureIDS[i]) {
      accessGranted = true;
      break;
    }
  }

  if (accessGranted) player.play(1);
  else player.play(2);

  delay(2000);
}