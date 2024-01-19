#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "webserver.h"

#define SS_PIN 2     //D4
#define RST_PIN 0    //D3
#define REDLED 16    //D1
#define GREENLED 15  //D8

LiquidCrystal_I2C lcd(0x27, 16, 2);

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

const char* ssid = "WiFi Name";
const char* password = "Password";

AsyncWebServer localServer(80);

uint8_t PERSON1 = 0;
uint8_t PERSON2 = 0;
uint8_t PERSON3 = 0;
uint8_t UNKNOWN = 0;

String processor(const String& var) {
  if (var == "PERSON1") {
    return String(PERSON1);
  }
  if (var == "PERSON2") {
    return String(PERSON2);
  }
  if (var == "PERSON3") {
    return String(PERSON3);
  }
  if (var == "UNKNOWN") {
    return String(UNKNOWN);
  }
  return String();
}


void setup() {
  Serial.begin(9600);  // Initiate a serial communication
  SPI.begin();         // Initiate  SPI bus
  mfrc522.PCD_Init();  // Initiate MFRC522
  mfrc522.PCD_DumpVersionToSerial();

  byte count = 0;

  Wire.begin(4, 5);

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
      delay(1);  // maybe unneeded?
    }            // end of good response
  }              // end of for loop
  Serial.println("Done.");
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.println(" device(s).");

  lcd.init();
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting");

  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);

  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, LOW);

  system_set_os_print(0);
  connectWifi();

  localServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  localServer.begin();
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

  if (content.substring(1) == "F3 BE 8A 11")  //change UID of the card that you want to give access
  {
    lcd.clear();
    //delay(500);
    lcd.setCursor(0, 0);
    lcd.print("Access Granted:");
    lcd.setCursor(0, 1);
    lcd.print("Red");
    PERSON1++;
    digitalWrite(GREENLED, HIGH);
  } else if (content.substring(1) == "D8 F6 F3 99")  //change UID of the card that you want to give access
  {
    lcd.clear();
    //delay(500);
    lcd.setCursor(0, 0);
    lcd.print("Access Granted:");
    lcd.setCursor(0, 1);
    lcd.print("Blue");
    PERSON2++;
    digitalWrite(GREENLED, HIGH);
  }else if (content.substring(1) == "2A 94 3A 96")  //change UID of the card that you want to give access
  {
    lcd.clear();
    //delay(500);
    lcd.setCursor(0, 0);
    lcd.print("Access Granted:");
    lcd.setCursor(0, 1);
    lcd.print("Green");
    PERSON3++;
    digitalWrite(GREENLED, HIGH);
  } else {
    lcd.clear();
    //delay(500);
    lcd.setCursor(0, 0);
    lcd.print("Access Denied:");
    lcd.setCursor(0, 1);
    lcd.print("ID:");
    lcd.print(content);
    UNKNOWN++;
    digitalWrite(REDLED, HIGH);
  }

  delay(2000);
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Ready");
  lcd.setCursor(2, 1);
  lcd.print("Scan a card");
}

void connectWifi() {
  if (WiFi.status() != WL_CONNECTED) {  // Connect to Wi-Fi network with SSID and password
    //Serial.print("Connecting to ");
    //Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      lcd.print(".");
      //Serial.print(".");
    }
    // Print local IP address and start web server
    //Serial.println("");
    //Serial.println("WiFi connected.");
    //Serial.println("IP address: ");
    //if (printDebug)
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ready");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  }
}