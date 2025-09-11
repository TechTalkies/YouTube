/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// sh1106 128x64 i2c
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// maze config
const int cellSize = 4;                          // pixel size of each cell
const int mazeWidth = SCREEN_WIDTH / cellSize;  // number of cells horizontally
const int mazeHeight = SCREEN_HEIGHT / cellSize;  // number of cells vertically

int maze[mazeHeight][mazeWidth];  // 0 = wall, 1 = path
bool visited[mazeHeight][mazeWidth];

// directions: up, right, down, left
int dx[4] = { 0, 1, 0, -1 };
int dy[4] = { -1, 0, 1, 0 };

void drawMazeStep() {
  u8g2.clearBuffer();

  for (int y = 0; y < mazeHeight; y++) {
    for (int x = 0; x < mazeWidth; x++) {
      if (maze[y][x] == 0) {
        // draw wall
        u8g2.drawBox(x * cellSize, y * cellSize, cellSize, cellSize);
      }
    }
  }

  u8g2.sendBuffer();
}

// shuffle directions to randomize path
void shuffle(int arr[], int n) {
  for (int i = n - 1; i > 0; i--) {
    int j = random(i + 1);
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}

// recursive backtracking with step-by-step visualization
void generateMaze(int cx, int cy) {
  visited[cy][cx] = true;
  maze[cy][cx] = 1;

  int dirs[4] = { 0, 1, 2, 3 };
  shuffle(dirs, 4);

  for (int i = 0; i < 4; i++) {
    int dir = dirs[i];
    int nx = cx + dx[dir] * 2;
    int ny = cy + dy[dir] * 2;

    if (nx >= 0 && nx < mazeWidth && ny >= 0 && ny < mazeHeight && !visited[ny][nx]) {
      maze[cy + dy[dir]][cx + dx[dir]] = 1;  // carve wall between
      drawMazeStep();
      delay(100);  // slow down for visual effect

      generateMaze(nx, ny);
    }
  }
}

void setup() {
  randomSeed(analogRead(0));
  u8g2.begin();

  // fill with walls
  for (int y = 0; y < mazeHeight; y++) {
    for (int x = 0; x < mazeWidth; x++) {
      maze[y][x] = 0;
      visited[y][x] = false;
    }
  }

  generateMaze(0, 0);  // start top-left
}

void loop() {
  // do nothing
}
