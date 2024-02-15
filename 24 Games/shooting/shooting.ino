//Original game by Yurii Shevchuk - https://github.com/itdxer/ArduinoGame/tree/master
//Modified by Tech Talkies (https://www.youtube.com/@techtalkies1)

#include <LedControl.h>
#include <Arduino.h>

/* Display PINs */
#define CLK 13
#define CS 10
#define DIN 11

LedControl lc = LedControl(DIN, CLK, CS, 1);

/* Joystick PINs */
#define VRX A0
#define VRY A1

// Messages
int gameOverMessage[10][8] = {
  { 0x00, 0x3c, 0x40, 0x40, 0x48, 0x44, 0x38, 0x00 },  // G
  { 0x00, 0x38, 0x44, 0x7c, 0x44, 0x44, 0x44, 0x00 },  // A
  { 0x00, 0x44, 0x6c, 0x54, 0x44, 0x44, 0x44, 0x00 },  // M
  { 0x00, 0x7c, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00 },  // E
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  //  (space)
  { 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00 },  // O
  { 0x00, 0x44, 0x44, 0x44, 0x44, 0x28, 0x10, 0x00 },  // V
  { 0x00, 0x7c, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00 },  // E
  { 0x00, 0x30, 0x48, 0x48, 0x70, 0x48, 0x44, 0x00 },  // R
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  //  (space)
};

int startNewGameMessage[15][8] = {
  { 0x00, 0x3c, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00 },  // S
  { 0x00, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 },  // T
  { 0x00, 0x38, 0x44, 0x7c, 0x44, 0x44, 0x44, 0x00 },  // A
  { 0x00, 0x30, 0x48, 0x48, 0x70, 0x48, 0x44, 0x00 },  // R
  { 0x00, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 },  // T
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  //  (space)
  { 0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x44, 0x00 },  // N
  { 0x00, 0x7c, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00 },  // E
  { 0x00, 0x00, 0x44, 0x54, 0x54, 0x54, 0x38, 0x00 },  // W
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  //  (space)
  { 0x00, 0x3c, 0x40, 0x40, 0x48, 0x44, 0x38, 0x00 },  // G
  { 0x00, 0x38, 0x44, 0x7c, 0x44, 0x44, 0x44, 0x00 },  // A
  { 0x00, 0x44, 0x6c, 0x54, 0x44, 0x44, 0x44, 0x00 },  // M
  { 0x00, 0x7c, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00 },  // E
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  //  (space)
};

// Grid
byte EMPTY_GRID[8] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
};

// Rockets
const int MAX_NUMBER_OF_ROCKETS = 20;
const int ALL_ROCKETS_HAVE_BEEN_USED = -1;
bool GAME_OVER = true;

struct Rocket {
  int coordX;
  int coordY;
  long unsigned updateTime;

  bool doesHitTarget(int, int);
};

Rocket ROCKET_NONE = { 0, 0, 0 };
Rocket activeRockets[MAX_NUMBER_OF_ROCKETS] = {};

int findRocketID() {
  for (int i = 0; i < MAX_NUMBER_OF_ROCKETS; i++) {
    if (isRocketNone(activeRockets[i])) {
      return i;
    }
  }

  return ALL_ROCKETS_HAVE_BEEN_USED;
}

bool isRocketNone(Rocket rocket) {
  return (
    rocket.coordX == ROCKET_NONE.coordX && rocket.coordY == ROCKET_NONE.coordY && rocket.updateTime == ROCKET_NONE.updateTime);
}

void deleteRocket(int rocketID) {
  activeRockets[rocketID] = ROCKET_NONE;
}

bool Rocket::doesHitTarget(int targetCoordX, int targetCoordY) {
  return (
    coordX == targetCoordX && coordY == targetCoordY);
}

// Spaceship
class Spaceship {
public:
  int coordX;
  int movementSpeed;
  int gunReloadingSpeed;

  int score = 0;

  bool canMoveLeft();
  bool canMoveRight();
  bool canShoot();

  bool isTouchingCoords(int, int);

  void moveLeft();
  void moveRight();
  void shootLeft();
  void shootRight();

private:
  long unsigned int lastMovementTime;
  long unsigned int lastGunReloadingTime;
};

bool Spaceship::canMoveLeft() {
  return (
    coordX > 1 && (millis() - lastMovementTime) > movementSpeed);
}

bool Spaceship::canMoveRight() {
  return (
    coordX < 6 && (millis() - lastMovementTime) > movementSpeed);
}

bool Spaceship::canShoot() {
  return (millis() - lastGunReloadingTime) > gunReloadingSpeed;
}

void Spaceship::shootLeft() {
  int rocketID = findRocketID();

  if (rocketID != ALL_ROCKETS_HAVE_BEEN_USED) {
    activeRockets[rocketID] = (Rocket){ coordX - 1, 2, millis() };
  }

  lastGunReloadingTime = millis();
}

void Spaceship::shootRight() {
  int rocketID = findRocketID();

  if (rocketID != ALL_ROCKETS_HAVE_BEEN_USED) {
    activeRockets[rocketID] = (Rocket){ coordX + 1, 2, millis() };
  }

  lastGunReloadingTime = millis();
}

bool Spaceship::isTouchingCoords(int placeCoordX, int placeCoordY) {
  bool isTouchingRightSide = (placeCoordX == coordX + 1 && (placeCoordY == 1 || placeCoordY == 2));
  bool isTouchingLeftSide = (placeCoordX == coordX - 1 && (placeCoordY == 1 || placeCoordY == 2));
  bool isTouchingFrontSide = (placeCoordX == coordX && placeCoordY == 1);

  return isTouchingRightSide || isTouchingLeftSide || isTouchingFrontSide;
}

void Spaceship::moveLeft() {
  if (coordX > 1) {
    coordX -= 1;
  }
  lastMovementTime = millis();
}

void Spaceship::moveRight() {
  if (coordX < 6) {
    coordX += 1;
  }
  lastMovementTime = millis();
}

Spaceship spaceship;

// Meteor
const int MAX_NUMBER_OF_METEORS = 64;
const int ALL_MOTEORS_HAVE_BEEN_USED = -1;

class Meteor {
public:
  int coordX;
  int coordY = 7;  // Always starts from the top part of the screen

  int movementSpeed = 2000;

  bool canMove();
  void moveForward();

private:
  long unsigned int lastMovementTime = 0;
};

bool Meteor::canMove() {
  return (millis() - lastMovementTime) > movementSpeed;
}

Meteor liveMeteors[MAX_NUMBER_OF_METEORS] = {};
bool occupiedMeteorPlace[MAX_NUMBER_OF_METEORS] = {};

void Meteor::moveForward() {
  int newCoordX = coordX + random(1);      // Move +1 forward or do not move at all
  int newCoordY = coordY + random(2) - 1;  // Move left, right or do not move at all

  bool isNewPositionOccupied = false;
  Meteor meteor;

  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    if (occupiedMeteorPlace[meteorID]) {
      meteor = liveMeteors[meteorID];

      if (meteor.coordX == newCoordX && meteor.coordY == newCoordY) {
        isNewPositionOccupied = true;
        break;
      }
    }
  }

  if (!isNewPositionOccupied) {
    coordX = newCoordX;
    coordY = newCoordY;
    lastMovementTime = millis();
  }
}

// Other

long lastCreatedMeteorTime = millis();
int createMeteorSpeed = 1000;

void createMeteor() {
  if ((millis() - lastCreatedMeteorTime) < createMeteorSpeed) {
    return;
  }

  Meteor meteor;
  meteor.coordX = random(8);

  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    if (!occupiedMeteorPlace[meteorID]) {
      occupiedMeteorPlace[meteorID] = true;
      liveMeteors[meteorID] = meteor;
      break;
    }
  }
}

// Game
byte currentGrid[8] = {};
byte meteorGrid[8] = {};
int gameSpeed = 500;

void drawGrid(byte* grid) {
  for (int row = 0; row < 8; row++) {
    lc.setColumn(0, row, grid[row]);
  }
}

void drawSpaceship(int coord) {
  int shift = coord - 1;
  currentGrid[1] = B10100000 >> shift;
  currentGrid[0] = B11100000 >> shift;
}

void drawRockets() {
  Rocket rocket;

  for (int rocketID = 0; rocketID < MAX_NUMBER_OF_ROCKETS; rocketID++) {
    rocket = activeRockets[rocketID];

    if (!isRocketNone(rocket)) {
      lc.setLed(0, rocket.coordX, rocket.coordY, true);

      if (rocket.coordY < 8) {
        activeRockets[rocketID].coordY += 1;
        activeRockets[rocketID].updateTime = millis();
      } else {
        deleteRocket(rocketID);
      }
    }
  }
}

void drawMeteors() {
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    Meteor meteor = liveMeteors[meteorID];

    if (occupiedMeteorPlace[meteorID]) {
      lc.setLed(0, meteor.coordX, meteor.coordY, true);

      if (meteor.canMove()) {
        liveMeteors[meteorID].moveForward();

        if (meteor.coordX < 0 || meteor.coordX > 7 || meteor.coordY < 0) {
          occupiedMeteorPlace[meteorID] = false;
        }
      }
    }
  }
}

bool isButtonPressed(int buttonID) {
  int buttonState = digitalRead(buttonID);
  int dx = map(analogRead(VRX), 0, 906, 2, -2);
  if (dx != 0) { dx = -dx / abs(dx); }

  return buttonState == HIGH;
}

int getDirection() {
  int dx = map(analogRead(VRX), 0, 906, 2, -2);
  if (dx != 0) { dx = -dx / abs(dx); }

  return dx;
}


void blinkScreen(int nTimes) {
  for (int i = 0; i < nTimes; i++) {
    for (int row = 0; row < 8; row++) {
      lc.setColumn(0, row, B11111111);
    }
    delay(50);

    for (int row = 0; row < 8; row++) {
      lc.setColumn(0, row, B00000000);
    }
    delay(50);
  }
}

void checkIfMeteorTouchedSpaceship() {
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    Meteor meteor = liveMeteors[meteorID];

    if (occupiedMeteorPlace[meteorID] && spaceship.isTouchingCoords(meteor.coordX, meteor.coordY)) {
      resetGame();

      blinkScreen(6);

      for (int i = 0; i < 10; i++) {
        for (int row = 0; row < 8; row++) {
          lc.setColumn(0, row, gameOverMessage[i][7 - row]);
        }

        delay(250);
      }
    }
  }
}

void checkIfSpaceshipHitMeteor() {
  Rocket rocket;
  Meteor meteor;

  for (int rocketID = 0; rocketID < MAX_NUMBER_OF_ROCKETS; rocketID++) {
    rocket = activeRockets[rocketID];

    if (!isRocketNone(rocket)) {
      for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
        meteor = liveMeteors[meteorID];

        if (occupiedMeteorPlace[meteorID] && rocket.doesHitTarget(meteor.coordX, meteor.coordY)) {
          occupiedMeteorPlace[meteorID] = false;
        }
      }  // meteor loop end
    }
  }  // rocket loop end
}

void resetGame() {
  GAME_OVER = true;

  drawGrid(EMPTY_GRID);

  // Set up default position for the spaceship
  spaceship.coordX = 4;
  spaceship.movementSpeed = 50;       // The lower - the faster.
  spaceship.gunReloadingSpeed = 200;  // The lower - the faster.

  // Clear all meteors
  for (int meteorID = 0; meteorID < MAX_NUMBER_OF_METEORS; meteorID++) {
    occupiedMeteorPlace[meteorID] = false;
  }

  // Clear all spaceship rockets
  for (int rocketID = 0; rocketID < MAX_NUMBER_OF_ROCKETS; rocketID++) {
    deleteRocket(rocketID);
  }
}

void setup() {
  Serial.begin(9600);

  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false);  // turn off power saving, enables display
  lc.setIntensity(0, 4);  // sets brightness (0~15 possible values)
  lc.clearDisplay(0);     // clear screen

  resetGame();
}

void loop() {
  if (!GAME_OVER) {
    if (getDirection() == -1 && spaceship.canMoveLeft()) {
      spaceship.moveLeft();
    }

    if (getDirection() == 1 && spaceship.canMoveRight()) {
      spaceship.moveRight();
    }

    if (spaceship.canShoot()) {
      spaceship.shootLeft();
      spaceship.shootRight();
    }

    if (random(10) < 3) {
      createMeteor();
    }

    memcpy(currentGrid, EMPTY_GRID, 8);

    drawSpaceship(spaceship.coordX);
    drawGrid(currentGrid);
    drawRockets();
    drawMeteors();

    checkIfMeteorTouchedSpaceship();
    checkIfSpaceshipHitMeteor();

  } else {
    for (int i = 0; i < 15; i++) {
      for (int row = 0; row < 8; row++) {
        lc.setColumn(0, row, startNewGameMessage[i][7 - row]);
      }

      if (getDirection() != 0) {
        GAME_OVER = false;

        blinkScreen(3);

        break;
      }

      delay(250);
    }
  }
}