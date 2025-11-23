/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Servo.h>

// Button wiring
int button = A1;   //pin of the first button
int button1 = A3;  //pin  of the second button

Servo servo;  //create a servo object
int pos = 0;  //initial position of the servo
void  setup() {
  // put your setup code here, to run once:
  servo.attach(9);  //pin  used by the servo
  pinMode(button, INPUT_PULLUP);  //define first button as  input pullup
  pinMode(button1, INPUT_PULLUP); //define second button as input  pullup
  /*
  INPUT_PULLUP send to arduino LOW signal, so, when you press  the button, you send a LOW signal to arduino
  */
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(button) ==  LOW) { //if Value read of the button ==LOW:
    pos++;  //increases the value  of the "pos" variable each time the push button of the left is pressed
    delay(5);  //5 milliseconds of delay
    servo.write(pos); //servo goes to variable pos
  }
  if (digitalRead(button1) == LOW) { //if Value read of the button ==LOW:
    pos--;  //decreases the value of the "pos" variable each time the push button  of the right is pressed
    delay(5); //5 milliseconds of delay
    servo.write(pos);  //servo goes to variable pos
  }
}