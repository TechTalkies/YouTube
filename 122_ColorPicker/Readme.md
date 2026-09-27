# Real Life Color Picker

I built this **IRL Color Picker** because I often come across colors in the real world that I want to use in a design. Instead of trying to remember a color or take a photo and work it out later, this device lets me point the sensor at something, scan its color, and get an RGB/HEX value.

The project is built around a [**Xiao ESP32-S3**](https://www.seeedstudio.com/XIAO-ESP32S3-p-5627.html?sensecap_affiliate=P9GHEkF&referring_service=link), a **TCS34725 color sensor**, and an [**1.47" ST7789 color display**](https://www.seeedstudio.com/1-47inch-172x320-Resolution-LCD-Display-Module-p-5756.html?sensecap_affiliate=P9GHEkF&referring_service=link).

[![Watch the video](https://img.youtube.com/vi/XO6Na3-kmec/maxresdefault.jpg)](https://youtu.be/XO6Na3-kmec)

**Video:** https://youtu.be/XO6Na3-kmec

## Features

- Scan a color from the real world
- Convert the sensor reading into RGB values
- Display the resulting HEX color
- Save colors to non-volatile storage
- Browse saved colors
- Delete saved colors
- Three-button interface
- FreeRTOS task for button scanning and debouncing

## Hardware

| Component | Description |
|---|---|
| ESP32-S3 | Main controller |
| TCS34725 | RGB + clear-light color sensor |
| ST7789 | 172 × 320 color display |
| 3 × Push buttons | Left, Center and Right controls |

## Connections

### ST7789 Display

The display connections used by the supplied Arduino code are:

| ST7789 | ESP32-S3 |
|---|---:|
| CS | GPIO 8 |
| DC | GPIO 9 |
| RST | GPIO 4 |
| MOSI / DIN | GPIO 44 |
| SCLK / CLK | GPIO 7 |

### TCS34725 Color Sensor

| TCS34725 | ESP32-S3 |
|---|---:|
| SDA | GPIO 5 |
| SCL | GPIO 6 |
| VCC | 3.3 V |
| GND | GND |

### Buttons

The buttons are configured with `INPUT_PULLUP`, so each button connects its GPIO to GND when pressed.

| Button | ESP32-S3 |
|---|---:|
| Left | GPIO 1 |
| Center | GPIO 2 |
| Right | GPIO 43 |

## Controls

| Screen | Button | Action |
|---|---|---|
| Main menu | Left / Right | Select Scan or Browse |
| Main menu | Center | Enter selected mode |
| Scan | Right — hold | Start live color scanning |
| Live scan | Right — release | Stop scanning |
| Result | Center | Save color |
| Result | Left | Discard color |
| Browse | Left / Right | Previous / next saved color |
| Browse | Center | Delete selected color |
| Delete confirmation | Center | Confirm deletion |
| Delete confirmation | Left | Cancel |

## Color Reading

The TCS34725 provides raw 16-bit readings for red, green, blue and clear light.

The code converts these raw readings into an `0–255` RGB representation by normalizing the RGB channels against the clear-channel reading.

The resulting color is then packed as:

```text
#RRGGBB
```

For example:

```text
R = 255
G = 64
B = 32

HEX = #FF4020
```

The current conversion is intentionally simple and can be calibrated further for different lighting conditions and target surfaces.

## Display

The Arduino version uses the Adafruit display libraries:

- Adafruit GFX Library
- Adafruit ST7735 and ST7789 Library

The display is initialized at:

```cpp
tft.init(172, 320);
```

and used in landscape orientation.

## Software

### Arduino Libraries

Install the following libraries through the Arduino IDE Library Manager:

- **Adafruit GFX Library**
- **Adafruit ST7735 and ST7789 Library**
- **Adafruit TCS34725**
- **Adafruit BusIO**

`Preferences` and FreeRTOS support are provided by the ESP32 Arduino core.

## FreeRTOS

The project keeps a dedicated FreeRTOS task for button scanning.

The button task:

1. Polls the three buttons.
2. Debounces the inputs.
3. Detects press and release events.
4. Sends events to a FreeRTOS queue.

The main Arduino `loop()` processes the queued events and runs the UI state machine.

This keeps button handling separate from display rendering and sensor sampling.

## UI Flow

```text
             ┌─────────────┐
             │  Main Menu  │
             └──────┬──────┘
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
     ┌─────────┐         ┌─────────┐
     │  Scan   │         │ Browse  │
     └────┬────┘         └────┬────┘
          │                   │
          ▼                   ▼
    ┌───────────┐       Saved Colors
    │ Live Scan │
    └─────┬─────┘
          │
          ▼
    ┌───────────┐
    │  Result   │
    └─────┬─────┘
          │
       Save?
          │
          ▼
    ┌───────────┐
    │   Saved   │
    └───────────┘
```

## Storage

Saved colors are stored using the ESP32 Arduino `Preferences` API.

The project stores up to **20 colors**.

Each saved color is stored as a packed 24-bit RGB value:

```text
0xRRGGBB
```

## Source Code

The accompanying Arduino sketch is the reference implementation for the project:

```text
122_ColorPicker.ino
```

The code is intentionally kept as a single Arduino sketch so it can be opened and uploaded directly from the Arduino IDE once the required libraries are installed.

## Video

[![IRL Color Picker](https://img.youtube.com/vi/XO6Na3-kmec/maxresdefault.jpg)](https://youtu.be/XO6Na3-kmec)

**Watch the build:** https://youtu.be/XO6Na3-kmec

