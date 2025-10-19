#include <bluefruit.h>
#include <U8g2lib.h>  // https://github.com/olikraus/u8g2 (2.35.30)
#include "LSM6DS3.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ----- display -----
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

//Create a instance of class LSM6DS3
LSM6DS3 myIMU(I2C_MODE, 0x6A);  //I2C device address 0x6A

BLEDis bledis;
BLEHidAdafruit blehid;

#define MOVE_STEP 10
#define BUTTON_PIN D1

long lastBtnMs = 0;
bool clicked = false;

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Bluefruit.begin();
  // HID Device can have a min connection interval of 9*1.25 = 11.25 ms
  Bluefruit.Periph.setConnInterval(9, 16);  // min = 9*1.25=11.25 ms, max = 16*1.25=20ms
  Bluefruit.setTxPower(4);                  // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("TechTalkies");
  bledis.setModel("TechTalkies Mouse");
  bledis.begin();

  // BLE HID
  blehid.begin();

  // Set up and start advertising
  startAdv();

  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  delay(100);

  if (myIMU.begin() != 0) {
    Serial.println("Device error");
    u8g2.drawStr(10, 30, "IMU error");
  } else {
    Serial.println("Device OK!");
    u8g2.drawStr(10, 30, "Air mouse running");
  }
  u8g2.sendBuffer();
}

void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);

  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for 'Name' in the advertising packet
  Bluefruit.Advertising.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);    // number of seconds in fast mode
  Bluefruit.Advertising.start(0);              // 0 = Don't stop advertising after n seconds
}

void loop() {

  // --- read raw accel ---
  float x = myIMU.readFloatGyroX();
  float y = myIMU.readFloatGyroY();
  float z = myIMU.readFloatGyroZ();

  blehid.mouseMove(-z, y);

  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!clicked) {
      clicked = true;
      blehid.mouseButtonPress(MOUSE_BUTTON_LEFT);
    }
  } else if (clicked) {
    clicked = false;
    blehid.mouseButtonRelease();
  }
}
