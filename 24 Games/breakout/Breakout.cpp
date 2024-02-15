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

#include "Breakout.h"

void Breakout::restart(){
  paddle = {PADDLE_START, PADDLE_SIZE};
  ball = {PADDLE_START+paddle.size/2, 1, 0, 1};
  bricks = BRICKS;

  resetBricks();
  updatePaddle();
  level[7-ball.y][ball.x] = 'O';
}

void Breakout::resetBricks(){
  int bricksSetup = 0;
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
      if(bricksSetup < bricks){
        level[i][j] = 'B';
        bricksSetup++;
      }
      else{
        level[i][j] = 'E';
      }
    }  
  }}

void Breakout::updatePaddle(){
  // clear horizontal
  for(int i = 0; i < 8; i++){
    level[7][i] = 'E';
  }
  // add paddle
  for(int i = 0; i < paddle.size; i++){
    level[7][paddle.pos+i] = 'P';  
  }
}

void Breakout::won(){

  // decrease speed variable increases difficulty (this really should be made to work the opposite way)
  speed--;

  // lowest speed / highest difficulty is when the speed variable is 3 - can be lowered for even faster play
  if(speed < 3){
    speed = 3;
  }
  // clear level and add happy winner face :)
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
        level[i][j] = faces.happyFace[i][j];
    }   
  }
}
void Breakout::lost(){
  // clear level and add sad loser face :(
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 8; j++){
        level[i][j] = faces.sadFace[i][j];
    }   
  }
}

void Breakout::play(int controls){
  moveBall();
  checkCollision();
  level[7-ball.y][ball.x] = 'O';
  movePaddle(controls);
  
}

void Breakout::movePaddle(int controls){
  switch(controls){
    case LEFT:
      paddle.moveLeft();
      break;
    case RIGHT:
      paddle.moveRight();
      break;
  }
  updatePaddle();

}
void Breakout::checkCollision(){
  // if a brick is hit
  if(level[7-ball.y][ball.x] == 'B'){
    bricks--;
  }
  
  else if(ball.y <= 1){
    
    // if ball is in paddle, move above
    if(level[7-ball.y][ball.x] == 'P'){
      ball.y++;
    }
    
    // if the paddle is hit
    if(level[7-ball.y+1][ball.x] == 'P'){
      hitPaddle();
    }
    checkSides();
  }
}

void Breakout::hitPaddle(){
      // hit on middle
      if(ball.x == paddle.pos+paddle.size/2){
        ball.bounceUp();
      }
      
      // hit on ends from up bounce
      else if(ball.dirX == 0){
        hitEnd();
      }
      ball.up();}
void Breakout::hitEnd(){
  // left end
  if(ball.x < paddle.pos+paddle.size/2){
    ball.bounceLeft();
  }
        
  // right end
  else if(ball.x > paddle.pos+paddle.size/2){
    ball.bounceRight();
  }
}

void Breakout::checkSides(){
  if(ball.y == 1){
      
    // hit on left side
    if(ball.dirX == 1 && ball.x == paddle.pos-1){
      ball.bounceBackLeft();
    }
      
    // hit on right side
    else if(ball.dirX == -1 && ball.x == paddle.pos + paddle.size){
      ball.bounceBackRight();
    }
  }
}


void Breakout::moveBall(){
  level[7-ball.y][ball.x] = 'E';
  ball.move();
}


