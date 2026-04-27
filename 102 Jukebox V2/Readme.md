# 🎵 ESP32 RFID Jukebox

> Tap a card, play a song. A DIY RFID-controlled music player built with ESP32, DFPlayer Mini, and an OLED display.

**By [Tech Talkies](https://www.youtube.com/@techtalkies1)** · [techtalkies.in](https://techtalkies.in)

---

## Overview

The ESP32 RFID Jukebox lets you trigger music playback by tapping NFC/RFID cards or fobs against a reader. Each card is mapped to either a **specific track** or an entire **folder of songs** stored on a microSD card. A 128×64 OLED display shows what's currently playing.

Perfect for:
- Kids' music players (each card = a story or album)
- Party/event installations
- Retro jukebox builds
- Accessibility-focused audio players

---

## Hardware Required

| Component | Details |
|---|---|
| ESP32 board | Any variant. Connection diagram is for ESP32C3 Super mini|
| MFRC522 RFID reader | Classic 13.56 MHz SPI module |
| DFPlayer Mini | MP3 playback module with microSD slot |
| SH1106 OLED (128×64) | I2C display |
| Speaker | 4Ω or 8Ω, connected to DFPlayer |
| RFID cards / fobs | Standard MIFARE Classic 1K |
| microSD card | FAT32 formatted, with MP3 files |

---

## Pin Connections

### MFRC522 → ESP32

| MFRC522 | ESP32 GPIO |
|---|---|
| SDA (SS) | 5 |
| SCK | 6 |
| MOSI | 7 |
| MISO | 8 |
| RST | 9 |
| 3.3V / GND | 3.3V / GND |

### DFPlayer Mini → ESP32

| DFPlayer | ESP32 GPIO |
|---|---|
| TX | 0|
| RX | 1 |
| VCC / GND | 5V / GND |

### SH1106 OLED → ESP32

| OLED | ESP32 |
|---|---|
| SDA | GPIO 2 |
| SCL | GPIO 3 |
| VCC / GND | 3.3V / GND |

---

## SD Card Setup

Format your microSD card as **FAT32** and organise files like this:

```
/01/  ← Folders to act as playlists
    001.mp3
    002.mp3
/02/
    001.mp3
    002.mp3
/mp3/     ← All tracks inside mp3 folder
    001.mp3
    002.mp3
    003.mp3
```

DFPlayer Mini uses numbered folders (`01`, `02`, …) and numbered files (`001.mp3`, `002.mp3`, …). Files inside 'mp3' folder are used for individual track playback.

---

## Libraries

Install these via the Arduino Library Manager:

- [**DFRobotDFPlayerMini**](https://github.com/DFRobot/DFRobotDFPlayerMini) — DFRobot
- [**MFRC522**](https://github.com/miguelbalboa/rfid) — miguelbalboa
- [**Adafruit GFX Library**](https://github.com/adafruit/Adafruit-gfx-library) — Adafruit
- [**Adafruit SH110X**](https://github.com/adafruit/Adafruit_SH110X) — Adafruit
- [**EspSoftwareSerial**](https://github.com/plerup/espsoftwareserial) — plerup

---

## Card Configuration

Open the sketch and edit the two arrays to match your cards.

### Individual Song Cards

```cpp
SongCard songCards[] = {
  { "AA BB CC DD", 1 },   // UID → plays track 001.mp3
  { "AA BB CC DD", 2 },   // UID → plays track 002.mp3
  { "AA BB CC DD", 3 },
};
```

### Folder Cards (plays entire folder on loop)

```cpp
FolderCard folderCards[] = {
  { "AA BB CC DD", 1 },   // UID → loops folder /01/
  { "AA BB CC DD", 2 },   // UID → loops folder /02/
};
```

### Finding Your Card UIDs

Upload the "read_cards" sketch and open the Serial Monitor at **9600 baud**. Tap any card — the UID will print in the format `AA BB CC DD`. Copy it into the array above.

---

## How It Works

1. On startup the display shows **"Ready"** and the RFID reader initialises.
2. When a card is tapped, the UID is read and checked against `songCards[]` first, then `folderCards[]`.
3. A **song card** calls `player.play(track)` — plays that specific file once.
4. A **folder card** calls `player.loopFolder(folder)` — shuffles/loops the entire folder.
5. The OLED updates to show what's playing.
6. Unknown cards are silently ignored (logged to Serial).

---

## Customisation

- **Volume** — change `player.volume(25)` in `setup()` (range: 0–30)
- **Add more cards** — just extend the `songCards[]` or `folderCards[]` arrays
- **Debounce delay** — the `delay(1500)` in `loop()` prevents double-reads; adjust as needed

---

## License

```
Copyright (c) Tech Talkies YouTube Channel
https://www.youtube.com/@techtalkies1
```

Free to use and modify for personal and educational projects. A shoutout or link back to the channel is always appreciated! 🙏

---

*Made with ❤️ and solder by Tech Talkies*
