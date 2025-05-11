/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
// For DRV8833

// Define the control inputs
#define IN1 9
#define IN2 10
#define IN3 6
#define IN4 5

void setup(void) {
  // Set all the motor control inputs to OUTPUT
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Turn off motors - Initial state
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void loop(void) {

  // motor 1 forward
  analogWrite(IN1, 255 / 4);
  digitalWrite(IN2, LOW);
  delay(2000);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(500);

  // motor 2
  analogWrite(IN3, 255 / 4);
  digitalWrite(IN4, LOW);
  delay(2000);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(500);

  //Both motors slow
  analogWrite(IN1, 255 / 4);
  digitalWrite(IN2, LOW);
  analogWrite(IN3, 255 / 4);
  digitalWrite(IN4, LOW);
  delay(2000);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(500);

  //Both motors fast
  analogWrite(IN1, 255 / 2);
  digitalWrite(IN2, LOW);
  analogWrite(IN3, 255 / 2);
  digitalWrite(IN4, LOW);
  delay(2000);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(500);
}