/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 150    // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600   // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50  // Analog servos run at ~50 Hz updates

// our servo # counter
uint8_t servonum = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("8 channel Servo test!");

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);
}

void loop() {

  for (int posDegrees = 0; posDegrees <= 180; posDegrees++) {

    int pwm0 = map(posDegrees, 0, 180, SERVOMIN, SERVOMAX);
    int pwm1 = map(constrain(posDegrees - 20, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
    int pwm2 = map(constrain(posDegrees - 40, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
    int pwm3 = map(constrain(posDegrees - 60, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
    int pwm4 = map(constrain(posDegrees - 80, 0, 180), 0, 180, SERVOMIN, SERVOMAX);

    pwm.setPWM(0, 0, pwm0);
    pwm.setPWM(1, 0, pwm1);
    pwm.setPWM(2, 0, pwm2);
    pwm.setPWM(3, 0, pwm3);
    pwm.setPWM(4, 0, pwm4);

    delay(30);
  }

  for (int posDegrees = 180; posDegrees >= 0; posDegrees--) {

    int pwm0 = map(posDegrees, 0, 180, SERVOMIN, SERVOMAX);
    int pwm1 = map(constrain(posDegrees - 20, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
    int pwm2 = map(constrain(posDegrees - 40, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
    int pwm3 = map(constrain(posDegrees - 60, 0, 180), 0, 180, SERVOMIN, SERVOMAX);
    int pwm4 = map(constrain(posDegrees - 80, 0, 180), 0, 180, SERVOMIN, SERVOMAX);

    pwm.setPWM(0, 0, pwm0);
    pwm.setPWM(1, 0, pwm1);
    pwm.setPWM(2, 0, pwm2);
    pwm.setPWM(3, 0, pwm3);
    pwm.setPWM(4, 0, pwm4);

    delay(30);
  }
}
