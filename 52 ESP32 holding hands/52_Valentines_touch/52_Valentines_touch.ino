#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const String messages[] = { " Made for each other", "Love birds", "Two hearts One beat", "Perfect   match",
                            "Double the love", "Together, unstoppable" };

#define TOUCH_PAD_1 13  // First touch pad
#define TOUCH_PAD_2 15  // Second touch pad
#define TOUCH_THRESHOLD 20
#define CONTACT_THRESHOLD 5

bool person1 = false;
bool person2 = false;
bool holding_hands = false;
bool state_changed = false;

String currentMessage;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  delay(2000);  // Pause for 2 seconds
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.display();
}

void loop() {
  int touch1 = touchRead(TOUCH_PAD_1);
  int touch2 = touchRead(TOUCH_PAD_2);

  person1 = touch1 < TOUCH_THRESHOLD;
  person2 = touch2 < TOUCH_THRESHOLD;

  if (touch1 < CONTACT_THRESHOLD & touch2 < CONTACT_THRESHOLD) {
    holding_hands = true;
  } else {
    holding_hands = false;
    currentMessage = messages[random(6)];
  }

  Serial.print(touch1);
  Serial.print(" ");
  Serial.print(touch2);
  Serial.print(" ");
  Serial.println(holding_hands);

  if (holding_hands) {
    display.clearDisplay();
    display.display();
    delay(1000);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(currentMessage);
    display.display();
    delay(1000);
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(30, 0);
    display.println("Love Meter");
    display.setCursor(7, 12);
    display.println("Touch the pins and");
    display.println("     hold hands");
    display.display();
    delay(100);
  }
}