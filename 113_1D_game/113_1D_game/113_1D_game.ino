/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <FastLED.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// ------------------------------------------------------------
// PIN MAP
// ------------------------------------------------------------
#define LED_STRIP_PIN 7
#define NUM_LEDS 60

#define TFT_CS 0
#define TFT_DC 1
#define TFT_RST 3
#define SCREEN_W 128
#define SCREEN_H 160

#define BTN_LEFT 9
#define BTN_RIGHT 20
#define BTN_SELECT 21
#define BTN_RESET 10

#define BUZZER_PIN 8
#define BRIGHTNESS 80

// ------------------------------------------------------------
// GLOBAL OBJECTS
// ------------------------------------------------------------
CRGB leds[NUM_LEDS];
Adafruit_ST7735 display(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas(SCREEN_W, SCREEN_H);
bool tftDirty = false;

// ------------------------------------------------------------
// BUTTON HANDLING
// ------------------------------------------------------------
struct Button {
  uint8_t pin;
  bool lastRaw = true;
  bool stable = true;
  unsigned long lastChange = 0;
  bool pressedEvent = false;
};

Button btnLeft, btnRight, btnSelect, btnReset;

void initButton(Button &b, uint8_t pin) {
  b.pin = pin;
  pinMode(pin, INPUT_PULLUP);
}

void updateButton(Button &b) {
  bool raw = digitalRead(b.pin);
  if (raw != b.lastRaw) {
    b.lastChange = millis();
    b.lastRaw = raw;
  }
  if ((millis() - b.lastChange) > 25 && raw != b.stable) {
    b.stable = raw;
    if (b.stable == LOW) b.pressedEvent = true;
  }
}

bool wasPressed(Button &b) {
  if (b.pressedEvent) {
    b.pressedEvent = false;
    return true;
  }
  return false;
}

bool isHeld(Button &b) {
  return b.stable == LOW;
}

void updateAllButtons() {
  updateButton(btnLeft);
  updateButton(btnRight);
  updateButton(btnSelect);
  updateButton(btnReset);
}

// ------------------------------------------------------------
// BUZZER
// ------------------------------------------------------------
struct BuzzNote {
  uint16_t freq;
  uint16_t durMs;
};
const int BUZZ_MAX_QUEUE = 6;
BuzzNote buzzQueue[BUZZ_MAX_QUEUE];
int buzzQueueLen = 0;
int buzzQueueIndex = 0;
unsigned long buzzNoteEndAt = 0;
bool buzzNotePlaying = false;

void setupBuzzer() {
  ledcAttach(BUZZER_PIN, 2000, 8);
  ledcWriteTone(BUZZER_PIN, 0);
}

void queueNotes(const uint16_t *freqs, const uint16_t *durs, int count) {
  int n = min(count, BUZZ_MAX_QUEUE);
  for (int i = 0; i < n; i++) {
    buzzQueue[i].freq = freqs[i];
    buzzQueue[i].durMs = durs[i];
  }
  buzzQueueLen = n;
  buzzQueueIndex = 0;
  buzzNotePlaying = false;
}

void beep(uint16_t freq, uint16_t durMs) {
  uint16_t f[1] = { freq };
  uint16_t d[1] = { durMs };
  queueNotes(f, d, 1);
}

void updateBuzzer() {
  unsigned long now = millis();
  if (buzzNotePlaying && now >= buzzNoteEndAt) {
    ledcWriteTone(BUZZER_PIN, 0);
    buzzNotePlaying = false;
    buzzQueueIndex++;
  }
  if (!buzzNotePlaying && buzzQueueIndex < buzzQueueLen) {
    BuzzNote &n = buzzQueue[buzzQueueIndex];
    ledcWriteTone(BUZZER_PIN, n.freq);
    buzzNoteEndAt = now + n.durMs;
    buzzNotePlaying = true;
  }
}

void beepGameOver() {
  static const uint16_t freqs[] = { 320, 220 };
  static const uint16_t durs[] = { 140, 220 };
  queueNotes(freqs, durs, 2);
}

void beepLaunch() {
  static const uint16_t freqs[] = { 600, 900 };
  static const uint16_t durs[] = { 60, 90 };
  queueNotes(freqs, durs, 2);
}

// ------------------------------------------------------------
// GAME STATE MACHINE
// ------------------------------------------------------------
enum GameState {
  STATE_MENU,
  STATE_RUNNER,
  STATE_PONG,
  STATE_INVADERS,
  STATE_REACTION,
  STATE_MEMORY,
  STATE_TUG,
  STATE_GAME_OVER
};

GameState currentState = STATE_MENU;
int lastScore = 0;
String lastGameName = "";

const char *GAME_NAMES[] = {
  "Invaders", "Runner", "Pong", "Reaction", "Memory", "Tug of War"
};
const int NUM_GAMES = 6;
int menuIndex = 0;

void drawMenu();
void loopMenu();
void launchGame(int idx);
void drawGameOver();
void loopGameOver();
void goToMenu();
void goToGameOver(String gameName, int score);

void initRunner();
void loopRunner();
void initPong();
void loopPong();
void initInvaders();
void loopInvaders();
void initReaction();
void loopReaction();
void initMemory();
void loopMemory();
void initTug();
void loopTug();

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, LED_STRIP_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);

  display.initR(INITR_BLACKTAB);
  display.setRotation(0);
  display.fillScreen(ST77XX_BLACK);
  display.setTextWrap(false);

  initButton(btnLeft, BTN_LEFT);
  initButton(btnRight, BTN_RIGHT);
  initButton(btnSelect, BTN_SELECT);
  initButton(btnReset, BTN_RESET);

  setupBuzzer();
  randomSeed(esp_random());

  drawMenu();
}

void loop() {
  updateAllButtons();
  updateBuzzer();

  // FIX 1: Reset button triggers immediately instead of holding
  if (wasPressed(btnReset) && currentState != STATE_MENU) {
    goToMenu();
  }

  switch (currentState) {
    case STATE_MENU: loopMenu(); break;
    case STATE_RUNNER: loopRunner(); break;
    case STATE_PONG: loopPong(); break;
    case STATE_INVADERS: loopInvaders(); break;
    case STATE_REACTION: loopReaction(); break;
    case STATE_MEMORY: loopMemory(); break;
    case STATE_TUG: loopTug(); break;
    case STATE_GAME_OVER: loopGameOver(); break;
  }

  // FIX 2: Push canvas sprite to TFT only when dirtied, preventing flicker
  if (tftDirty) {
    display.drawRGBBitmap(0, 0, canvas.getBuffer(), SCREEN_W, SCREEN_H);
    tftDirty = false;
  }

  FastLED.show();
}

void goToMenu() {
  currentState = STATE_MENU;
  FastLED.clear(true);
  drawMenu();
}

void goToGameOver(String gameName, int score) {
  lastGameName = gameName;
  lastScore = score;
  currentState = STATE_GAME_OVER;
  beepGameOver();
  drawGameOver();
}

// ------------------------------------------------------------
// MENU
// ------------------------------------------------------------
void drawMenu() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, ST77XX_CYAN);
  canvas.setTextColor(ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(4, 2);
  canvas.print("1D Games");

  canvas.setTextSize(1);
  for (int i = 0; i < NUM_GAMES; i++) {
    int y = 26 + i * 13;
    if (i == menuIndex) {
      canvas.fillRect(0, y - 2, SCREEN_W, 12, ST77XX_WHITE);
      canvas.setTextColor(ST77XX_BLACK);
    } else {
      canvas.setTextColor(ST77XX_WHITE);
    }
    canvas.setCursor(6, y);
    canvas.print(GAME_NAMES[i]);
  }
  tftDirty = true;
}

void loopMenu() {
  if (wasPressed(btnLeft)) {
    menuIndex = (menuIndex - 1 + NUM_GAMES) % NUM_GAMES;
    beep(500, 30);
    drawMenu();
  }
  if (wasPressed(btnRight)) {
    menuIndex = (menuIndex + 1) % NUM_GAMES;
    beep(500, 30);
    drawMenu();
  }
  if (wasPressed(btnSelect)) {
    beepLaunch();
    launchGame(menuIndex);
  }
}

void launchGame(int idx) {
  FastLED.clear(true);
  switch (idx) {
    case 0:
      initInvaders();
      currentState = STATE_INVADERS;
      break;
    case 1:
      initRunner();
      currentState = STATE_RUNNER;
      break;
    case 2:
      initPong();
      currentState = STATE_PONG;
      break;
    case 3:
      initReaction();
      currentState = STATE_REACTION;
      break;
    case 4:
      initMemory();
      currentState = STATE_MEMORY;
      break;
    case 5:
      initTug();
      currentState = STATE_TUG;
      break;
  }
}

void drawGameOver() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, ST77XX_RED);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(4, 6);
  canvas.print(lastGameName);

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(2);
  canvas.setCursor(14, 32);
  canvas.print("GAME OVER");

  canvas.setTextSize(1);
  canvas.setCursor(14, 62);
  canvas.print("SCORE");
  canvas.setTextSize(3);
  canvas.setCursor(14, 74);
  canvas.print(lastScore);

  canvas.setTextSize(1);
  canvas.setCursor(14, 112);
  canvas.print("SELECT = menu");

  tftDirty = true;

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(60, 0, 0);
}

void loopGameOver() {
  if (wasPressed(btnSelect) || wasPressed(btnLeft) || wasPressed(btnRight)) {
    goToMenu();
  }
}

void showScoreTFT(const char *title, uint16_t accent, int score) {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, accent);
  canvas.setTextColor(ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(4, 2);
  canvas.print(title);

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(4, 34);
  canvas.print("SCORE");
  canvas.setTextSize(4);
  canvas.setCursor(4, 46);
  canvas.print(score);

  tftDirty = true;
}

// ============================================================
// GAME 1: RUNNER
// ============================================================
namespace RunnerGame {
const int MAX_OBSTACLES = 4;
float obstaclePos[MAX_OBSTACLES];
bool obstacleAlive[MAX_OBSTACLES];
int obstacleSize[MAX_OBSTACLES];
float obstacleSpeed;
bool jumping;
unsigned long jumpStart;
int score;
unsigned long lastTick;
unsigned long nextSpawnTime;
}

void initRunner() {
  using namespace RunnerGame;
  for (int i = 0; i < MAX_OBSTACLES; i++) obstacleAlive[i] = false;
  obstacleAlive[0] = true;
  obstaclePos[0] = NUM_LEDS / 2.0;
  obstacleSize[0] = random(1, 4);
  obstacleSpeed = 8.0;
  jumping = false;
  score = 0;
  lastTick = millis();
  nextSpawnTime = millis() + 1500;
  showScoreTFT("RUNNER", ST77XX_GREEN, score);
}

void loopRunner() {
  using namespace RunnerGame;

  if (wasPressed(btnSelect) && !jumping) {
    jumping = true;
    jumpStart = millis();
    beep(750, 50);
  }
  if (jumping) {
    unsigned long elapsed = millis() - jumpStart;
    if (elapsed > 200 && !isHeld(btnSelect)) jumping = false;
    else if (elapsed > 600) jumping = false;
  }

  unsigned long now = millis();
  float dt = (now - lastTick) / 1000.0;
  lastTick = now;

  if (now > nextSpawnTime) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
      if (!obstacleAlive[i]) {
        obstacleAlive[i] = true;
        obstaclePos[i] = NUM_LEDS - 1;
        obstacleSize[i] = random(1, 4);
        break;
      }
    }
    nextSpawnTime = now + max(600, 2000 - score * 50) + random(0, 1000);
    obstacleSpeed = min(obstacleSpeed + 0.4f, 25.0f);
  }

  const int PLAYER_POS = 5;

  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacleAlive[i]) {
      obstaclePos[i] -= obstacleSpeed * dt;

      float oStart = obstaclePos[i];
      float oEnd = oStart + obstacleSize[i];

      if (oStart <= PLAYER_POS && oEnd > PLAYER_POS) {
        if (!jumping) {
          goToGameOver("Runner", score);
          return;
        }
      }

      if (oEnd < 0) {
        score++;
        beep(1000, 40);
        obstacleAlive[i] = false;
        showScoreTFT("RUNNER", ST77XX_GREEN, score);
      }
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[PLAYER_POS] = jumping ? CRGB(0, 255, 80) : CRGB(0, 120, 0);
  for (int i = 0; i < MAX_OBSTACLES; i++) {
    if (obstacleAlive[i]) {
      for (int j = 0; j < obstacleSize[i]; j++) {
        int p = (int)obstaclePos[i] + j;
        if (p >= 0 && p < NUM_LEDS && (p != PLAYER_POS || !jumping)) leds[p] = CRGB::Red;
      }
    }
  }
}

// ============================================================
// GAME 2: PONG
// ============================================================
namespace PongGame {
float ballPos;
float ballVel;
int scoreA, scoreB;
unsigned long lastTick;
const int ZONE = 4;
}

void drawPongTFT() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, ST77XX_BLUE);
  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(2);
  canvas.setCursor(4, 2);
  canvas.print("PONG");

  canvas.setTextColor(ST77XX_CYAN);
  canvas.setTextSize(5);
  canvas.setCursor(30, 60);
  canvas.print(PongGame::scoreA);

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(2);
  canvas.setCursor(76, 70);
  canvas.print("-");

  canvas.setTextColor(ST77XX_RED);
  canvas.setTextSize(5);
  canvas.setCursor(100, 60);
  canvas.print(PongGame::scoreB);

  tftDirty = true;
}

void initPong() {
  using namespace PongGame;
  ballPos = NUM_LEDS / 2.0;
  ballVel = 14.0;
  scoreA = 0;
  scoreB = 0;
  lastTick = millis();
  drawPongTFT();
}

void loopPong() {
  using namespace PongGame;

  unsigned long now = millis();
  float dt = (now - lastTick) / 1000.0;
  lastTick = now;
  ballPos += ballVel * dt;

  bool inZoneA = ballPos <= ZONE;
  bool inZoneB = ballPos >= NUM_LEDS - 1 - ZONE;

  if (inZoneA) {
    if (wasPressed(btnLeft)) {
      ballVel = fabs(ballVel) + 0.6;
      beep(700, 40);
    } else if (ballPos <= 0) {
      scoreB++;
      beep(220, 150);
      drawPongTFT();
      if (scoreB >= 5) {
        goToGameOver("Pong (P2 wins)", scoreB);
        return;
      }
      ballPos = NUM_LEDS / 2.0;
      ballVel = 14.0;
    }
  }
  if (inZoneB) {
    if (wasPressed(btnRight)) {
      ballVel = -(fabs(ballVel) + 0.6);
      beep(700, 40);
    } else if (ballPos >= NUM_LEDS - 1) {
      scoreA++;
      beep(220, 150);
      drawPongTFT();
      if (scoreA >= 5) {
        goToGameOver("Pong (P1 wins)", scoreA);
        return;
      }
      ballPos = NUM_LEDS / 2.0;
      ballVel = -14.0;
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int i = 0; i < ZONE; i++) leds[i] = CRGB(0, 0, 40);
  for (int i = NUM_LEDS - ZONE; i < NUM_LEDS; i++) leds[i] = CRGB(40, 0, 0);
  int bp = constrain((int)ballPos, 0, NUM_LEDS - 1);
  leds[bp] = CRGB::White;
}

// ============================================================
// GAME 3: SPACE INVADERS
// ============================================================
namespace InvadersGame {
const int MAX_ENEMIES = 6;
float enemyPos[MAX_ENEMIES];
bool enemyAlive[MAX_ENEMIES];
uint8_t enemyColor[MAX_ENEMIES];
uint8_t enemySize[MAX_ENEMIES];
float enemySpeed;

const int MAX_SHOTS = 10;
float shotsPos[MAX_SHOTS];
uint8_t shotsColor[MAX_SHOTS];
bool shotsActive[MAX_SHOTS];

int score;
int lives;
unsigned long lastTick;
unsigned long nextSpawnTime;
}

void drawInvadersTFT() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, ST77XX_CYAN);
  canvas.setTextColor(ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(4, 2);
  canvas.print("INVADERS");

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(6, 32);
  canvas.print("LIVES");
  canvas.setTextSize(3);
  canvas.setCursor(6, 44);
  canvas.print(InvadersGame::lives);

  canvas.setTextSize(1);
  canvas.setCursor(90, 32);
  canvas.print("SCORE");
  canvas.setTextSize(3);
  canvas.setCursor(90, 44);
  canvas.print(InvadersGame::score);

  tftDirty = true;
}

void spawnEnemyInvaders() {
  using namespace InvadersGame;
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!enemyAlive[i]) {
      enemyAlive[i] = true;
      enemyPos[i] = NUM_LEDS - 1 - random(0, 5);
      enemyColor[i] = random(0, 3);
      enemySize[i] = random(1, 4);
      return;
    }
  }
}

void initInvaders() {
  using namespace InvadersGame;
  for (int i = 0; i < MAX_ENEMIES; i++) enemyAlive[i] = false;
  for (int i = 0; i < MAX_SHOTS; i++) shotsActive[i] = false;
  enemySpeed = 4.0;
  score = 0;
  lives = 3;
  lastTick = millis();
  nextSpawnTime = millis() + 1000 + random(0, 1000);
  spawnEnemyInvaders();
  drawInvadersTFT();
}

void loopInvaders() {
  using namespace InvadersGame;

  int pressedColor = -1;
  if (wasPressed(btnLeft)) pressedColor = 0;
  else if (wasPressed(btnRight)) pressedColor = 1;
  else if (wasPressed(btnSelect)) pressedColor = 2;

  if (pressedColor != -1) {
    for (int i = 0; i < MAX_SHOTS; i++) {
      if (!shotsActive[i]) {
        shotsActive[i] = true;
        shotsPos[i] = 1;
        shotsColor[i] = pressedColor;
        beep(1200, 25);
        break;
      }
    }
  }

  unsigned long now = millis();
  float dt = (now - lastTick) / 1000.0;
  lastTick = now;

  if (now > nextSpawnTime) {
    spawnEnemyInvaders();
    nextSpawnTime = now + max(500, 2000 - score * 50) + random(0, 1000);
    enemySpeed = min(enemySpeed + 0.3f, 14.0f);
  }

  for (int s = 0; s < MAX_SHOTS; s++) {
    if (shotsActive[s]) {
      shotsPos[s] += 40.0 * dt;
      int hitIdx = -1;
      float closest = NUM_LEDS;
      for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemyAlive[i] && shotsPos[s] >= enemyPos[i]) {
          if (enemyPos[i] < closest) {
            closest = enemyPos[i];
            hitIdx = i;
          }
        }
      }

      if (hitIdx != -1) {
        int i = hitIdx;
        if (shotsColor[s] == enemyColor[i]) {
          enemySize[i]--;
          enemyPos[i] += 1.0;
          score++;
          beep(950, 60);
          if (enemySize[i] <= 0) enemyAlive[i] = false;
        } else {
          lives--;
          beep(260, 180);
          drawInvadersTFT();
          if (lives <= 0) {
            shotsActive[s] = false;
            goToGameOver("Invaders", score);
            return;
          }
        }
        shotsActive[s] = false;
        drawInvadersTFT();
      }
      if (shotsPos[s] >= NUM_LEDS) shotsActive[s] = false;
    }
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemyAlive[i]) {
      enemyPos[i] -= enemySpeed * dt;
      if (enemyPos[i] <= 0) {
        enemyAlive[i] = false;
        lives--;
        beep(260, 180);
        drawInvadersTFT();
        if (lives <= 0) {
          goToGameOver("Invaders", score);
          return;
        }
      }
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[0] = CRGB::White;
  for (int s = 0; s < MAX_SHOTS; s++) {
    if (shotsActive[s]) {
      int sp = constrain((int)shotsPos[s], 0, NUM_LEDS - 1);
      leds[sp] = (shotsColor[s] == 0) ? CRGB::Red : (shotsColor[s] == 1) ? CRGB::Green
                                                                         : CRGB::Blue;
    }
  }
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (enemyAlive[i]) {
      CRGB c = (enemyColor[i] == 0) ? CRGB::Red : (enemyColor[i] == 1) ? CRGB::Green
                                                                       : CRGB::Blue;
      for (int j = 0; j < enemySize[i]; j++) {
        int p = (int)enemyPos[i] + j;
        if (p >= 0 && p < NUM_LEDS) leds[p] = c;
      }
    }
  }
}

// ============================================================
// GAME 4: REACTION
// ============================================================
namespace ReactionGame {
float pos;
float vel;
int targetStart;
int score;
int misses;
unsigned long lastTick;
bool judged;
bool wasInZone;
}

void drawReactionTFT() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, ST77XX_YELLOW);
  canvas.setTextColor(ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(4, 2);
  canvas.print("REACTION");

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(6, 32);
  canvas.print("MISSES");
  canvas.setTextSize(3);
  canvas.setCursor(6, 44);
  canvas.print(ReactionGame::misses);
  canvas.setTextSize(1);
  canvas.print("/3");

  canvas.setTextSize(1);
  canvas.setCursor(90, 32);
  canvas.print("SCORE");
  canvas.setTextSize(3);
  canvas.setCursor(90, 44);
  canvas.print(ReactionGame::score);

  tftDirty = true;
}

void newTargetReaction() {
  ReactionGame::targetStart = random(5, NUM_LEDS - 8);
  ReactionGame::judged = false;
  ReactionGame::wasInZone = false;
}

void initReaction() {
  using namespace ReactionGame;
  pos = 0;
  vel = 20.0;
  score = 0;
  misses = 0;
  lastTick = millis();
  newTargetReaction();
  drawReactionTFT();
}

void loopReaction() {
  using namespace ReactionGame;

  unsigned long now = millis();
  float dt = (now - lastTick) / 1000.0;
  lastTick = now;

  pos += vel * dt;
  if (pos <= 0 || pos >= NUM_LEDS - 1) {
    vel = -vel;
    pos = constrain(pos, 0.0f, (float)(NUM_LEDS - 1));
  }

  bool inZone = ((int)pos >= targetStart && (int)pos < targetStart + 3);

  if (wasPressed(btnSelect)) {
    if (inZone && !judged) {
      score++;
      judged = true;
      beep(1000, 50);
      vel += (vel > 0 ? 2.0 : -2.0);
      newTargetReaction();
      drawReactionTFT();
      inZone = false;
    } else if (!inZone) {
      misses++;
      beep(200, 150);
      drawReactionTFT();
      if (misses >= 3) {
        goToGameOver("Reaction", score);
        return;
      }
    }
  }

  if (ReactionGame::wasInZone && !inZone && !judged) {
    misses++;
    judged = true;
    beep(200, 150);
    drawReactionTFT();
    if (misses >= 3) {
      goToGameOver("Reaction", score);
      return;
    }
    newTargetReaction();
  }
  ReactionGame::wasInZone = inZone;

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int i = 0; i < 3; i++) leds[targetStart + i] = CRGB(0, 0, 60);
  int lp = constrain((int)pos, 0, NUM_LEDS - 1);
  leds[lp] = CRGB::White;
}

// ============================================================
// GAME 5: MEMORY
// ============================================================
namespace MemoryGame {
const int MAX_SEQ = 60;
int sequence[MAX_SEQ];
int seqLen;
int playerStep;
bool showingSequence;
int showIndex;
unsigned long showTimer;
bool litNow;
int score;
const int ZONE_SIZE = NUM_LEDS / 3;
}

CRGB memoryColor(int idx) {
  return idx == 0 ? CRGB(80, 0, 0) : idx == 1 ? CRGB(0, 80, 0)
                                              : CRGB(0, 0, 80);
}
CRGB memoryColorDim(int idx) {
  return idx == 0 ? CRGB(10, 0, 0) : idx == 1 ? CRGB(0, 10, 0)
                                              : CRGB(0, 0, 10);
}
uint16_t memoryFreq(int idx) {
  return idx == 0 ? 440 : idx == 1 ? 554
                                   : 659;
}

void drawMemoryZones(int litZone) {
  for (int z = 0; z < 3; z++) {
    CRGB c = (z == litZone) ? memoryColor(z) : memoryColorDim(z);
    int start = z * MemoryGame::ZONE_SIZE;
    int end = (z == 2) ? NUM_LEDS : start + MemoryGame::ZONE_SIZE;
    for (int i = start; i < end; i++) leds[i] = c;
  }
}

void startShowingMemory() {
  MemoryGame::showingSequence = true;
  MemoryGame::showIndex = 0;
  MemoryGame::litNow = false;
  MemoryGame::showTimer = millis();
}

void initMemory() {
  using namespace MemoryGame;
  seqLen = 0;
  sequence[seqLen++] = random(0, 3);
  playerStep = 0;
  score = 0;
  startShowingMemory();
  showScoreTFT("MEMORY", ST77XX_MAGENTA, score);
}

void loopMemory() {
  using namespace MemoryGame;

  if (showingSequence) {
    unsigned long now = millis();
    unsigned long onTime = 400, gapTime = 200;
    if (!litNow && now - showTimer > gapTime) {
      if (showIndex >= seqLen) showingSequence = false;
      else {
        litNow = true;
        showTimer = now;
        beep(memoryFreq(sequence[showIndex]), 350);
      }
    } else if (litNow && now - showTimer > onTime) {
      litNow = false;
      showTimer = now;
      showIndex++;
    }
    drawMemoryZones(litNow ? sequence[showIndex < seqLen ? showIndex : seqLen - 1] : -1);
  } else {
    int pressedBtn = -1;
    if (wasPressed(btnLeft)) pressedBtn = 0;
    else if (wasPressed(btnRight)) pressedBtn = 1;
    else if (wasPressed(btnSelect)) pressedBtn = 2;

    int heldBtn = -1;
    if (isHeld(btnLeft)) heldBtn = 0;
    else if (isHeld(btnRight)) heldBtn = 1;
    else if (isHeld(btnSelect)) heldBtn = 2;

    if (pressedBtn >= 0) {
      beep(memoryFreq(pressedBtn), 150);
      if (pressedBtn == sequence[playerStep]) {
        playerStep++;
        if (playerStep >= seqLen) {
          score++;
          if (seqLen < MAX_SEQ) sequence[seqLen++] = random(0, 3);
          playerStep = 0;
          startShowingMemory();
          showScoreTFT("MEMORY", ST77XX_MAGENTA, score);
          return;
        }
      } else {
        goToGameOver("Memory", score);
        return;
      }
    }
    drawMemoryZones(heldBtn);
  }
}

// ============================================================
// GAME 6: TUG OF WAR
// ============================================================
namespace TugGame {
float markerPos;
unsigned long lastTick;
}

void drawTugTFT() {
  canvas.fillScreen(ST77XX_BLACK);
  canvas.fillRect(0, 0, SCREEN_W, 20, ST77XX_ORANGE);
  canvas.setTextColor(ST77XX_BLACK);
  canvas.setTextSize(2);
  canvas.setCursor(4, 2);
  canvas.print("TUG OF WAR");

  canvas.setTextColor(ST77XX_WHITE);
  canvas.setTextSize(1);
  canvas.setCursor(14, 40);
  canvas.print("MASH LEFT / RIGHT!");

  canvas.setTextColor(ST77XX_CYAN);
  canvas.setCursor(14, 60);
  canvas.print("P1 (LEFT) pushes ->");
  canvas.setTextColor(ST77XX_RED);
  canvas.setCursor(14, 74);
  canvas.print("<- P2 (RIGHT) pushes");

  tftDirty = true;
}

void initTug() {
  TugGame::markerPos = NUM_LEDS / 2.0;
  TugGame::lastTick = millis();
  drawTugTFT();
}

void loopTug() {
  using namespace TugGame;

  unsigned long now = millis();
  float dt = (now - lastTick) / 1000.0;
  lastTick = now;

  if (wasPressed(btnLeft)) {
    markerPos += 3.0;
    beep(600, 20);
  }
  if (wasPressed(btnRight)) {
    markerPos -= 3.0;
    beep(600, 20);
  }

  float center = NUM_LEDS / 2.0;
  markerPos += (center - markerPos) * 0.15 * dt;
  markerPos = constrain(markerPos, 0.0f, (float)(NUM_LEDS - 1));

  if (markerPos >= NUM_LEDS - 1) {
    goToGameOver("Tug of War (P1 wins)", 1);
    return;
  }
  if (markerPos <= 0) {
    goToGameOver("Tug of War (P2 wins)", 1);
    return;
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < markerPos) leds[i] = CRGB(0, 0, 25);
    else leds[i] = CRGB(25, 0, 0);
  }
  int mp = (int)markerPos;
  leds[mp] = CRGB::White;
}