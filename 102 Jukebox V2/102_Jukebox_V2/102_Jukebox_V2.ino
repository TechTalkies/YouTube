/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

static const uint8_t PIN_MP3_TX = 0; //TX of DF Mini
static const uint8_t PIN_MP3_RX = 1;

#define SS_PIN 5
#define SCK_PIN 6
#define MOSI_PIN 7
#define MISO_PIN 8
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
EspSoftwareSerial::UART myPort;
DFRobotDFPlayerMini player;
Adafruit_SH1106G display(128, 64, &Wire, -1);

struct SongCard {
  const char *uid;
  uint16_t track;
};

struct FolderCard {
  const char *uid;
  uint8_t folder;
};

// Individual song cards
SongCard songCards[] = {
  { "AA BB CC DD", 1 },
  { "AA BB CC DD", 2 },
  { "AA BB CC DD", 3 },
};

const uint8_t SONG_COUNT = sizeof(songCards) / sizeof(songCards[0]);

// Folder cards
FolderCard folderCards[] = {
  { "AA BB CC DD", 1 },
  { "AA BB CC DD", 2 }
};

const uint8_t FOLDER_COUNT = sizeof(folderCards) / sizeof(folderCards[0]);

void showScreen(const char *line2) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Playing");
  display.println("----------");
  display.println(line2);
  display.display();
}

String readUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (i > 0) uid += " ";
    if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void playFolderAll(uint8_t folder) {
  Serial.print("Folder ");
  Serial.println(folder);
  player.loopFolder(folder);
  char buf[20];
  sprintf(buf, "Folder %03d", folder);
  showScreen(buf);
}

void handleCard(String uid) {
  Serial.print("Card: ");
  Serial.println(uid);

  for (uint8_t i = 0; i < SONG_COUNT; i++) {
    if (uid == songCards[i].uid) {
      Serial.print("Song track ");
      Serial.println(songCards[i].track);
      player.play(songCards[i].track);
      char buf[20];
      sprintf(buf, "Track %03d", songCards[i].track);
      showScreen(buf);
      return;
    }
  }

  for (uint8_t i = 0; i < FOLDER_COUNT; i++) {
    if (uid == folderCards[i].uid) {
      Serial.println("Folder card detected");
      playFolderAll(folderCards[i].folder);
      return;
    }
  }

  Serial.println("Unknown card");
}

void setup() {
  Wire.begin(2, 3);
  display.begin(0x3C, true);
  showScreen("Ready");
  Serial.begin(9600);

  myPort.begin(9600, SWSERIAL_8N1, PIN_MP3_TX, PIN_MP3_RX, false);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  if (player.begin(myPort)) {
    player.volume(25);
  }
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = readUID();
  handleCard(uid);

  mfrc522.PICC_HaltA();
  delay(1500);
}
