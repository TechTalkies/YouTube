/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1
-------------------------------------------------*/
#include <Arduino.h>
#include "T-Keyboard-S3-Pro_Drive.h"
#include "Arduino_GFX_Library.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDConsumerControl.h"
#include "Arduino_DriveBus_Library.h"
#include <WiFi.h>
#define KEY_HID_R 'r'
// IIC
#define IIC_SDA 42
#define IIC_SCL 2
// IIC 2
#define IIC_SDA_2 6
#define IIC_SCL_2 7
// N085-1212TBWIG06-C08
#define LCD_WIDTH 128
#define LCD_HEIGHT 128
#define LCD_MOSI 40
#define LCD_SCLK 41
#define LCD_DC 39
#define LCD_RST 38
#define LCD_BL 1
// Rotary Encoder
#define KNOB_DATA_A 4
#define KNOB_DATA_B 5
#define IIC_MAIN_DEVICE_ADDRESS 0x01  // The IIC address of the main device is 0x01 by default
#define IIC_LCD_CS_DEVICE_DELAY 20    // Screen waiting time


/* -----Customizable variables section----- */

const char *WIFI_SSID = "WifiName";
const char *WIFI_PASS = "Password";

#define GREY RGB565(128, 128, 128)  //Color for unfilled circle
#define BG_COLOR BLACK              //Screen background color
#define DANGER_THRESHOLD 85         //Threshold percentage for red alert
#define LED_MAX_BRIGHTNESS 75       //Maxium RGB LED brightness
#define BREATHING_DELAY 20          //Speed of white LED breathing

/* -----Customizable variables section----- */


const uint16_t TCP_PORT = 5000;
bool flashOn = false;
bool alertActive = true;
bool breathingUp = true;
unsigned long lastFlashMs = 0;
unsigned long breathingMs = 0;
uint8_t breathingVal = 0;

int8_t cpu_percent = -1;
int8_t ram_percent = -1;
int8_t gpu_percent = -1;
int8_t net_percent = -1;

uint32_t net_down_bps = 0;
uint32_t net_up_bps = 0;

char net_buf[64];
uint8_t net_idx = 0;

WiFiServer server(TCP_PORT);
WiFiClient client;

static char ip0[4];
static char ip1[4];
static char ip2[4];
static char ip3[4];

int8_t bar_radius = 27;
int8_t bottomX = 5;
int8_t bottomY = 100;

enum KNOB_State {
  KNOB_NULL,
  KNOB_INCREMENT,
  KNOB_DECREMENT,
};

int8_t KNOB_Data = 0;
bool KNOB_Trigger_Flag = false;
uint8_t KNOB_State_Flag = KNOB_State::KNOB_NULL;

//  0B000000[KNOB_DATA_A][KNOB_DATA_B]
uint8_t KNOB_Previous_Logical = 0B00000000;

uint8_t IIC_Master_Receive_Data;

size_t IIC_Bus_CycleTime = 0;
size_t KNOB_CycleTime = 0;

uint8_t KEY1_Lock = 0;
uint8_t KEY2_Lock = 0;
uint8_t KEY3_Lock = 0;
uint8_t KEY4_Lock = 0;
uint8_t KEY5_Lock = 0;

std::vector<unsigned char> IIC_Device_ID_Registry_Scan;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

std::vector<T_Keyboard_S3_Pro_Device_KEY> KEY_Trigger;

USBHIDKeyboard Keyboard;
USBHIDConsumerControl ConsumerControl;

/*LCD*/
bool IIC_Device_ID_State = false;
std::vector<unsigned char> IIC_Device_ID_Scan;
std::vector<unsigned char> IIC_Device_ID_Registry;
Arduino_DataBus *bus = new Arduino_HWSPI(
  LCD_DC /* DC */, -1 /* CS */, LCD_SCLK /* SCK */, LCD_MOSI /* MOSI */, -1 /* MISO */);

Arduino_GFX *gfx = new Arduino_GC9107(
  bus, -1 /* RST */, 0 /* rotation */, true /* IPS */,
  LCD_WIDTH /* width */, LCD_HEIGHT /* height */,
  2 /* col offset 1 */, 1 /* row offset 1 */, 0 /* col_offset2 */, 0 /* row_offset2 */);

Arduino_Canvas *sprite = new Arduino_Canvas(128, 128, gfx);

uint16_t valueToColor(uint8_t value) {
  if (value > 100)
    value = 100;

  const uint8_t MIN = 40;
  const uint8_t MAX = 180;

  uint8_t r, g;

  if (value <= 50) {
    // green → yellow
    r = map(value, 0, 50, MIN, MAX);
    g = MAX;
  } else {
    // yellow → red
    r = MAX;
    g = map(value, 50, 100, MAX, MIN);
  }

  return gfx->color565(r, g, 0);
}

void KNOB_Logical_Scan_Loop(void) {
  uint8_t KNOB_Logical_Scan = 0B00000000;

  if (digitalRead(KNOB_DATA_A) == 1) {
    KNOB_Logical_Scan |= 0B00000010;
  } else {
    KNOB_Logical_Scan &= 0B11111101;
  }

  if (digitalRead(KNOB_DATA_B) == 1) {
    KNOB_Logical_Scan |= 0B00000001;
  } else {
    KNOB_Logical_Scan &= 0B11111110;
  }

  if (KNOB_Previous_Logical != KNOB_Logical_Scan) {
    if (KNOB_Logical_Scan == 0B00000000 || KNOB_Logical_Scan == 0B00000011) {
      KNOB_Previous_Logical = KNOB_Logical_Scan;
      KNOB_Trigger_Flag = true;
    } else {
      if (KNOB_Logical_Scan == 0B00000010) {
        switch (KNOB_Previous_Logical) {
          case 0B00000000:
            KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
            break;
          case 0B00000011:
            KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
            break;

          default:
            break;
        }
      }
      if (KNOB_Logical_Scan == 0B00000001) {
        switch (KNOB_Previous_Logical) {
          case 0B00000000:
            KNOB_State_Flag = KNOB_State::KNOB_DECREMENT;
            break;
          case 0B00000011:
            KNOB_State_Flag = KNOB_State::KNOB_INCREMENT;
            break;

          default:
            break;
        }
      }
    }
    // delay(10);
  }
}

void IIC_KEY_Read_Loop(void) {
  if (IIC_Device_ID_Registry_Scan.size() > 0) {
    // Scan to all devices receiving data
    for (int i = 0; i < IIC_Device_ID_Registry_Scan.size(); i++) {
      IIC_Bus->IIC_ReadC8_Data(IIC_Device_ID_Registry_Scan[i], T_KEYBOARD_S3_PRO_RD_KEY_TRIGGER,
                               &IIC_Master_Receive_Data, 1);

      T_Keyboard_S3_Pro_Device_KEY key_trigger_temp;
      key_trigger_temp.ID = IIC_Device_ID_Registry_Scan[i];
      key_trigger_temp.Trigger_Data = IIC_Master_Receive_Data;

      KEY_Trigger.push_back(key_trigger_temp);
    }
  }
}

void IIC_KEY_Trigger_Loop(void) {
  if (KEY_Trigger.size() > 0) {
    switch (KEY_Trigger[0].ID) {
      case 0x01:
        if (((KEY_Trigger[0].Trigger_Data & 0B00010000) >> 4) == 1) {
          if (KEY1_Lock == 0) {
            // TODO
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press(KEY_ESC);
            delay(20);
            Keyboard.release(KEY_LEFT_CTRL);
            Keyboard.release(KEY_LEFT_SHIFT);
            Keyboard.release(KEY_ESC);
            delay(80);
          }
          KEY1_Lock = 1;
        } else {
          KEY1_Lock = 0;  // loop auto release
        }

        if (((KEY_Trigger[0].Trigger_Data & 0B00001000) >> 3) == 1) {
          if (KEY2_Lock == 0) {
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('b');
            delay(20);
            Keyboard.releaseAll();
            delay(80);
          }
          KEY2_Lock = 1;
        } else {
          KEY2_Lock = 0;
        }

        if (((KEY_Trigger[0].Trigger_Data & 0B00000100) >> 2) == 1) {
          if (KEY3_Lock == 0) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press(KEY_HID_R);
            delay(20);
            Keyboard.release(KEY_LEFT_GUI);
            Keyboard.release(KEY_HID_R);
            delay(80);
            Keyboard.sendString("resmon");
            delay(20);
            Keyboard.press(KEY_RETURN);
            delay(20);
            Keyboard.release(KEY_RETURN);
          }

          KEY3_Lock = 1;
        } else {
          KEY3_Lock = 0;
        }

        if (((KEY_Trigger[0].Trigger_Data & 0B00000010) >> 1) == 1) {
          if (KEY4_Lock == 0) {
            Keyboard.press(KEY_LEFT_GUI);
            Keyboard.press('a');
            delay(20);
            Keyboard.releaseAll();
            delay(1000);

            Keyboard.press(KEY_RETURN);
            Keyboard.release(KEY_RETURN);
            delay(1000);
            Keyboard.press(KEY_RETURN);
            Keyboard.release(KEY_RETURN);
          }
          KEY4_Lock = 1;
        } else {
          KEY4_Lock = 0;
        }

        if ((KEY_Trigger[0].Trigger_Data & 0B00000001) == 1) {
          if (KEY5_Lock == 0) {
          }
        }

        break;

      default:
        Keyboard.release(KEY1_Lock);
        Keyboard.release(KEY2_Lock);
        Keyboard.release(KEY3_Lock);
        Keyboard.release(KEY4_Lock);
        break;
    }
    KEY_Trigger.erase(KEY_Trigger.begin());
  }
}

void KNOB_Trigger_Loop(void) {
  KNOB_Logical_Scan_Loop();

  if (KNOB_Trigger_Flag == true) {
    KNOB_Trigger_Flag = false;

    switch (KNOB_State_Flag) {
      case KNOB_State::KNOB_INCREMENT:
        KNOB_Data++;
        Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
        ConsumerControl.press(CONSUMER_CONTROL_VOLUME_INCREMENT);
        ConsumerControl.release();

        delay(200);
        break;
      case KNOB_State::KNOB_DECREMENT:
        KNOB_Data--;
        Serial.printf("\nKNOB_Data: %d\n", KNOB_Data);
        ConsumerControl.press(CONSUMER_CONTROL_VOLUME_DECREMENT);
        ConsumerControl.release();
        delay(200);
        break;

      default:
        break;
    }
  } else {
    ConsumerControl.release();
  }
}

void Task1(void *pvParameters) {
  // You can add some code here, so if this task is executed, the content here will be executed once
  // Of course, after entering the while loop, it won't be executed again in this part

  while (1) {
    IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Registry_Scan);
    if (millis() > IIC_Bus_CycleTime) {
      if (IIC_Bus->IIC_Device_7Bit_Scan(&IIC_Device_ID_Scan) == true) {
        if (IIC_Device_ID_Scan.size() != IIC_Device_ID_Registry.size()) {
          IIC_Device_ID_State = true;
        }
      }
      // Modify the device status
      IIC_Bus_CycleTime = millis() + 10;  // 10ms
    }
    delay(1);  // Modify the device status
  }
}

void LEDTask() {
  unsigned long now = millis();

  if (cpu_percent > DANGER_THRESHOLD || gpu_percent > DANGER_THRESHOLD || ram_percent > DANGER_THRESHOLD) {
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H, (uint8_t)((uint16_t)360 >> 8));  // Set the Hue value of the LED.
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L, (uint8_t)360);                   // Set the Hue value of the LED.
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION, 100);                      // Set the LED's color status value

    flashOn = !flashOn;

    if (now - lastFlashMs > 250) {
      lastFlashMs = now;

      if (flashOn)  // Set Red
      {
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, LED_MAX_BRIGHTNESS);
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111);  // Select LED
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111);  // Select LED
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000);  // Display LED
        delay(5);
      } else  // Off
      {
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, 0);
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111);  // Select LED
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111);  // Select LED
        IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000);  // Display LED
        delay(5);
      }
    }
  } else {
    // Breathe White
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_COLOR_STATURATION, 0);  // Set the LED's color status value

    if (now - breathingMs < BREATHING_DELAY)
      return;

    breathingMs = now;
    // update value
    if (breathingUp) {
      if (breathingVal < LED_MAX_BRIGHTNESS)
        breathingVal++;
      else
        breathingUp = false;
    } else {
      if (breathingVal > 5)
        breathingVal--;
      else
        breathingUp = true;
    }
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_BRIGHTNESS, breathingVal);  // Set the LED's color status value
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B00111111);     // Select LED
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_2, 0B11111111);     // Select LED
    IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_CONTROL_1, 0B01000000);     // Display LED
    delay(5);
  }
}

void ButtonsTask(void *pvParameters) {
  for (;;) {
    IIC_KEY_Read_Loop();
    IIC_KEY_Trigger_Loop();
    if (millis() > KNOB_CycleTime) {
      KNOB_Logical_Scan_Loop();
      KNOB_CycleTime = millis() + 20;  // 20ms
    }
    KNOB_Trigger_Loop();

    LEDTask();

    vTaskDelay(pdMS_TO_TICKS(2));  // ~500 Hz, very responsive
  }
}

void Select_Screen_All(std::vector<unsigned char> device_id, bool select) {
  if (select == true) {
    for (int i = 0; i < device_id.size(); i++) {
      IIC_Bus->IIC_WriteC8D8(device_id[i],
                             T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00011111);  // Select all screen
      delay(IIC_LCD_CS_DEVICE_DELAY);
    }
  } else {
    for (int i = 0; i < device_id.size(); i++) {
      IIC_Bus->IIC_WriteC8D8(device_id[i],
                             T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000000);  // Cancel screen
      delay(IIC_LCD_CS_DEVICE_DELAY);
    }
  }
}

void draw_text_centered(int cx, int cy, const char *text) {
  int16_t x1, y1;
  uint16_t w, h;

  sprite->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = cx - (w / 2);
  int y = cy + (h / 2);  // baseline correction

  sprite->setCursor(x, y);
  sprite->println(text);
}

void set_text_to_sprite(uint8_t x, uint8_t y, const char *text, uint16_t textSize = 2) {
  sprite->setTextSize(textSize);
  draw_text_centered(x, y, text);
}

void set_progress_bar(uint8_t progress) {
  uint16_t angle = map(progress, 0, 100, 0, 360);
  // sprite->drawCircle(64, 60, bar_radius, WHITE);
  sprite->fillArc(64, 60, bar_radius + 4, bar_radius - 4, 0, 360, GREY);
  sprite->fillArc(64, 60, bar_radius + 4, bar_radius - 4, 0, angle, valueToColor(progress));
  sprite->setTextColor(valueToColor(progress));
  char buf[8];
  snprintf(buf, sizeof(buf), "%u%%", progress);
  set_text_to_sprite(64, 45, buf);
  sprite->setTextColor(WHITE);
}

void draw_down_arrow(int x, int y, uint16_t color) {
  sprite->fillTriangle(
    x, y,
    x - 4, y - 6,
    x + 4, y - 6,
    color);
}

void draw_up_arrow(int x, int y, uint16_t color) {
  sprite->fillTriangle(
    x, y,
    x - 4, y + 6,
    x + 4, y + 6,
    color);
}

void format_kb(uint32_t bytes_per_sec, char *out, size_t len) {
  uint32_t kb = bytes_per_sec / 1024;
  snprintf(out, len, "%u KB", kb);
}

void draw_net_stats(
  int x,
  int y,
  uint32_t down_bps,
  uint32_t up_bps,
  uint16_t color) {
  char buf[16];

  // down
  draw_down_arrow(x + 2, y, color);
  format_kb(down_bps, buf, sizeof(buf));
  set_text_to_sprite(x + 50, y - 15, buf);

  // up
  draw_up_arrow(x, y + 15, color);
  format_kb(up_bps, buf, sizeof(buf));
  set_text_to_sprite(x + 50, y + 15, buf);
}

void update_screen(std::vector<unsigned char> device_id, bool connected) {
  sprite->setTextColor(WHITE);

  Select_Screen_All(device_id, true);

  for (int i = 0; i < device_id.size(); i++) {
    IIC_Bus->IIC_WriteC8D8(device_id[i],
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00010000);  // Select screen 1 LCD_1
    delay(IIC_LCD_CS_DEVICE_DELAY);
  }

  if (cpu_percent > DANGER_THRESHOLD)
    sprite->fillScreen(RED);
  else
    sprite->fillScreen(BG_COLOR);
  set_text_to_sprite(64, 0, "CPU");
  if (connected)
    set_progress_bar(cpu_percent);
  else
    set_text_to_sprite(64, 40, ip0, 3);
  sprite->drawLine(bottomX, bottomY, 123, bottomY, WHITE);
  set_text_to_sprite(64, bottomY, "Tasks");
  sprite->flush();
  delay(100);

  for (int i = 0; i < device_id.size(); i++) {
    IIC_Bus->IIC_WriteC8D8(device_id[i],
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00001000);  // Select screen 2 LCD_2
    delay(IIC_LCD_CS_DEVICE_DELAY);
  }
  if (gpu_percent > DANGER_THRESHOLD)
    sprite->fillScreen(RED);
  else
    sprite->fillScreen(BG_COLOR);
  set_text_to_sprite(64, 0, "GPU");
  if (connected)
    set_progress_bar(gpu_percent);
  else
    set_text_to_sprite(64, 40, ip1, 3);
  sprite->drawLine(bottomX, bottomY, 123, bottomY, WHITE);
  set_text_to_sprite(64, bottomY, "Reset");
  sprite->flush();
  delay(100);

  for (int i = 0; i < device_id.size(); i++) {
    IIC_Bus->IIC_WriteC8D8(device_id[i],
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000100);  // Select screen 3 LCD_3
    delay(IIC_LCD_CS_DEVICE_DELAY);
  }
  if (ram_percent > DANGER_THRESHOLD)
    sprite->fillScreen(RED);
  else
    sprite->fillScreen(BG_COLOR);
  set_text_to_sprite(64, 0, "RAM");
  if (connected)
    set_progress_bar(ram_percent);
  else
    set_text_to_sprite(64, 40, ip2, 3);
  sprite->drawLine(bottomX, bottomY, 123, bottomY, WHITE);
  set_text_to_sprite(64, bottomY, "Resource");
  sprite->flush();
  delay(100);

  for (int i = 0; i < device_id.size(); i++) {
    IIC_Bus->IIC_WriteC8D8(device_id[i],
                           T_KEYBOARD_S3_PRO_WR_LCD_CS, 0B00000010);  // Select screen 4 LCD_4
    delay(IIC_LCD_CS_DEVICE_DELAY);
  }

  sprite->fillScreen(BG_COLOR);
  set_text_to_sprite(64, 0, "NETWORK");
  if (connected)
    draw_net_stats(20, 50, net_down_bps, net_up_bps, WHITE);
  else
    set_text_to_sprite(64, 40, ip3, 3);

  sprite->drawLine(bottomX, bottomY, 123, bottomY, WHITE);
  set_text_to_sprite(64, bottomY, "Wi-Fi");
  sprite->flush();
  delay(100);

  Select_Screen_All(device_id, false);
}

void LCD_Initialization(std::vector<unsigned char> device_id) {
  Select_Screen_All(device_id, true);

  /* This step is important
   All LCD_CS pins must be selected and initialized once for the entire screen to be used.
   If the screen is not initialized, it will be uncontrollable.
   When multiple external devices are connected, the SPI speed must be below 4.5MHz; otherwise, the screen data will not be fully displayed.
   When adding screens from other devices, you also need to reinitialize the device.*/

  gfx->begin(4500000);
  gfx->fillScreen(BLACK);
  delay(500);

  Select_Screen_All(device_id, false);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Ciallo");

  pinMode(KNOB_DATA_A, INPUT_PULLUP);
  pinMode(KNOB_DATA_B, INPUT_PULLUP);

  while (IIC_Bus->begin() == false) {
    Serial.println("IIC_Bus initialization fail");
    delay(2000);
  }
  Serial.println("IIC_Bus initialization successfully");

  xTaskCreatePinnedToCore(Task1, "Task1", 10000, NULL, 1, NULL, 1);

  delay(100);

  Keyboard.begin();
  ConsumerControl.begin();
  USB.begin();

  delay(1000);

  // testLEDs(IIC_MAIN_DEVICE_ADDRESS);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected. ESP32 IP: ");
  Serial.println(WiFi.localIP());

  IPAddress ip = WiFi.localIP();

  snprintf(ip0, sizeof(ip0), "%d", ip[0]);
  snprintf(ip1, sizeof(ip1), "%d", ip[1]);
  snprintf(ip2, sizeof(ip2), "%d", ip[2]);
  snprintf(ip3, sizeof(ip3), "%d", ip[3]);

  server.begin();
  Serial.printf("TCP server started on port %d\n", TCP_PORT);

  /*LCD*/
  pinMode(LCD_RST, OUTPUT);
  digitalWrite(LCD_RST, HIGH);
  delay(100);
  digitalWrite(LCD_RST, LOW);
  delay(100);
  digitalWrite(LCD_RST, HIGH);
  delay(100);
  ledcAttachPin(LCD_BL, 1);
  ledcSetup(1, 2000, 8);
  ledcWrite(1, 0);  // brightness 0 - 255

  while (1) {
    bool temp = false;
    if (IIC_Device_ID_State == true) {
      delay(100);

      Serial.printf("Find IIC ID: %#X\n", IIC_Device_ID_Scan[0]);

      // Initialize only the main device; other devices will be tested later.
      if (IIC_Device_ID_Scan[0] == IIC_MAIN_DEVICE_ADDRESS) {
        IIC_Device_ID_Registry.push_back(IIC_Device_ID_Scan[0]);

        std::vector<unsigned char> vector_temp;
        vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);

        LCD_Initialization(vector_temp);

        temp = true;
      }
    } else {
      temp = false;
    }

    if (temp == true) {
      Serial.println("IIC_Bus select LCD_CS successful");
      break;
    } else {
      Serial.println("IIC ID not found");
      delay(1000);
    }
  }

  if (IIC_Bus->IIC_ReadC8_Delay_Data(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_RD_DRIVE_FIRMWARE_VERSION, 20, &IIC_Master_Receive_Data, 1) == true) {
    Serial.printf("STM32 dirve firmware version: %#X \n", IIC_Master_Receive_Data);
  }

  sprite->begin();

  std::vector<unsigned char> vector_temp;
  vector_temp.push_back(IIC_MAIN_DEVICE_ADDRESS);
  update_screen(vector_temp, false);

  xTaskCreatePinnedToCore(
    ButtonsTask,    // function
    "ButtonsTask",  // name
    2048,           // stack (buttons don’t need much)
    NULL,
    3,  // PRIORITY (higher than Task1)
    NULL,
    1  // SAME core as Task1
  );

  IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_MODE, 0B00000010);  // Select LED mode as free mode.
  delay(1000);
  IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_H, (uint8_t)((uint16_t)360 >> 8));  // red hue high
  IIC_Bus->IIC_WriteC8D8(IIC_MAIN_DEVICE_ADDRESS, T_KEYBOARD_S3_PRO_WR_LED_COLOR_HUE_L, (uint8_t)360);                   // Set the Hue value of the LED.
}

void parse_stats(char *line) {
  char *token = strtok(line, ";");

  while (token != NULL) {
    if (strncmp(token, "CPU=", 4) == 0) {
      cpu_percent = atoi(token + 4);
    } else if (strncmp(token, "RAM=", 4) == 0) {
      ram_percent = atoi(token + 4);
    } else if (strncmp(token, "GPU=", 4) == 0) {
      gpu_percent = atoi(token + 4);
    } else if (strncmp(token, "NET=", 4) == 0) {
      net_percent = atoi(token + 4);
    } else if (strncmp(token, "DOWN=", 5) == 0) {
      net_down_bps = atol(token + 5);
    } else if (strncmp(token, "UP=", 3) == 0) {
      net_up_bps = atol(token + 3);
    }

    token = strtok(NULL, ";");
  }

  // debug output
  Serial.printf(
    "CPU=%d RAM=%d GPU=%d NET=%d DOWN=%u UP=%u\n",
    cpu_percent,
    ram_percent,
    gpu_percent,
    net_percent,
    net_down_bps,
    net_up_bps);

  char *stats_line = (char *)malloc(64);
  snprintf(stats_line, 64, "CPU=%d RAM=%d GPU=%d NET=%d DOWN=%u UP=%u\n",
           cpu_percent,
           ram_percent,
           gpu_percent,
           net_percent,
           net_down_bps,
           net_up_bps);
  update_screen(IIC_Device_ID_Registry, true);
  free(stats_line);
}

void read_tcp() {
  // accept new client if needed
  if (!client || !client.connected()) {
    client = server.available();
    net_idx = 0;
    update_screen(IIC_Device_ID_Registry, false);
    return;
  }

  while (client.available()) {
    char c = client.read();

    if (c == '\n') {
      net_buf[net_idx] = '\0';
      parse_stats(net_buf);
      net_idx = 0;
    } else if (net_idx < sizeof(net_buf) - 1) {
      net_buf[net_idx++] = c;
    } else {
      // overflow protection
      net_idx = 0;
    }
  }
}

void loop() {
  read_tcp();
  // LEDTask();
}