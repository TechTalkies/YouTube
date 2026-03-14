/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <ESP32Servo.h>

Servo myservo, myservo2;  // create servo object to control a servo

int servoPin = 13;   // GPIO pin used to connect the servo control (digital out)
int servoPin2 = 12;  // GPIO pin used to connect the servo control (digital out)
int potPin = 36;     // GPIO pin used to connect the potentiometer (analog in)

int ADC_Max = 4096;  // This is the default ADC max value on the ESP32 (12 bit ADC width);
                     // this width can be set (in low-level oode) from 9-12 bits, for a
                     // a range of max values of 512-4096

int val;  // variable to read the value from the analog pin

void setup() {
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);  // Standard 50hz servo
  myservo.attach(servoPin);    // attaches the servo on pin
  myservo2.attach(servoPin2);  // attaches the servo on pin
}

void loop() {
  val = analogRead(potPin);            // read the value of the potentiometer (value between 0 and 1023)
  val = map(val, 0, ADC_Max, 0, 180);  // scale it to use it with the servo (value between 0 and 180)
  myservo.write(val);                  // set the servo position according to the scaled value
  myservo2.write(val);                  // set the servo position according to the scaled value
  delay(200);                          // wait for the servo to get there
}
