// 8x8 LED Matrix Breakout game
//
// For more info about this project (drawings, schematics, explanetations etc.), please visit my blog http://makertech.dk
// 
// * use at own risk *
//
// Author: Signe Colding-Jørgensen
// Maker Tech
// blog: http://makertech.dk
// facebook: https://www.facebook.com/makertechdk/
// instagram: https://www.instagram.com/makertech_dk/
// youtube: https://www.youtube.com/channel/UCZfXxUfpS7q-v1gsMY-hQtA
// twitter: https://twitter.com/MakerTech_dk

// This code is released under the GNU GPLv3 license (http://choosealicense.com/licenses/gpl-3.0/)

#include "ball.h"
#include "config.h"
#include "faces.h"
#include "paddle.h"
#include "Arduino.h"

class Breakout{
  Paddle paddle = {PADDLE_START, PADDLE_SIZE};
  Faces faces;
  
  void moveBall();
  void checkCollision();
  void resetBricks();
  void movePaddle(int controls);
  void checkSides();
  void hitEnd();
  void hitPaddle();

  public:
  Ball ball = {PADDLE_START+paddle.size/2, 1, 0, 1};
  char level[8][8];
  int speed = START_SPEED;
  int bricks = BRICKS;
  
  void restart();
  void updatePaddle();
  void won();
  void lost();
  void play(int controls);
  
};

