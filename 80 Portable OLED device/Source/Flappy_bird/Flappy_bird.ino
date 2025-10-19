/* -------------------------------------------------
 Arduino project by Tech Talkies YouTube Channel.
 https://www.youtube.com/@techtalkies1
 -------------------------------------------------*/

#include <Wire.h>
#include <U8g2lib.h>  // https://github.com/olikraus/u8g2 (2.35.30)

// display
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// pins
#define BUTTON_PIN D1  // active-low button

// screen
const int SCREEN_W = 128;
const int SCREEN_H = 64;

// bird
const int BIRD_X = 30;
float birdY;
float birdV;                // vertical velocity (pixels per tick)
const float GRAVITY = 0.3;  // gravity per tick
const float FLAP_V = -3.0;  // flap impulse

// pipes
struct Pipe {
  int x;
  int gapY;
  bool passed;  // for scoring
};
const int PIPE_W = 16;
const int GAP_H = 30;      // gap height in pixels
const int PIPE_SPEED = 1;  // pixels per frame
const int MAX_PIPES = 5;
Pipe pipes[MAX_PIPES];
int pipeInterval = 70;  // horizontal spacing in pixels (approx)
int framesSinceLastPipe = 0;

// game
int gameState = 0;  // 0=start, 1=playing, 2=gameover
int score = 0;
int highScore = 0;

// timing
const int TICK_MS = 20;  // 50 Hz
uint32_t lastTick = 0;

// button debounce
uint32_t lastBtnMs = 0;
bool lastBtnState = HIGH;

// 'bird1', 20x20px
const unsigned char bird1[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 
	0x0c, 0x00, 0xc0, 0x1f, 0x00, 0xc0, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0x80, 0xff, 0x00, 0x80, 0xff, 
	0x00, 0xf8, 0xff, 0x01, 0xf8, 0xff, 0x01, 0xf8, 0xff, 0x01, 0xf0, 0x7f, 0x00, 0x70, 0x1f, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void resetPipes() {
  for (int i = 0; i < MAX_PIPES; i++) {
    pipes[i].x = SCREEN_W + i * (PIPE_W + pipeInterval);
    pipes[i].gapY = random(8, SCREEN_H - GAP_H - 8);
    pipes[i].passed = false;
  }
  framesSinceLastPipe = 0;
}

void setupGame() {
  birdY = SCREEN_H / 2;
  birdV = 0;
  score = 0;
  resetPipes();
}

void drawStartScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(35, 12, "Flappy Xiao");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(25, 28, "Press to start");
  // draw bird
  u8g2.drawXBMP(52, 42, 20, 20, bird1);
  u8g2.sendBuffer();
}

void drawGame() {
  u8g2.clearBuffer();

  // draw pipes
  for (int i = 0; i < MAX_PIPES; i++) {
    int x = pipes[i].x;
    int gapY = pipes[i].gapY;
    // top pipe
    u8g2.drawBox(x, 0, PIPE_W, gapY);
    // bottom pipe
    u8g2.drawBox(x, gapY + GAP_H, PIPE_W, SCREEN_H - (gapY + GAP_H));
  }

  // draw ground line (optional)
  // u8g2.drawHLine(0, SCREEN_H-1, SCREEN_W);

  // draw bird (small circle)
  u8g2.drawDisc(BIRD_X, (int)birdY, 3, U8G2_DRAW_ALL);

  // draw score top-left
  char buf[16];
  snprintf(buf, sizeof(buf), "Score %d", score);
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(2, 10, buf);

  u8g2.sendBuffer();
}

void drawGameOver() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(18, 22, "Game Over");
  u8g2.setFont(u8g2_font_6x12_tr);
  char buf[32];
  snprintf(buf, sizeof(buf), "Score: %d  high: %d", score, highScore);
  u8g2.drawStr(8, 40, buf);
  u8g2.drawStr(8, 54, "Press button");
  u8g2.sendBuffer();
}

bool checkCollision() {
  // ground / ceiling
  if (birdY - 3 <= 0) return true;
  if (birdY + 3 >= SCREEN_H) return true;

  // pipes
  for (int i = 0; i < MAX_PIPES; i++) {
    int x = pipes[i].x;
    int gapY = pipes[i].gapY;
    // simple AABB vs circle approximation
    int bx = BIRD_X;
    int by = (int)birdY;
    if (bx + 3 >= x && bx - 3 <= x + PIPE_W) {
      if (by - 3 < gapY || by + 3 > gapY + GAP_H) {
        return true;
      }
    }
  }
  return false;
}

void advancePipes() {
  for (int i = 0; i < MAX_PIPES; i++) {
    pipes[i].x -= PIPE_SPEED;
    // recycle pipe when off screen
    if (pipes[i].x + PIPE_W < 0) {
      // place it to the right after the rightmost pipe
      int rightmost = 0;
      for (int j = 0; j < MAX_PIPES; j++)
        if (pipes[j].x > pipes[rightmost].x) rightmost = j;
      pipes[i].x = pipes[rightmost].x + PIPE_W + pipeInterval;
      pipes[i].gapY = random(8, SCREEN_H - GAP_H - 8);
      pipes[i].passed = false;
    }
    // score when pipe passes bird
    if (!pipes[i].passed && pipes[i].x + PIPE_W < BIRD_X - 3) {
      pipes[i].passed = true;
      score++;
      if (score > highScore) highScore = score;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();

  randomSeed(micros());
  drawStartScreen();
  lastTick = millis();
}

bool readButtonPressed() {
  bool pressed = (digitalRead(BUTTON_PIN) == LOW);
  // simple debounce on transition
  if (pressed != (lastBtnState == LOW)) {
    // changed state
    if (millis() - lastBtnMs > 40) {  // debounce
      lastBtnMs = millis();
      lastBtnState = pressed ? LOW : HIGH;
      if (pressed) return true;
    }
  }
  return false;
}

void loop() {
  // tick timing
  uint32_t now = millis();
  if (now - lastTick < TICK_MS) return;
  lastTick = now;

  // handle states
  if (gameState == 0) {
    // start screen - wait for button press to start
    if (readButtonPressed()) {
      setupGame();
      gameState = 1;
      drawGame();  // initial frame
    }
    return;
  }

  if (gameState == 1) {
    // gameplay
    // flap on button press
    if (readButtonPressed()) {
      birdV = FLAP_V;
    }

    // physics
    birdV += GRAVITY;
    birdY += birdV;

    // advance pipes and scoring
    advancePipes();

    // check collision
    if (checkCollision()) {
      gameState = 2;
      drawGameOver();
      return;
    }

    // draw
    drawGame();
    return;
  }

  if (gameState == 2) {
    // game over - wait for button press to return to start
    if (readButtonPressed()) {
      gameState = 0;
      drawStartScreen();
    }
    return;
  }
}