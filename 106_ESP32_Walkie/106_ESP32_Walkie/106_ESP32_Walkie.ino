/* -------------------------------------------------
Copyright (c)
Arduino project by Tech Talkies YouTube Channel.
https://www.youtube.com/@techtalkies1

DEVICE IDENTITY:
Flash the same firmware on both devices.
The board reads its own MAC at boot and automatically sets the correct peer.
Fill in both MAC addresses before flashing.
-------------------------------------------------*/

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_idf_version.h>
#include <ESP_I2S.h>
#include <math.h>
#include "secrets.h"


uint8_t *peerMac = nullptr;  // Assigned at runtime

// ============================================================================
// STATUS TONE STATE & CONFIG
// ============================================================================
enum WalkieTone {
  TONE_NONE = 0,
  TONE_STARTUP,
  TONE_PTT_PRESS,
  TONE_PTT_RELEASE,
  TONE_PING_OK,
  TONE_PING_FAIL
};

volatile int pendingTone = TONE_NONE;
volatile bool isTransmitting = false;
volatile bool waitingForPong = false;
unsigned long pingSentTime = 0;
unsigned long lastPingTime = 0;
const unsigned long PING_INTERVAL = 10000;  // 10 seconds link test interval

// ============================================================================
// PIN CONFIGURATION
// ============================================================================

// Onboard PDM mic — fixed for XIAO ESP32S3 Sense
#define PIN_PDM_CLK 42
#define PIN_PDM_DATA 41

// MAX98357A I2S speaker amp — customizable
#define PIN_SPK_WS 43   // D6
#define PIN_SPK_BCLK 6  // D5
#define PIN_SPK_DIN 5   // D4

// Push-to-talk button wired to GND (active LOW, internal pull-up)
#define PIN_PTT 9  // D7

// ============================================================================
// AUDIO CONFIG
// ============================================================================
#define SAMPLE_RATE 16000
#define SAMPLES_PER_PACKET 128
#define GAIN 4  // Software gain — raise if mic is too quiet

I2SClass pdmMic;
I2SClass i2sSpeaker;

// ============================================================================
// RING BUFFER
// WiFi ISR (Core 0) pushes decoded audio in via ringPush().
// Main loop (Core 1) drains it to i2sSpeaker via ringPop().
// Keeps I2S writes on a single thread — prevents audio corruption.
// ============================================================================
#define RING_SIZE 2048  // 128 ms headroom at 16 kHz

static int16_t ringBuf[RING_SIZE];
static volatile int ringHead = 0;
static volatile int ringTail = 0;

inline int ringAvailable() {
  return (ringHead - ringTail + RING_SIZE) % RING_SIZE;
}
inline int ringFree() {
  return RING_SIZE - ringAvailable() - 1;
}

void ringPush(const int16_t *data, int count) {
  if (ringFree() < count) {
    // Drop oldest samples to make room rather than blocking
    ringTail = (ringTail + (count - ringFree())) % RING_SIZE;
  }
  for (int i = 0; i < count; i++) {
    ringBuf[ringHead] = data[i];
    ringHead = (ringHead + 1) % RING_SIZE;
  }
}

void ringPop(int16_t *data, int count) {
  int avail = ringAvailable();
  int toRead = (avail < count) ? avail : count;
  for (int i = 0; i < toRead; i++) {
    data[i] = ringBuf[ringTail];
    ringTail = (ringTail + 1) % RING_SIZE;
  }
  // Silence padding if buffer ran dry (prevents speaker noise on underrun)
  for (int i = toRead; i < count; i++) data[i] = 0;
}

inline void ringFlush() {
  ringTail = ringHead;  // Single assignment — safe on 32-bit ESP32
}

// ============================================================================
// STATUS TONES GENERATOR
// ============================================================================
void playPureTone(float frequency, int durationMs, float volume) {
  int numSamples = (SAMPLE_RATE * durationMs) / 1000;
  if (numSamples <= 0) return;

  int16_t *toneBuffer = (int16_t *)malloc(numSamples * sizeof(int16_t));
  if (!toneBuffer) return;

  for (int i = 0; i < numSamples; i++) {
    float t = (float)i / SAMPLE_RATE;
    float sample = sin(2.0 * 3.1415926535 * frequency * t);
    // Scale safely to protect speaker coil (amplitude limit 16384 out of 32767)
    toneBuffer[i] = (int16_t)(sample * 16384.0 * volume);
  }

  // Write directly to I2S speaker
  i2sSpeaker.write((const uint8_t *)toneBuffer, numSamples * sizeof(int16_t));
  free(toneBuffer);
}

void playStatusTone(int toneType) {
  if (toneType == TONE_STARTUP) {
    playPureTone(523.25, 120, 0.35);  // C5
    delay(40);
    playPureTone(783.99, 180, 0.35);  // G5
  } else if (toneType == TONE_PTT_PRESS) {
    playPureTone(880.0, 45, 0.25);   // A5
    playPureTone(1200.0, 45, 0.25);  // D6-ish
  } else if (toneType == TONE_PTT_RELEASE) {
    playPureTone(1000.0, 80, 0.3);
    playPureTone(800.0, 100, 0.3);
  } else if (toneType == TONE_PING_OK) {
    playPureTone(1760.0, 30, 0.25);
    delay(30);
    playPureTone(1760.0, 30, 0.25);
  } else if (toneType == TONE_PING_FAIL) {
    for (float f = 400.0; f > 180.0; f -= 12.0) {
      playPureTone(f, 8, 0.4);
    }
  }
}

// ============================================================================
// ESP-NOW RECEIVE CALLBACK  (runs on WiFi / Core 0)
// Only pushes to ring buffer — never writes to I2S directly.
// ============================================================================
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  const uint8_t *mac = info->src_addr;
#else
void onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  // 1. Differentiate control packets (PING/PONG) from raw audio packets
  if (len == 4) {
    uint32_t val = 0;
    memcpy(&val, data, 4);
    if (val == 0xDEADBEEF) {
      // PING received! Reply back with PONG.
      uint32_t pong = 0xCAFEBABE;
      esp_now_send((uint8_t *)mac, (uint8_t *)&pong, sizeof(pong));
    } else if (val == 0xCAFEBABE) {
      // PONG received!
      waitingForPong = false;
      pendingTone = TONE_PING_OK;
    }
    return;
  }

  // 2. Audio Packet handler
  if (len < 2 || len % 2 != 0) return;

  if (isTransmitting) return;

  ringPush((const int16_t *)data, len / sizeof(int16_t));
}

// ============================================================================
// HARDWARE INIT
// ============================================================================
bool initMic() {
  pdmMic.setPinsPdmRx(PIN_PDM_CLK, PIN_PDM_DATA);
  if (!pdmMic.begin(I2S_MODE_PDM_RX, SAMPLE_RATE,
                    I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("[ERROR] PDM mic init failed");
    return false;
  }
  pdmMic.setTimeout(50);
  return true;
}

bool initSpeaker() {
  i2sSpeaker.setPins(PIN_SPK_BCLK, PIN_SPK_WS, PIN_SPK_DIN);
  if (!i2sSpeaker.begin(I2S_MODE_STD, SAMPLE_RATE,
                        I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
    Serial.println("[ERROR] Speaker init failed");
    return false;
  }
  i2sSpeaker.setTimeout(50);
  return true;
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  //Serial.begin(115200);
  delay(500);

  pinMode(PIN_PTT, INPUT_PULLUP);

  // --- Read own MAC and auto-assign peer ---
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  uint8_t ownMac[6];
  esp_wifi_get_mac(WIFI_IF_STA, ownMac);  // Fills array with station MAC bytes

  Serial.printf("\nOwn MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                ownMac[0], ownMac[1], ownMac[2],
                ownMac[3], ownMac[4], ownMac[5]);

  if (memcmp(ownMac, mac1, 6) == 0) {
    peerMac = mac2;
    Serial.println("Identity : Device 1");
    Serial.printf("Peer     : Device 2 (%02X:%02X:%02X:%02X:%02X:%02X)\n",
                  mac2[0], mac2[1], mac2[2], mac2[3], mac2[4], mac2[5]);
  } else if (memcmp(ownMac, mac2, 6) == 0) {
    peerMac = mac1;
    Serial.println("Identity : Device 2");
    Serial.printf("Peer     : Device 1 (%02X:%02X:%02X:%02X:%02X:%02X)\n",
                  mac1[0], mac1[1], mac1[2], mac1[3], mac1[4], mac1[5]);
  } else {
    Serial.println("[ERROR] MAC not in list — update mac1/mac2 and reflash.");
    Serial.println("        Upload a blank sketch first, open Serial, then read the MAC printed above.");
    while (true) delay(100);
  }

  // --- ESP-NOW ---
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERROR] ESP-NOW init failed");
    while (true) delay(100);
  }
  esp_now_register_recv_cb(onDataReceived);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerMac, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("[ERROR] Failed to add peer");
    while (true) delay(100);
  }

  // --- Audio hardware ---
  if (!initMic()) {
    while (true) delay(100);
  }
  Serial.println("PDM mic   : OK");

  if (!initSpeaker()) {
    while (true) delay(100);
  }
  Serial.println("Speaker   : OK");

  // Play startup melody confirming audio system is operational!
  playStatusTone(TONE_STARTUP);
  lastPingTime = millis();

  Serial.println("\nReady — hold PTT button to talk.\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================
static int16_t micBuf[SAMPLES_PER_PACKET];
static int16_t spkBuf[SAMPLES_PER_PACKET];
static bool wasTransmitting = false;

void loop() {
  // 0. Play any safely deferred response/pending tones
  if (pendingTone != TONE_NONE) {
    int toneToPlay = pendingTone;
    pendingTone = TONE_NONE;  // Clear first to prevent race
    playStatusTone(toneToPlay);
  }

  bool pttHeld = (digitalRead(PIN_PTT) == LOW);  // Button pulls PTT pin LOW

  // ---- TRANSMIT (PTT held) ----
  if (pttHeld) {
    if (!wasTransmitting) {
      Serial.println("<< TX >>");
      ringFlush();  // Discard any buffered incoming audio before we transmit
      isTransmitting = true;
      wasTransmitting = true;
      playStatusTone(TONE_PTT_PRESS);
    }

    size_t bytesRead = pdmMic.readBytes((char *)micBuf,
                                        SAMPLES_PER_PACKET * sizeof(int16_t));
    if (bytesRead > 0) {
      int count = bytesRead / sizeof(int16_t);

      // Software gain with hard clamp
      for (int i = 0; i < count; i++) {
        int32_t s = (int32_t)micBuf[i] * GAIN;
        if (s > 32767) s = 32767;
        if (s < -32768) s = -32768;
        micBuf[i] = (int16_t)s;
      }

      esp_err_t r = esp_now_send(peerMac, (uint8_t *)micBuf, bytesRead);
      if (r != ESP_OK) Serial.printf("[TX ERROR] esp_now_send: %d\n", r);
    }

    // ---- RECEIVE (PTT released) ----
  } else {
    if (wasTransmitting) {
      Serial.println("<< RX >>");
      isTransmitting = false;
      wasTransmitting = false;
      playStatusTone(TONE_PTT_RELEASE);
      lastPingTime = millis();  // Reset ping timer
    }

    // Periodic link survey ping
    unsigned long currentMillis = millis();
    if (currentMillis - lastPingTime >= PING_INTERVAL) {
      lastPingTime = currentMillis;

      if (!isTransmitting) {
        Serial.println("[ESP-NOW SURVEY] Initiating automatic peer connection check...");

        uint32_t pingVal = 0xDEADBEEF;
        pingSentTime = millis();
        waitingForPong = true;

        esp_err_t r = esp_now_send(peerMac, (uint8_t *)&pingVal, sizeof(pingVal));
        if (r != ESP_OK) {
          Serial.printf("[ESP-NOW ERROR] Failed to send Ping, error code: %d\n", r);
          waitingForPong = false;
          pendingTone = TONE_PING_FAIL;  // SAD buzzer chime
        }
      }
    }

    // Ping timeout detection logic
    if (waitingForPong && (millis() - pingSentTime > 1500)) {
      Serial.println("[ESP-NOW TIMEOUT] Host unreachable (PONG timed out).");
      waitingForPong = false;
      pendingTone = TONE_PING_FAIL;  // SAD buzzer chime
    }

    // Drain ring buffer to speaker in fixed-size chunks
    while (ringAvailable() >= SAMPLES_PER_PACKET) {
      ringPop(spkBuf, SAMPLES_PER_PACKET);
      i2sSpeaker.write((const uint8_t *)spkBuf,
                       SAMPLES_PER_PACKET * sizeof(int16_t));
    }
    delay(2);
  }
}