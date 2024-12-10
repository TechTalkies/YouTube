#include <LiquidCrystal_I2C.h>
#include "music.h"

#define BUZZERPIN 9
#define BUTTON_NEXT 2
#define BUTTON_PREV 4
#define BUTTON_SELECT 3  //Has to be interrup pins. 2 or 3 on the Nano

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// List of carols (add more as needed)
String carols[] = { "  Jingle Bells", "  Silent Night", "  Santa Claus", "  We Wish You", " Deck the Halls" };
int carolCount = sizeof(carols) / sizeof(carols[0]);
int currentCarol = 0;
volatile bool isPlaying = false;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_NEXT, INPUT_PULLUP);
  pinMode(BUTTON_PREV, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUZZERPIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_SELECT), playPause, LOW);

  lcd.init();
  lcd.backlight();
  printLCD();
}

void loop() {
  if (!isPlaying)
    checkButtons();
  else {
    playSelectedCarol(currentCarol);
  }
}

void playSelectedCarol(int index) {
  printLCD();

  switch (index) {
    case 0:
      playMelody(jingle, 220, sizeof(jingle));
      break;
    case 1:
      playMelody(silent, 130, sizeof(silent));
      break;
    case 2:
      playMelody(santa, 137, sizeof(santa));
      break;
    case 3:
      playMelody(weWish, 160, sizeof(weWish));
      break;
    case 4:
      playMelody(deck, 150, sizeof(deck));
      break;
  }
}

void playMelody(int16_t melody[], uint8_t songTempo, uint8_t size) {
  uint16_t notes = size / sizeof(melody[0]) / 2;
  uint16_t wholenote = (60000 * 4) / songTempo;
  int8_t divider;
  uint16_t noteDuration;
  for (uint16_t i = 0; i < notes * 2; i += 2) {
    divider = melody[i + 1];
    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else {
      noteDuration = wholenote / abs(divider) * 1.5;
    }
    tone(BUZZERPIN, melody[i], noteDuration * 0.9);
    if (!isPlaying) {
      noTone(BUZZERPIN);
      printLCD();
      return;
    }
    delay(noteDuration);
    noTone(BUZZERPIN);
  }
}

void printLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(carols[currentCarol]);
  lcd.setCursor(0, 1);
  if (isPlaying)
    lcd.print("<<<   Stop   >>>");
  else
    lcd.print("<<<   Play   >>>");
  //Serial.println(carols[currentCarol]);
}

void checkButtons() {
  if (digitalRead(BUTTON_NEXT) == LOW) {
    currentCarol = (currentCarol + 1) % carolCount;
    Serial.println("Next");
    printLCD();
    delay(300);  // Debounce
  }

  if (digitalRead(BUTTON_PREV) == LOW) {
    currentCarol = (currentCarol - 1 + carolCount) % carolCount;
    Serial.println("Previous");
    printLCD();
    delay(300);  // Debounce
  }
  /*
  if (digitalRead(BUTTON_SELECT) == LOW) {
    playSelectedCarol(currentCarol);
    Serial.println("Play");
    delay(300);  // Debounce
  }
  */
}

void playPause() {
  isPlaying = !isPlaying;
}