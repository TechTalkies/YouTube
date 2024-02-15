//Original game at https://virushardrive.wordpress.com/2017/06/26/juego-arduino-tetris-con-matrices-leds-8x8-red-mod/
//Modified by Tech Talkies (https://www.youtube.com/@techtalkies1)

#include <LedControl.h>

/* Display PINs */
#define CLK 13
#define CS 10
#define DIN 11

LedControl lc = LedControl(DIN, CLK, CS, 1);

int lc0[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int lc1[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
long active[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
long screen[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int tmpCol = 0;

int figura = 0;
int figuraNext = 0;
int fromLeft = 0;
int fromRight = 0;
int angle = 0;
int colCheck = 0;
int moveCheck = 0;
int score = 0;
int started = 0;
int lcRows = 8;
int lcCols = 8;
int allLines = 0;
int currLines = 0;
int brickDelay = 0;
int defDelay = 500;
int level = 0;

boolean sound = true;
//Pinos

int start_button = 2;
int speaker_pin = 7;
int sound_button = 8;

byte X[8] = {
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b01010,
  0b10001,
  0b00000,
  0b00000
};

byte O[8] = {
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000
};

byte L[8] = {
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11000,
  0b11111,
  0b11111,
  0b00000
};

byte J[8] = {
  0b00011,
  0b00011,
  0b00011,
  0b00011,
  0b00011,
  0b11111,
  0b11111,
  0b00000
};

byte T[8] = {
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b01110,
  0b01110,
  0b00000,
  0b00000
};

byte I[8] = {
  0b01100,
  0b01100,
  0b01100,
  0b01100,
  0b01100,
  0b01100,
  0b01100,
  0b00000
};

byte Z[8] = {
  0b00000,
  0b00000,
  0b11110,
  0b11110,
  0b01111,
  0b01111,
  0b00000,
  0b00000
};

byte S[8] = {
  0b00000,
  0b00000,
  0b01111,
  0b01111,
  0b11110,
  0b11110,
  0b00000,
  0b00000
};

//Nuty
int length = 99;

void updateColumn(int colnum) {
  lc0[colnum] = active[colnum] >> (lcRows);
  lc1[colnum] = active[colnum];
  lc.setColumn(0, colnum, (screen[colnum] >> (lcRows)) | lc0[colnum]);
  lc.setColumn(1, colnum, screen[colnum] | lc1[colnum]);
}



void buttonDelay(int bdelay) {
  if (brickDelay > bdelay) {
    brickDelay -= bdelay;
  }
  delay(bdelay);
}


void splashScreen() {
  int up[] = {
    B11101110,  //   o o o   o o o
    B01001000,  //     o     o
    B01001100,  //     o     o o
    B01001000,  //     o     o
    B01001110,  //     o     o o o
    B00000000,  //
    B11101110,  //   o o o   o o o
    B01001010   //     o     o   o
  };

  int down[] = {
    B01001100,  //     o     o o
    B01001010,  //     o     o   o
    B01001001,  //     o     o     o
    B00000000,  //
    B01000111,  //     o       o o o
    B01000100,  //     o       o
    B01000010,  //     o         o
    B01001110   //     o     o o o
  };

  for (int rownum = 0; rownum < 8; rownum++) {
    lc.setRow(0, rownum, up[rownum]);
    lc.setRow(1, rownum, down[rownum]);
  }
}


void setup() {
  pinMode(speaker_pin, OUTPUT);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(start_button, INPUT_PULLUP);
  pinMode(sound_button, INPUT_PULLUP);

  lc.shutdown(0, false);

  lc.setIntensity(0, 5);

  lc.clearDisplay(0);
  lc.clearDisplay(1);
  Serial.begin(9600);

  randomSeed(analogRead(0));
}


//LOOP
void loop() {



  if (started == 0) {
    splashScreen();

    for (int i = 0; i < length; i++) {

      if (digitalRead(sound_button) == LOW) {
        sound = !sound;
        delay(300);
      }

      if (digitalRead(start_button) == LOW) {
        started = 1;
        break;
      }
    }
  }



  else {

    lc.clearDisplay(0);
    lc.clearDisplay(1);
    memset(lc0, 0, sizeof(lc0));
    memset(lc1, 0, sizeof(lc1));
    memset(active, 0, sizeof(active));
    memset(screen, 0, sizeof(screen));
    tmpCol = 0;

    while (started == 1) {

      if (allLines < 100) {
        level = 0;  //Level 0
      } else if (allLines < 200) {
        level = 1;  //Level 1
      } else if (allLines < 300) {
        level = 2;  //Level 2
      } else if (allLines < 400) {
        level = 3;  //Level 3
      } else if (allLines < 500) {
        level = 4;  //Level 4
      } else {
        level = 5;  //Level 5
      }


      defDelay = (5 - level) * 100;
      brickDelay = defDelay;


      if (figura == 0) {
        figura = random(1, 8);
      } else {
        figura = figuraNext;
      }
      figuraNext = random(1, 8);
      angle = 0;



      switch (figura) {
        case 1:
          //"O"
          active[3] = 131072 + 65536;
          active[4] = 131072 + 65536;
          fromLeft = 3;
          fromRight = 3;
          break;

        case 2:
          //"L"
          active[3] = 262144 + 131072 + 65536;
          active[4] = 65536;
          fromLeft = 3;
          fromRight = 3;
          break;

        case 3:
          //"J"
          active[3] = 65536;
          active[4] = 262144 + 131072 + 65536;
          fromLeft = 3;
          fromRight = 3;
          break;

        case 4:
          //"T"
          active[2] = 131072;
          active[3] = 131072 + 65536;
          active[4] = 131072;
          fromLeft = 2;
          fromRight = 3;
          break;

        case 5:
          //"I"
          active[3] = 524288 + 262144 + 131072 + 65536;
          fromLeft = 3;
          fromRight = 4;
          break;

        case 6:
          //"Z"
          active[2] = 131072;
          active[3] = 131072 + 65536;
          active[4] = 65536;
          fromLeft = 2;
          fromRight = 3;
          break;

        case 7:
          //"S"
          active[2] = 65536;
          active[3] = 131072 + 65536;
          active[4] = 131072;
          fromLeft = 2;
          fromRight = 3;
          break;
      }

      for (int krok = 0; krok < lcRows + 1; krok++) {
        colCheck = 0;

        for (int i = 0; i < (lcCols / 2); i++) {

          int dx = map(analogRead(A1), 0, 906, 2, -2);
          if (dx != 0) { dx = dx / abs(dx); }

          if ((dx > 0) && (fromLeft > 0)) {
            moveCheck = 0;
            for (int colnum = fromLeft; colnum < (lcCols - fromRight); colnum++) {
              if ((active[colnum] & screen[colnum - 1]) == 0) {
                moveCheck++;
              }
            }

            if (moveCheck == (lcCols - fromLeft - fromRight)) {
              for (int colnum = (fromLeft - 1); colnum < (lcCols - fromRight); colnum++) {
                if (colnum < (lcCols - 1)) {
                  active[colnum] = active[colnum + 1];
                } else {
                  active[colnum] = 0;
                }
                updateColumn(colnum);
              }
              fromLeft--;
              fromRight++;
              buttonDelay(200);
            }
          }
        }

        for (int i = 0; i < (lcCols / 2); i++) {
          int dx = map(analogRead(A1), 0, 906, 2, -2);
          if (dx != 0) { dx = dx / abs(dx); }

          if ((dx < 0) && (fromRight > 0)) {
            moveCheck = 0;
            for (int colnum = fromLeft; colnum < (lcCols - fromRight); colnum++) {
              if ((active[colnum] & screen[colnum + 1]) == 0) {
                moveCheck++;
              }
            }

            if (moveCheck == (lcCols - fromLeft - fromRight)) {
              for (int colnum = (lcCols - fromRight); colnum > (fromLeft - 1); colnum--) {
                if (colnum > 0) {
                  active[colnum] = active[colnum - 1];
                } else {
                  active[colnum] = 0;
                }
                updateColumn(colnum);
              }
              fromLeft++;
              fromRight--;
              buttonDelay(200);
            }
          }
        }

        int dx2 = map(analogRead(A0), 0, 906, 2, -2);
        if (dx2 != 0) { dx2 = -dx2 / abs(dx2); }

        if (dx2 > 0) {
          brickDelay = 0;
        } else {
          brickDelay = defDelay;
        }

          if (dx2 < 0) {
            switch (figura) {
              case 1:
                //"O"
                break;

              case 2:
                //"L"
                switch (angle) {
                  case 0:
                    // . o .        . . .
                    // . o .  --->  o o o
                    // . o o        o . .
                    if ((fromLeft > 0)
                        && (((active[fromLeft + 1] | (active[fromLeft + 1] << 1)) & screen[fromLeft - 1]) == 0)) {
                      active[fromLeft - 1] = (active[fromLeft + 1] | (active[fromLeft + 1] << 1));
                      updateColumn(fromLeft - 1);
                      active[fromLeft] = (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft + 1);
                      fromLeft--;
                      angle = 1;
                    }
                    break;

                  case 1:
                    // . . .        o o .
                    // o o o  --->  . o .
                    // o . .        . o .
                    if ((((active[fromLeft + 2] << 1) & screen[fromLeft]) == 0)
                        && ((((active[fromLeft + 1] << 1) | (active[fromLeft + 1] >> 1)) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft] = (active[fromLeft + 2] << 1);
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = active[fromLeft + 1] | (active[fromLeft + 1] << 1) | (active[fromLeft + 1] >> 1);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = 0;
                      updateColumn(fromLeft + 2);
                      fromRight++;
                      angle = 2;
                    }
                    break;

                  case 2:
                    // o o .        . . o
                    // . o .  --->  o o o
                    // . o .        . . .
                    if ((fromRight > 0)
                        && (((active[fromLeft] >> 1) & screen[fromLeft]) == 0)
                        && ((((active[fromLeft + 1] << 1) & active[fromLeft + 1]) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft] = (active[fromLeft] >> 1);
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = active[fromLeft];
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = ((active[fromLeft + 1] << 1) | active[fromLeft + 1]);
                      updateColumn(fromLeft + 2);
                      fromRight--;
                      krok--;
                      angle = 3;
                    }
                    break;

                  case 3:
                    // . . o        . o .
                    // o o o  --->  . o .
                    // . . .        . o o
                    if (((((active[fromLeft] << 1) | (active[fromLeft] >> 1)) & screen[fromLeft + 1]) == 0)
                        && (((active[fromLeft] >> 1) & screen[fromLeft + 2]) == 0)
                        && (krok < lcRows)) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft + 2] | (active[fromLeft + 2] >> 1));
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = ((active[fromLeft + 2] >> 1) & (active[fromLeft + 2] >> 2));
                      updateColumn(fromLeft + 2);
                      fromLeft++;
                      krok++;
                      angle = 0;
                    }
                    break;
                }
                break;

              case 3:
                //"J"
                switch (angle) {
                  case 0:
                    // . o .        o . .
                    // . o .  --->  o o o
                    // o o .        . . .
                    if ((fromRight > 0)
                        && ((((active[fromLeft] << 2) | (active[fromLeft] << 1)) & screen[fromLeft]) == 0)
                        && (((active[fromLeft] << 1) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft] = ((active[fromLeft] << 2) | (active[fromLeft] << 1));
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = ((active[fromLeft + 1] << 1) & (active[fromLeft + 1] >> 1));
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = active[fromLeft + 1];
                      updateColumn(fromLeft + 2);
                      fromRight--;
                      krok--;
                      angle = 1;
                    }
                    break;

                  case 1:
                    // o . .        . o o
                    // o o o  --->  . o .
                    // . . .        . o .
                    if ((krok < lcRows)
                        && ((((active[fromLeft + 1] << 1) | (active[fromLeft + 1] >> 1)) & screen[fromLeft + 1]) == 0)
                        && (((active[fromLeft + 2] << 1) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft + 1] | (active[fromLeft + 1] << 1) | (active[fromLeft + 1] >> 1));
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = (active[fromLeft + 2] << 1);
                      updateColumn(fromLeft + 2);
                      fromLeft++;
                      krok++;
                      angle = 2;
                    }
                    break;

                  case 2:
                    // . o o        . . .
                    // . o .  --->  o o o
                    // . o .        . . o
                    if ((fromLeft > 0)
                        && (((active[fromLeft + 1] >> 1) & screen[fromLeft - 1]) == 0)
                        && ((((active[fromLeft + 1] >> 1) | (active[fromLeft + 1] >> 2)) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft - 1] = (active[fromLeft + 1] >> 1);
                      updateColumn(fromLeft - 1);
                      active[fromLeft] = active[fromLeft - 1];
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft] | (active[fromLeft + 1] >> 2));
                      updateColumn(fromLeft + 1);
                      fromLeft--;
                      angle = 3;
                    }
                    break;

                  case 3:
                    // . . .        . o .
                    // o o o  --->  . o .
                    // . . o        o o .
                    if ((((active[fromLeft] >> 1) & screen[fromLeft]) == 0)
                        && ((((active[fromLeft] << 1) | (active[fromLeft >> 1])) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft] = (active[fromLeft] >> 1);
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = ((active[fromLeft + 1] << 1) | active[fromLeft + 2]);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = 0;
                      updateColumn(fromLeft + 2);
                      fromRight++;
                      angle = 0;
                    }
                    break;
                }
                break;

              case 4:
                //"T"
                switch (angle) {
                  case 0:
                    // . . .        . o .
                    // o o o  --->  o o .
                    // . o .        . o .
                    if (((active[fromLeft + 1] << 1) & screen[fromLeft + 1]) == 0) {
                      //active[fromLeft]
                      active[fromLeft + 1] = active[fromLeft + 1] | (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = 0;
                      updateColumn(fromLeft + 2);
                      fromRight++;
                      angle = 1;
                    }
                    break;

                  case 1:
                    // . o .        . o .
                    // o o .  --->  o o o
                    // . o .        . . .
                    if ((fromRight > 0)
                        && ((active[fromLeft] & screen[fromLeft + 2]) == 0)) {
                      //active[fromLeft]
                      active[fromLeft + 1] = active[fromLeft + 1] & (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = active[fromLeft];
                      updateColumn(fromLeft + 2);
                      fromRight--;
                      krok--;
                      angle = 2;
                    }
                    break;

                  case 2:
                    // . o .        . o .
                    // o o o  --->  . o o
                    // . . .        . o .
                    if ((((active[fromLeft + 1] >> 1) & screen[fromLeft + 1]) == 0)
                        && (krok < lcRows)) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = active[fromLeft + 1] | (active[fromLeft + 1] >> 1);
                      updateColumn(fromLeft + 1);
                      //active[fromLeft + 2]
                      fromLeft++;
                      krok++;
                      angle = 3;
                    }
                    break;

                  case 3:
                    if ((fromLeft > 0)
                        && ((active[fromLeft + 1] & screen[fromLeft - 1]) == 0)) {
                      active[fromLeft - 1] = active[fromLeft + 1];
                      updateColumn(fromLeft - 1);
                      active[fromLeft] = active[fromLeft] & (active[fromLeft] >> 1);
                      updateColumn(fromLeft);
                      fromLeft--;
                      angle = 0;
                    }
                    break;
                }
                break;

              case 5:
                //"I"
                switch (angle) {
                  case 0:
                    // . o . .        . . . .
                    // . o . .  --->  o o o o
                    // . o . .        . . . .
                    // . o . .        . . . .
                    if ((fromLeft > 0)
                        && (fromRight > 1)

                        && ((((((active[fromLeft] >> 1) & (active[fromLeft] << 2)) & screen[fromLeft - 1]) & screen[fromLeft + 1]) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft - 1] = ((active[fromLeft] >> 1) & (active[fromLeft] << 2));
                      updateColumn(fromLeft - 1);
                      active[fromLeft] = active[fromLeft - 1];
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = active[fromLeft];
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = active[fromLeft];
                      updateColumn(fromLeft + 2);
                      fromLeft--;
                      fromRight -= 2;
                      krok -= 2;
                      angle = 1;
                    }
                    break;

                  case 1:
                    // . . . .        . . o .
                    // o o o o  --->  . . o .
                    // . . . .        . . o .
                    // . . . .        . . o .
                    if ((krok < (lcRows - 1))
                        && (((active[fromLeft] << 1) | (active[fromLeft] >> 1) | (active[fromLeft] >> 2)) & screen[fromLeft + 2]) == 0) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = 0;
                      updateColumn(fromLeft + 1);

                      active[fromLeft + 2] = (active[fromLeft + 2] | (active[fromLeft + 2] << 1) | (active[fromLeft + 2] >> 1) | (active[fromLeft + 2] >> 2));
                      updateColumn(fromLeft + 2);
                      active[fromLeft + 3] = 0;
                      updateColumn(fromLeft + 3);
                      fromLeft += 2;
                      fromRight++;
                      krok += 2;
                      angle = 2;
                    }
                    break;

                  case 2:
                    // . . o .        . . . .
                    // . . o .  --->  . . . .
                    // . . o .        o o o o
                    // . . o .        . . . .
                    if ((fromLeft > 1)
                        && (fromRight > 0)

                        && ((((((active[fromLeft] << 1) & (active[fromLeft] >> 2)) & screen[fromLeft - 2]) & screen[fromLeft - 1]) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft - 2] = ((active[fromLeft] << 1) & (active[fromLeft] >> 2));
                      updateColumn(fromLeft - 2);
                      active[fromLeft - 1] = active[fromLeft - 2];
                      updateColumn(fromLeft - 1);
                      active[fromLeft] = active[fromLeft - 1];
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = active[fromLeft];
                      updateColumn(fromLeft + 1);
                      fromLeft -= 2;
                      fromRight--;
                      krok--;
                      angle = 3;
                    }
                    break;

                  case 3:
                    // . . . .        . o . .
                    // . . . .  --->  . o . .
                    // o o o o        . o . .
                    // . . . .        . o . .
                    if ((krok < (lcRows))
                        && (((active[fromLeft] >> 1) | (active[fromLeft] << 1) | (active[fromLeft] << 2)) & screen[fromLeft + 1]) == 0) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);

                      active[fromLeft + 1] = (active[fromLeft + 1] | (active[fromLeft + 1] >> 1) | (active[fromLeft + 1] << 1) | (active[fromLeft + 1] << 2));
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = 0;
                      updateColumn(fromLeft + 2);
                      active[fromLeft + 3] = 0;
                      updateColumn(fromLeft + 3);
                      fromLeft++;
                      fromRight += 2;
                      krok++;
                      angle = 0;
                    }
                    break;
                }
                break;

              case 6:
                //"Z"
                switch (angle) {
                  case 0:
                    // . . .        . o .
                    // o o .  --->  o o .
                    // . o o        o . .
                    if (((active[fromLeft + 1] & screen[fromLeft]) == 0)
                        && (((active[fromLeft + 1] << 1) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft] = active[fromLeft + 1];
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = 0;
                      updateColumn(fromLeft + 2);
                      fromRight++;
                      angle = 1;
                    }
                    break;

                  case 1:
                    // . o .        o o .
                    // o o .  --->  . o o
                    // o . .        . . .
                    if ((fromRight > 0)
                        && ((((active[fromLeft] << 2) & (active[fromLeft] << 1)) & screen[fromLeft]) == 0)
                        && (((active[fromLeft] & active[fromLeft + 1]) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft] = ((active[fromLeft] << 2) & (active[fromLeft] << 1));
                      updateColumn(fromLeft);
                      //active[fromLeft + 1]
                      active[fromLeft + 2] = (active[fromLeft] >> 1);
                      updateColumn(fromLeft + 2);
                      fromRight--;
                      krok--;
                      angle = 2;
                    }
                    break;

                  case 2:
                    // o o .        . . o
                    // . o o  --->  . o o
                    // . . .        . o .
                    if ((krok < lcRows)
                        && (((active[fromLeft + 1] >> 1) & screen[fromLeft + 1]) == 0)
                        && (((active[fromLeft + 2] << 1) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft + 1] >> 1);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = (active[fromLeft + 2] | (active[fromLeft + 2] << 1));
                      updateColumn(fromLeft + 2);
                      fromLeft++;
                      krok++;
                      angle = 3;
                    }
                    break;

                  case 3:
                    // . . o        . . .
                    // . o o  --->  o o .
                    // . o .        . o o
                    if ((fromLeft > 0)
                        && (((active[fromLeft] & active[fromLeft + 1]) & screen[fromLeft - 1]) == 0)
                        && (((active[fromLeft + 1] >> 1) & screen[fromLeft + 1]) == 0)) {
                      active[fromLeft - 1] = (active[fromLeft] & active[fromLeft + 1]);
                      updateColumn(fromLeft - 1);
                      //active[fromLeft]
                      active[fromLeft + 1] = (active[fromLeft - 1] >> 1);
                      updateColumn(fromLeft + 1);
                      fromLeft--;
                      angle = 0;
                    }
                    break;
                }
                break;

              case 7:
                //"S"
                switch (angle) {
                  case 0:
                    // . . .        o . .
                    // . o o  --->  o o .
                    // o o .        . o .
                    if (((active[fromLeft + 1] << 1) & screen[fromLeft]) == 0) {
                      active[fromLeft] = (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft);
                      //active[fromLeft + 1]
                      active[fromLeft + 2] = 0;
                      updateColumn(fromLeft + 2);
                      fromRight++;
                      angle = 1;
                    }
                    break;

                  case 1:
                    // o . .        . o o
                    // o o .  --->  o o .
                    // . o .        . . .
                    if ((fromRight > 0)
                        && (((active[fromLeft + 1] << 1) & screen[fromLeft + 1]) == 0)
                        && (((active[fromLeft] & (active[fromLeft] << 1)) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft] = (active[fromLeft] & active[fromLeft + 1]);
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft + 1] << 1);
                      updateColumn(fromLeft + 1);
                      active[fromLeft + 2] = (active[fromLeft] << 1);
                      updateColumn(fromLeft + 2);
                      fromRight--;
                      krok--;
                      angle = 2;
                    }
                    break;

                  case 2:
                    // . o o        . o .
                    // o o .  --->  . o o
                    // . . .        . . o
                    if ((krok < lcRows)
                        && (((active[fromLeft + 1] >> 1) & screen[fromLeft + 2]) == 0)) {
                      active[fromLeft] = 0;
                      updateColumn(fromLeft);
                      //active[fromLeft + 1]
                      active[fromLeft + 2] = (active[fromLeft + 1] >> 1);
                      updateColumn(fromLeft + 2);
                      fromLeft++;
                      krok++;
                      angle = 3;
                    }
                    break;

                  case 3:
                    // . o .        . . .
                    // . o o  --->  . o o
                    // . . o        o o .
                    if ((fromLeft > 0)
                        && ((active[fromLeft + 1] & ((active[fromLeft + 1] >> 1)) & screen[fromLeft - 1]) == 0)
                        && ((active[fromLeft + 1] & screen[fromLeft]) == 0)) {
                      active[fromLeft - 1] = (active[fromLeft + 1] & (active[fromLeft + 1] >> 1));
                      updateColumn(fromLeft - 1);
                      active[fromLeft] = active[fromLeft + 1];
                      updateColumn(fromLeft);
                      active[fromLeft + 1] = (active[fromLeft - 1] << 1);
                      updateColumn(fromLeft + 1);
                      fromLeft--;
                      angle = 0;
                    }
                    break;
                }
                break;
            }
            buttonDelay(200);
          }
        
        
        //Restart
        if (digitalRead(start_button) == LOW) {
          memset(lc0, 0, sizeof(lc0));
          memset(lc1, 0, sizeof(lc1));
          memset(active, 0, sizeof(active));
          memset(screen, 0, sizeof(screen));
          score = 0;
          allLines = 0;
          figura = 0;
          break;
        }

        for (int colnum = 0; colnum < lcCols; colnum++) {

          if ((screen[colnum] & (active[colnum] >> 1)) == 0) {
            colCheck++;
          }

          else {
            colCheck = 0;
            if (krok == 0) {
              started = 0;
            }
          }
        }

        if ((colCheck == lcCols) && (krok < lcRows)) {
          for (int colnum = 0; colnum < lcCols; colnum++) {
            active[colnum] = active[colnum] >> 1;
            updateColumn(colnum);
          }
        } else {
          break;
        }
        delay(brickDelay);
      }

      for (int colnum = 0; colnum < lcCols; colnum++) {
        screen[colnum] = screen[colnum] | (lc0[colnum] << (lcRows));
        screen[colnum] = screen[colnum] | lc1[colnum];
        lc0[colnum] = 0;
        lc1[colnum] = 0;
        active[colnum] = 0;
      }



      currLines = 0;
      for (int rownum = 0; rownum < lcRows; rownum++) {
        colCheck = 0;
        for (int colnum = 0; colnum < lcCols; colnum++) {
          if (((screen[colnum] >> rownum) & 1) == 1) {
            colCheck++;
          }
        }
        if (colCheck == lcCols) {
          //Animacja kasowania
          for (int colnum = 0; colnum < lcCols; colnum++) {
            tmpCol = ~((int)round(pow(2, rownum)));
            screen[colnum] = screen[colnum] & tmpCol;
            updateColumn(colnum);
            tmpCol = (int)(round(pow(2, rownum)) - 1);
            tmpCol = screen[colnum] & tmpCol;
            screen[colnum] = (screen[colnum] >> (rownum + 1));
            screen[colnum] = (screen[colnum] << rownum);
            screen[colnum] = screen[colnum] | tmpCol;
          }


          for (int colnum = 0; colnum < lcCols; colnum++) {
            updateColumn(colnum);
          }
          rownum--;
          currLines++;
          allLines++;
        }
      }

      if (currLines > 0) {
        score += (int)round(pow(4, currLines - 1));
      }
    }

    gameOver();
    // == Game Over ==
  }
}

void gameOver() {
  int cima[] = {
    B11111111,  //     o     o o o
    B11111111,  //   o o o    o
    B11111111,  //   o   o     o o
    B11111111,  //     o     o
    B11111111,  //     o     o o o
    B11111111,  //
    B11111111,  //   o o o   o o o
    B11111111   //     o     o   o
  };

  int baixo[] = {
    B11111111,  //     o     o o
    B11111111,  //     o     o   o
    B11111111,  //     o     o     o
    B11111111,  //
    B11111111,  //     o       o o o
    B11111111,  //     o       o
    B11111111,  //     o         o
    B11111111   //     o     o o o
  };

  for (int rownum = 8; rownum >= 0; rownum--) {
    lc.setRow(1, rownum, baixo[rownum]);
    delay(100);
  }

  for (int rownum = 8; rownum >= 0; rownum--) {
    lc.setRow(0, rownum, cima[rownum]);
    delay(100);
  }

  delay(1800);
}