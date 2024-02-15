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

#include "Ball.h"

void Ball::move(){

  // update y position
  if(dirY > 0){
    y++;
    if(y > 7){
      y = 6;
      down();
    }
  }
  else if(dirY < 0){
    y--;
    if(y < 0){
      y = 1;
      up(); 
    }
  }

  // update x position
  if(dirX > 0){
    x++;
    if(x > 7){
      x = 6;
      bounceLeft();  
    }
  }
  else if(dirX < 0){
    x--;
    if(x < 0){
      x = 1;
      bounceRight(); 
    }  
  }
}

void Ball::bounceBackLeft(){
  dirX = -1;
  dirY = 1;
  x++;
}

void Ball::bounceBackRight(){
  dirX = 1;
  dirY = 1;
  x--;
}

void Ball::bounceUp(){
  dirX = 0;
  dirY = 1;
}

void Ball::bounceLeft(){
  dirX = -1;
}
void Ball::bounceRight(){
  dirX = 1;
}

void Ball::up(){
  dirY = 1;
}

void Ball::down(){
  dirY = -1;
}

