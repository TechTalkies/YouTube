//Original game by Pedro Ney Stroski- https://www.electricalelibrary.com/en/2021/11/26/pong-with-arduino/
//Modified by Tech Talkies (https://www.youtube.com/@techtalkies1)

#include "LedControl.h"  //Include library to control  //the MAX7219 matrix.

/* Display PINs */
#define CLK 13
#define CS 10
#define DIN 11

LedControl myMatrix = LedControl(DIN, CLK, CS, 1);

// 1.Global variables ---
int column = 1, row = 1;  //Decides which point the ball must //start.
int directionX = 1, directionY = 1;   //Certificates that the ball go first //from right to left.
int paddle1 = 5, paddle1Val = 2;      //Pin and potentiometer value.
int velocidade = 300;                 //Ball's initial speed.
int counter = 0, mult = 10;
char score[4];  // Variable to convert int counter in char.
// end 1. ---

//2.Initial settings ----

// Variables used for rolling score, both start on 0.
int index = 0;  // Current char to be shown.
int shift = 0;  // Quantity of shifting columns.

// Array to store bit map to be shown.
int bitsMap[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
// Vector with power of 2 to calculate what to show.
const int twoPowers[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

int charachter;
#define displayspeed 7  //Score exhibition speed, less is faster.
//End 2. ----

//How numbers must appear on display.
const int zero[] = { B00000000,
                     B00000000,
                     B01110000,
                     B01010000,
                     B01010000,
                     B01010000,
                     B01110000,
                     B00000000 };
const int one[] = { B00000000,
                    B00000000,
                    B00100000,
                    B01100000,
                    B00100000,
                    B00100000,
                    B01110000,
                    B00000000 };
const int two[] = { B00000000,
                    B00000000,
                    B01110000,
                    B00010000,
                    B01110000,
                    B01000000,
                    B01110000,
                    B00000000 };
const int three[] = { B00000000,
                      B00000000,
                      B01110000,
                      B00010000,
                      B00110000,
                      B00010000,
                      B01110000,
                      B00000000 };
const int four[] = { B00000000,
                     B00000000,
                     B01000000,
                     B01010000,
                     B01110000,
                     B00010000,
                     B00010000,
                     B00000000 };
const int five[] = { B00000000,
                     B00000000,
                     B01110000,
                     B01000000,
                     B01110000,
                     B00010000,
                     B01110000,
                     B00000000 };
const int six[] = { B00000000,
                    B00000000,
                    B01110000,
                    B01000000,
                    B01110000,
                    B01010000,
                    B01110000,
                    B00000000 };
const int seven[] = { B00000000,
                      B00000000,
                      B01110000,
                      B00010000,
                      B00010000,
                      B00010000,
                      B00010000,
                      B00000000 };
const int eight[] = { B00000000,
                      B00000000,
                      B01110000,
                      B01010000,
                      B01110000,
                      B01010000,
                      B01110000,
                      B00000000 };
const int nine[] = { B00000000,
                     B00000000,
                     B01110000,
                     B01010000,
                     B01110000,
                     B00010000,
                     B00010000,
                     B00000000 };

void setup() {
  myMatrix.shutdown(0, false);  //Habilitate display.
  myMatrix.setIntensity(0, 8);  //Define medium brightness.
  myMatrix.clearDisplay(0);     //Clear display.
  randomSeed(analogRead(0));    //Creates a random seed.
  Serial.begin(9600);           //To appear score on serial //screen.
}

void loop() {
  //paddle1Val = analogRead(paddle1);
  //paddle1Val = map(analogRead(A1), 0, 906, 2, -2);
  if (map(analogRead(A1), 0, 906, 2, -2) < 0)
    paddle1Val = paddle1Val + 1;

  if (map(analogRead(A1), 0, 906, 2, -2) > 0)
    paddle1Val = paddle1Val - 1;

  if (paddle1Val > 6) paddle1 = 6;
  if (paddle1Val < 0) paddle1 = 0;

  column += directionX;
  row += directionY;

  if (column == 6 && directionX == 1 && (paddle1Val == row || paddle1Val + 1 == row || paddle1Val + 2 == row)) directionX = -1;

  if (column == 0 && directionX == -1) directionX = 1;

  if (row == 7 && directionY == 1) directionY = -1;

  if (row == 0 && directionY == -1) directionY = 1;

  if (column == 7) {
    oops();
  }
  myMatrix.clearDisplay(0);  //Clears screens for the //next animation frame.
  myMatrix.setLed(0, column, row, HIGH);
  myMatrix.setLed(0, 7, paddle1Val, HIGH);
  myMatrix.setLed(0, 7, paddle1Val + 1, HIGH);
  myMatrix.setLed(0, 7, paddle1Val + 2, HIGH);

  if (!(counter % mult)) {
    velocidade -= 5;
    mult* mult;
  }
  delay(velocidade);
  counter++;
}

void oops()  //Game over function.
{
  delay(1000);
  if (counter < 100) {  //If score is lower than 100.
    for (int y = 0; y < 40; y++) {
      loadMatrix2();
      drawScreen(bitsMap);
      //What the serial screen must show.
      Serial.println("Game Over");
      Serial.print(counter);
      Serial.println("\n");
      Serial.println("Score:");
      Serial.print(score);
      Serial.println("\n");
    }
  } else {  //If score is higher than 100.
    for (int y = 0; y < 40; y++) {
      loadMatrix();
      drawScreen(bitsMap);
      //What the serial screen must show.
      Serial.println("Game Over");
      Serial.print(counter);
      Serial.println("\n");
      Serial.println("Score:");
      Serial.print(score);
      Serial.println("\n");
    }
  }
  delay(1000);

  counter = 0;  //Reinitiate all values.
  velocidade = 300;
  column = 1;
  row = random(8) + 1;
}

void loadMatrix() {          //Load matrix to show 3 digits.
  itoa(counter, score, 10);  //Functions that convert int in char strings of 10 //base, to be shown at display.
  getChar(score[index]);     // Reads current phrase char.
  int currentChar = charachter;
  getChar(score[index + 1]);  // Reads next char.
  int nextChar = charachter;
  executeMatrix(currentChar, nextChar);
}

void loadMatrix2() {  //Load matrix to show 1 or 2 digits.
  itoa(counter, score, 10);
  getChar(score[0]);  // Reads current phrase char..
  int currentChar = charachter;
  getChar(score[1]);  // Reads next char.
  int nextChar = charachter;
  executeMatrix(currentChar, nextChar);
}

// Creates the matrix's bit map.
void executeMatrix(int currentBuffer[], int nextBuffer[]) {
  for (int r = 0; r < 8; r++) {                                                   // Increments r to r from 0 to 7 (row - line).
    bitsMap[r] = 0;                                                               // Reset line.
    for (int c = 0; c < 8; c++) {                                                 // Increments c to c from 0 to 7 (column - //coluna).
      bitsMap[r] = bitsMap[r] + ((twoPowers[c] & (currentBuffer[r] << shift)));   // Loads current char compensated by shifting.
      bitsMap[r] = bitsMap[r] + (twoPowers[c] & (nextBuffer[r] >> (8 - shift)));  // Loads the next char compensated by shifting.
    }
  }
  shift++;  // Increments the shift variable responsible by one line //shifting.

  if (shift == 8) {
    shift = 0;
    index++;
    //if(index == sizeof(texto)-2){
    if (index == sizeof(score) - 2) {
      index = 0;
    }
  }  // If shift is equal to 8, load the next char pair. //(currentChar and nextCahr).
}

void drawScreen(int buffer[]) {
  for (int v = 0; v < displayspeed; v++) {
    for (int i = 0; i < 8; i++) {    // linhas
      for (int j = 0; j < 8; j++) {  // colunas
        myMatrix.setLed(0, i, j, bitRead(buffer[i], 7 - j));
        //using bitRead()
      }
    }
  }
}

void getChar(char msgChar) {
  switch (msgChar) {
    case '0': charachter = zero; break;
    case '1': charachter = one; break;
    case '2': charachter = two; break;
    case '3': charachter = three; break;
    case '4': charachter = four; break;
    case '5': charachter = five; break;
    case '6': charachter = six; break;
    case '7': charachter = seven; break;
    case '8': charachter = eight; break;
    case '9': charachter = nine; break;
  }
}