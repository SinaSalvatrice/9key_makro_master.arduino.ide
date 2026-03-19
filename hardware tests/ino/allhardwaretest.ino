// =====================================================
#include <Keyboard.h>

// Move ActionType and Action struct to top for Arduino IDE compatibility
enum ActionType : uint8_t {
  ACT_NONE,
  ACT_KEY,
  ACT_RGB,
  ACT_MEDIA_PLACEHOLDER
};

struct Action {
  ActionType type;
  uint8_t keycode;
  uint8_t mod1;
  uint8_t mod2;
  uint8_t aux;
};

// =====================================================
// Hardware

#include <Keyboard.h>

const uint8_t ROWS = 3;
const uint8_t COLS = 3;
const uint8_t rowPins[ROWS] = {2, 4, 3};
const uint8_t colPins[COLS] = {8, 9, 27};
const uint16_t DEBOUNCE_MS = 20;

// Simple 1-9 key mapping for 3x3 matrix
const uint8_t keymap[3][3] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' }
};

bool rawState[ROWS][COLS];
bool stableState[ROWS][COLS];
uint32_t lastChange[ROWS][COLS];
bool suppressUntilRelease[ROWS][COLS];

void setAllRowsInactive() {
  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], HIGH);
  }
}

void scanMatrix(bool out[ROWS][COLS]) {
  setAllRowsInactive();
  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(5);
    for (uint8_t c = 0; c < COLS; c++) {
      out[r][c] = (digitalRead(colPins[c]) == LOW);
    }
    digitalWrite(rowPins[r], HIGH);
  }
}

void onKeyPressed(uint8_t r, uint8_t c) {
  if (suppressUntilRelease[r][c]) return;
  Keyboard.press(keymap[r][c]);
}

void onKeyReleased(uint8_t r, uint8_t c) {
  if (!suppressUntilRelease[r][c]) {
    Keyboard.release(keymap[r][c]);
  }
  suppressUntilRelease[r][c] = false;
}

void setup() {
  Serial.begin(115200);
  Keyboard.begin();
  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }
  bool initialScan[ROWS][COLS];
  scanMatrix(initialScan);
  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      rawState[r][c] = initialScan[r][c];
      stableState[r][c] = initialScan[r][c];
      lastChange[r][c] = millis();
      suppressUntilRelease[r][c] = stableState[r][c];
    }
  }
  Serial.println("RP2040 matrix keyboard ready");
}

void loop() {
  uint32_t now = millis();
  bool scanNow[ROWS][COLS];
  scanMatrix(scanNow);
  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (scanNow[r][c] != rawState[r][c]) {
        rawState[r][c] = scanNow[r][c];
        lastChange[r][c] = now;
      }
      if ((uint32_t)(now - lastChange[r][c]) >= DEBOUNCE_MS) {
        if (stableState[r][c] != rawState[r][c]) {
          stableState[r][c] = rawState[r][c];
          if (stableState[r][c]) {
            onKeyPressed(r, c);
          } else {
            onKeyReleased(r, c);
          }
        }
      }
    }
  }
}
}
