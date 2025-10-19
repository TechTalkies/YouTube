/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Wire.h>
#include <U8g2lib.h>  // https://github.com/olikraus/u8g2 (2.35.30)
#include "LSM6DS3.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ----- display -----
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Create a instance of class LSM6DS3
LSM6DS3 myIMU(I2C_MODE, 0x6A);  //I2C device address 0x6A

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int minVal = 265;
int maxVal = 402;

double x;
double y;
double z;

// ----- Pins -----
#define BUTTON_PIN D1  // press to reset (BOOT button on many ESP32 dev boards)

// ----- Maze grid -----
const int cellSize = 4;                     // keep at 4 for speed/clarity
const int cols = SCREEN_WIDTH / cellSize;   // 32
const int rows = SCREEN_HEIGHT / cellSize;  // 16

// maze[y][x]: 0 = wall, 1 = path
uint8_t maze[rows][cols];
bool visited[rows][cols];

struct Cell {
  int x, y;
};
Cell stack_[rows * cols];
int stackTop = 0;

// movement dirs: up, right, down, left (in cell units)
int dirX[4] = { 0, 1, 0, -1 };
int dirY[4] = { -1, 0, 1, 0 };

// player/goal
int ballX = 0, ballY = 0;  // in cell units on the maze grid
int goalX = 0, goalY = 0;
bool gameWon = false;

// ---------- Stack helpers ----------
inline void push(Cell c) {
  stack_[stackTop++] = c;
}
inline Cell pop() {
  return stack_[--stackTop];
}
inline bool stackEmpty() {
  return stackTop == 0;
}

// ---------- Maze generation (recursive backtracker on a cell grid) ----------
void generateMaze() {
  for (int y = 0; y < rows; y++)
    for (int x = 0; x < cols; x++) {
      maze[y][x] = 0;  // start all as walls
      visited[y][x] = false;
    }

  stackTop = 0;
  Cell start = { 0, 0 };
  push(start);
  visited[0][0] = true;
  maze[0][0] = 1;  // path

  while (!stackEmpty()) {
    Cell current = stack_[stackTop - 1];
    int neighbors[4][2];
    int nCount = 0;

    // look two cells away to create 1-cell-thick walls
    for (int i = 0; i < 4; i++) {
      int nx = current.x + dirX[i] * 2;
      int ny = current.y + dirY[i] * 2;
      if (nx >= 0 && ny >= 0 && nx < cols && ny < rows && !visited[ny][nx]) {
        neighbors[nCount][0] = nx;
        neighbors[nCount][1] = ny;
        nCount++;
      }
    }

    if (nCount > 0) {
      int r = random(nCount);
      int nx = neighbors[r][0];
      int ny = neighbors[r][1];

      // carve wall between current and chosen neighbor
      int wx = current.x + (nx - current.x) / 2;
      int wy = current.y + (ny - current.y) / 2;

      maze[wy][wx] = 1;  // open path
      maze[ny][nx] = 1;
      visited[ny][nx] = true;
      push({ nx, ny });
    } else {
      pop();
    }
  }
}

// ---------- Pick farthest reachable cell from (0,0) as goal (guarantees solvable) ----------
void chooseFarthestGoal() {
  static int16_t dist[rows][cols];
  for (int y = 0; y < rows; y++)
    for (int x = 0; x < cols; x++)
      dist[y][x] = -1;

  // simple BFS
  static int qx[rows * cols];
  static int qy[rows * cols];
  int qh = 0, qt = 0;

  if (maze[0][0] == 0) {
    // safety: ensure start is open
    maze[0][0] = 1;
  }

  qx[qt] = 0;
  qy[qt] = 0;
  qt++;
  dist[0][0] = 0;

  int fx = 0, fy = 0, fdist = 0;

  while (qh < qt) {
    int x = qx[qh], y = qy[qh];
    qh++;
    int d = dist[y][x];

    // track farthest
    if (d > fdist) {
      fdist = d;
      fx = x;
      fy = y;
    }

    for (int i = 0; i < 4; i++) {
      int nx = x + dirX[i];
      int ny = y + dirY[i];
      if (nx >= 0 && ny >= 0 && nx < cols && ny < rows) {
        if (maze[ny][nx] == 1 && dist[ny][nx] == -1) {  // only through open cells
          dist[ny][nx] = d + 1;
          qx[qt] = nx;
          qy[qt] = ny;
          qt++;
        }
      }
    }
  }

  goalX = fx;
  goalY = fy;
}

// ---------- Drawing ----------
void drawMaze() {
  u8g2.clearBuffer();

  // draw walls as white blocks; paths left black
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (maze[y][x] == 0) {
        u8g2.drawBox(x * cellSize, y * cellSize, cellSize, cellSize);
      }
    }
  }

  // draw goal as white ring on black path
  int gx = goalX * cellSize + cellSize / 2;
  int gy = goalY * cellSize + cellSize / 2;
  u8g2.drawCircle(gx, gy, max(1, cellSize / 2), U8G2_DRAW_ALL);

  // draw ball as solid white circle on black path, smaller so it doesn't touch walls
  int bx = ballX * cellSize + cellSize / 2;
  int by = ballY * cellSize + cellSize / 2;
  int radius = max(1, cellSize / 2 - 2);  // shrink radius to keep gap from walls
  u8g2.drawDisc(bx, by, radius, U8G2_DRAW_ALL);

  u8g2.sendBuffer();
}

void winScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(20, 35, "YOU WIN!");
  u8g2.sendBuffer();

  delay(3000);
  resetGame();
}

// ---------- Reset / Setup one round ----------
void resetGame() {
  generateMaze();
  ballX = 0;
  ballY = 0;
  chooseFarthestGoal();  // ensures solvable & interesting
  gameWon = false;
  drawMaze();
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  u8g2.begin();

  if (myIMU.begin() != 0) {
    Serial.println("Device error");
  } else {
    Serial.println("Device OK!");
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(10, 30, "Maze Init...");
  u8g2.sendBuffer();
  delay(1000);

  randomSeed(micros());  // decent enough entropy for maze variability
  resetGame();
}

void loop() {
  // --- button to reset (debounced) ---
  static uint32_t lastBtnMs = 0;
  if (digitalRead(BUTTON_PIN) == LOW && (millis() - lastBtnMs) > 250) {
    lastBtnMs = millis();
    while (digitalRead(BUTTON_PIN) == LOW) { delay(1); }
    resetGame();
    return;
  }

  if (gameWon) return;

  // --- read raw accel ---
  float AcX = myIMU.readRawAccelX();
  float AcY = myIMU.readRawAccelY();
  float AcZ = myIMU.readRawAccelZ();

  // convert to angle
  int xAng = map(AcX, -16000, 16000, -90, 90);
  int yAng = map(AcY, -16000, 16000, -90, 90);
  int zAng = map(AcZ, -16000, 16000, -90, 90);

  float x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  float y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  float z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  // --- movement thresholds ---
  int dx = 0, dy = 0;
  if (x > 10 && x < 200) dx = 1;
  if (x > 250 && x < 350) dx = -1;
  if (y > 10 && y < 200) dy = 1;
  if (y > 250 && y < 350) dy = -1;

  int newX = ballX + dx;
  int newY = ballY + dy;

  if (newX >= 0 && newY >= 0 && newX < cols && newY < rows && maze[newY][newX] == 1) {
    ballX = newX;
    ballY = newY;
    drawMaze();
  }

  if (ballX == goalX && ballY == goalY) {
    gameWon = true;
    winScreen();
  }

  delay(50);
}