int colorMe = 1;

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(D1, INPUT_PULLUP);
}

void loop() {

  if (digitalRead(D1) == LOW) {  // pressed (active low)
    colorMe++;

    if (colorMe > 3) colorMe = 1;
    delay(200);  // basic debounce
  }

  switch (colorMe) {
    case 1:
    default:
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, LOW);
      break;
    case 2:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(LED_GREEN, LOW);
      break;
    case 3:
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, HIGH);
      break;
  }
}