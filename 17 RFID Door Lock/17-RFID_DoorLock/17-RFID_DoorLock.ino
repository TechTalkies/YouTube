#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

Servo myServo;

#define SS_PIN 2    //D4
#define RST_PIN 0   //D3
#define REDLED 16   //D0
#define GREENLED 5  //D1

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

bool locked;

void setup() {
  Serial.begin(9600);  // Initiate a serial communication
  SPI.begin();         // Initiate  SPI bus
  mfrc522.PCD_Init();  // Initiate MFRC522
  mfrc522.PCD_DumpVersionToSerial();

  myServo.attach(4);  // servo attach D2 pin of arduino
  locked = false;

  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
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

  if (content.substring(1) == "2A 94 3A 96")  //change UID of the card that you want to give access
  {
    digitalWrite(GREENLED, HIGH);
    if (locked)
      myServo.write(0);
    else myServo.write(160);

    locked = !locked;
  } else
    digitalWrite(REDLED, HIGH);

  delay(2000);
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);
}
