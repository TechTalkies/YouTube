/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

// RF Remote LED Control
// A = Toggle ON/OFF
// B = Switch Mode (Static / Blink)
// C = Adjust Brightness
// D = Reserved

#define BTN_A A1
#define BTN_B A3
#define BTN_C A4
#define LED_PIN 9

// State variables
bool ledOn = false;
int mode = 0;           // 0 = static, 1 = blink
int maxBrightness = 255; // Brightness: 0–255

unsigned long lastButtonTime = 0;
const unsigned long debounceTime = 200;

unsigned long previousMillis = 0;
const unsigned long blinkInterval = 300; // Blink speed
bool blinkState = false;

void setup() {
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_C, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  analogWrite(LED_PIN, 0);  // LED OFF initially

  Serial.begin(9600);
  Serial.println("RF Remote LED Control Started");
}

void loop() {
  // Button A = Toggle LED ON/OFF
  if (digitalRead(BTN_A) == HIGH && debounce()) {
    ledOn = !ledOn;
    if (!ledOn) analogWrite(LED_PIN, 0);
    Serial.println("Button A: LED Toggle");
  }

  // Only react to B/C if LED is ON
  if (ledOn) {
    // Button B = Mode change (Static / Blink)
    if (digitalRead(BTN_B) == HIGH && debounce()) {
      mode++;
      if (mode > 1) mode = 0; // Wrap to static
      Serial.print("Button B: Mode = ");
      Serial.println(mode == 0 ? "Static" : "Blink");
    }

    // Button C = Adjust brightness
    if (digitalRead(BTN_C) == HIGH && debounce()) {
      maxBrightness += 30;
      if (maxBrightness > 255) maxBrightness = 0; // Reset to 0
      Serial.print("Button C: Brightness = ");
      Serial.println(maxBrightness);
    }

    // ------- LED Behavior --------
    if (mode == 0) {
      // Static
      analogWrite(LED_PIN, maxBrightness);
    } 
    else if (mode == 1) {
      // Blink
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;
        blinkState = !blinkState;
        analogWrite(LED_PIN, blinkState ? maxBrightness : 0);
      }
    }

  } else {
    delay(10); // Idle
  }
}

// Debounce helper
bool debounce() {
  if (millis() - lastButtonTime > debounceTime) {
    lastButtonTime = millis();
    return true;
  }
  return false;
}
