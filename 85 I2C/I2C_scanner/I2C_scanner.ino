/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/

#include <Wire.h>
#define MAX_I2C_DEVICES 16

struct I2CDeviceInfo {
  uint8_t address;
  String name;
};


I2CDeviceInfo foundDevices[MAX_I2C_DEVICES];
int foundCount = 0;
int currentIndex = 0;

// === Known Devices Map ===
struct KnownI2C {
  uint8_t address;
  const char *name;
};

static const KnownI2C I2C_KNOWN_LIST[] = {
  { 0x0B, "LC709203F" },
  { 0x0C, "MLX90393" },
  { 0x0D, "MLX90393" },
  { 0x0E, "MAG3110" },
  { 0x10, "GPS/VEML6075/VEML7700" },
  { 0x11, "Si4713" },
  { 0x12, "PMSA0031" },
  { 0x13, "VCNL40x0" },
  { 0x18, "LIS3DH/MCP9808 (Built-in)" },
  { 0x19, "LIS3DH/MCP9808" },
  { 0x1E, "HMC5883/LIS2MDL" },
  { 0x20, "MCP23017" },
  { 0x23, "BH1750" },
  { 0x24, "PN532" },
  { 0x26, "MSA301" },
  { 0x28, "BNO055/CAP1188/PCT2075" },
  { 0x29, "BNO055/TCS34725/VL53L0X" },
  { 0x30, "NeoKey/NeoSlider" },
  { 0x33, "MLX90640" },
  { 0x36, "MAX17048" },
  { 0x38, "AHT20/AHT10" },
  { 0x39, "APDS9960/TSL2561" },
  { 0x3C, "SSD1306 OLED" },
  { 0x3D, "SSD1306 OLED" },
  { 0x40, "INA219/SHT40/PCA9685" },
  { 0x42, "INA219" },
  { 0x48, "ADS1115/TMP102" },
  { 0x49, "ADS1115/AS7262" },
  { 0x5A, "MPR121/CCS811" },
  { 0x5C, "AM2315/LPS22/BH1750" },
  { 0x68, "RTC/MPU-6050" },
  { 0x69, "MPU-9250/ICM20649" },
  { 0x70, "HT16K33/TCA9548" },
  { 0x76, "BME280/BMP388" },
  { 0x77, "BME280/BMP280" },
};

String getDeviceName(uint8_t addr) {
  for (auto &x : I2C_KNOWN_LIST) {
    if (x.address == addr) return String(x.name);
  }
  return "Not defined";
}

void setup() {
  Serial.begin(9600);
  Serial.println("\nI2C Scanner");
  Serial.println("");

  Wire.begin();
  foundCount = 0;
  currentIndex = 0;
}

void loop() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);

      foundDevices[foundCount].address = address;
      foundDevices[foundCount].name = getDeviceName(address);
      foundCount++;

      Serial.print("Possibly: ");
      Serial.println(foundDevices[foundCount-1].name);
      Serial.println();

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found");
  else
    Serial.println("done");

  delay(5000);  // wait 5 seconds for next scan
}
