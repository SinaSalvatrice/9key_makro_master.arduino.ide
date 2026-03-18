#include <Keyboard.h>

// =====================================================
// Hardware
// =====================================================

const uint8_t ROWS = 3;
const uint8_t COLS = 3;

// Anpassen an deine Verdrahtung
const uint8_t rowPins[ROWS] = {2, 4, 3};
const uint8_t colPins[COLS] = {5, 6, 7};

// separater Selector-Button gegen GND
const uint8_t SELECTOR_PIN = 10;

const uint16_t DEBOUNCE_MS = 20;

// =====================================================
// Layer
// =====================================================

enum Layer : uint8_t {
  L_BASE = 0,
  L_NAV,
  L_EDIT,
  L_MEDIA,
  L_FN,
  L_RGB,
  L_EXTRA1,
  L_EXTRA2,
  L_EXTRA3,
  LAYER_COUNT
};

Layer activeLayer = L_BASE;

const char* layerName(Layer l) {
  switch (l) {
    case L_BASE:   return "BASE";
    case L_NAV:    return "NAV";
    case L_EDIT:   return "EDIT";
    case L_MEDIA:  return "MEDIA";
    case L_FN:     return "FN";
    case L_RGB:    return "RGB";
    case L_EXTRA1: return "EXTRA1";
    case L_EXTRA2: return "EXTRA2";
    case L_EXTRA3: return "EXTRA3";
    default:       return "?";
  }
}

// =====================================================
// Action model
// =====================================================

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

#define A_NONE            {ACT_NONE, 0, 0, 0, 0}
#define A_KEY(k,m1,m2)    {ACT_KEY, (uint8_t)(k), (uint8_t)(m1), (uint8_t)(m2), 0}
#define A_RGB(id)         {ACT_RGB, 0, 0, 0, (uint8_t)(id)}
#define A_MEDIA(id)       {ACT_MEDIA_PLACEHOLDER, 0, 0, 0, (uint8_t)(id)}

// =====================================================
// Media placeholder IDs
// =====================================================

enum MediaPlaceholder : uint8_t {
  MP_PREV,
  MP_SELECT,
  MP_NEXT,
  MP_REW,
  MP_PLAY,
  MP_FF,
  MP_STOP
};

// =====================================================
// Keymap
// =====================================================

const Action keymap[LAYER_COUNT][ROWS][COLS] = {
  // L_BASE
  {
    { A_KEY(KEY_TAB,         KEY_LEFT_GUI, 0), A_KEY(KEY_UP_ARROW,    0, 0), A_KEY(KEY_TAB,         KEY_LEFT_ALT, 0) },
    { A_KEY(KEY_LEFT_ARROW,  0, 0),            A_KEY(KEY_RETURN,      0, 0), A_KEY(KEY_RIGHT_ARROW, 0, 0)            },
    { A_KEY('z',             KEY_LEFT_CTRL, 0),A_KEY(KEY_DOWN_ARROW,  0, 0), A_KEY('r',             KEY_LEFT_CTRL, 0)}
  },

  // L_NAV (absichtlich doppelt)
  {
    { A_KEY(KEY_TAB,         KEY_LEFT_GUI, 0), A_KEY(KEY_UP_ARROW,    0, 0), A_KEY(KEY_TAB,         KEY_LEFT_ALT, 0) },
    { A_KEY(KEY_LEFT_ARROW,  0, 0),            A_KEY(KEY_RETURN,      0, 0), A_KEY(KEY_RIGHT_ARROW, 0, 0)            },
    { A_KEY('z',             KEY_LEFT_CTRL, 0),A_KEY(KEY_DOWN_ARROW,  0, 0), A_KEY('r',             KEY_LEFT_CTRL, 0)}
  },

  // L_EDIT
  {
    { A_KEY('a', KEY_LEFT_CTRL, 0), A_KEY('c', KEY_LEFT_CTRL, 0),       A_KEY('v', KEY_LEFT_CTRL, 0)       },
    { A_KEY('x', KEY_LEFT_CTRL, 0), A_KEY(KEY_RETURN, KEY_LEFT_CTRL, 0),A_NONE                              },
    { A_KEY('z', KEY_LEFT_CTRL, KEY_LEFT_SHIFT), A_KEY(' ', 0, 0),       A_KEY(KEY_BACKSPACE, 0, 0)        }
  },

  // L_MEDIA
  // Ohne TinyUSB hier nur Platzhalter.
  // Du kannst hier später app-spezifische Shortcuts draus machen.
  {
    { A_MEDIA(MP_PREV),   A_MEDIA(MP_SELECT), A_MEDIA(MP_NEXT) },
    { A_MEDIA(MP_REW),    A_MEDIA(MP_PLAY),   A_MEDIA(MP_FF)   },
    { A_KEY(KEY_DOWN_ARROW, 0, 0), A_MEDIA(MP_STOP), A_KEY(KEY_UP_ARROW, 0, 0) }
  },

  // L_FN
  {
    { A_KEY(KEY_F13, 0, 0), A_KEY(KEY_F14, 0, 0), A_KEY(KEY_F15, 0, 0) },
    { A_KEY(KEY_F16, 0, 0), A_KEY(KEY_F17, 0, 0), A_KEY(KEY_F18, 0, 0) },
    { A_KEY(KEY_F19, 0, 0), A_KEY(KEY_F20, 0, 0), A_KEY(KEY_F21, 0, 0) }
  },

  // L_RGB
  {
    { A_RGB(0), A_RGB(1), A_RGB(2) },
    { A_RGB(3), A_RGB(4), A_RGB(5) },
    { A_RGB(6), A_RGB(7), A_RGB(8) }
  },

  // L_EXTRA1
  {
    { A_NONE, A_NONE, A_NONE },
    { A_NONE, A_NONE, A_NONE },
    { A_NONE, A_NONE, A_NONE }
  },

  // L_EXTRA2
  {
    { A_NONE, A_NONE, A_NONE },
    { A_NONE, A_NONE, A_NONE },
    { A_NONE, A_NONE, A_NONE }
  },

  // L_EXTRA3
  {
    { A_NONE, A_NONE, A_NONE },
    { A_NONE, A_NONE, A_NONE },
    { A_NONE, A_NONE, A_NONE }
  }
};

// =====================================================
// State
// =====================================================

bool rawState[ROWS][COLS];
bool stableState[ROWS][COLS];
uint32_t lastChange[ROWS][COLS];
bool suppressUntilRelease[ROWS][COLS];

bool rawSelector = false;
bool stableSelector = false;
uint32_t selectorLastChange = 0;

// =====================================================
// Helpers
// =====================================================

bool readSelectorPressed() {
  return digitalRead(SELECTOR_PIN) == LOW;
}

void setAllRowsInactive() {
  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], HIGH);
  }
}

// Matrix-Annahme:
// rows = OUTPUT
// cols = INPUT_PULLUP
// aktive row = LOW
// gedrückte Taste => col liest LOW
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

void pressKeyboardAction(const Action &a) {
  if (a.mod1) Keyboard.press(a.mod1);
  if (a.mod2) Keyboard.press(a.mod2);
  if (a.keycode) Keyboard.press(a.keycode);
}

void releaseKeyboardAction(const Action &a) {
  if (a.keycode) Keyboard.release(a.keycode);
  if (a.mod2) Keyboard.release(a.mod2);
  if (a.mod1) Keyboard.release(a.mod1);
}

void handleRgbAction(uint8_t id) {
  Serial.print("RGB action: ");
  Serial.println(id);
  // hier später deine RGB-Funktion
}

void handleMediaPlaceholder(uint8_t id) {
  Serial.print("MEDIA placeholder: ");
  Serial.println(id);

  // Hier kannst du später pro App normale Tastatur-Shortcuts schicken.
  // Beispielidee:
  // MP_PLAY -> Space
  // MP_NEXT -> Ctrl+Right
  // MP_PREV -> Ctrl+Left
  //
  // Aktuell absichtlich nur Debug.
}

void releaseAllKeyboard() {
  Keyboard.releaseAll();
}

void enterSelectorMode() {
  releaseAllKeyboard();

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (stableState[r][c]) {
        suppressUntilRelease[r][c] = true;
      }
    }
  }

  Serial.println("Selector ON");
}

void leaveSelectorMode() {
  releaseAllKeyboard();
  Serial.println("Selector OFF");
}

// =====================================================
// Events
// =====================================================

void onKeyPressed(uint8_t r, uint8_t c) {
  if (stableSelector) {
    uint8_t idx = r * COLS + c;
    if (idx < LAYER_COUNT) {
      activeLayer = (Layer)idx;
      suppressUntilRelease[r][c] = true;

      Serial.print("Layer -> ");
      Serial.println(layerName(activeLayer));
    }
    return;
  }

  if (suppressUntilRelease[r][c]) return;

  const Action &a = keymap[activeLayer][r][c];

  switch (a.type) {
    case ACT_KEY:
      pressKeyboardAction(a);
      break;

    case ACT_RGB:
      suppressUntilRelease[r][c] = true;
      handleRgbAction(a.aux);
      break;

    case ACT_MEDIA_PLACEHOLDER:
      suppressUntilRelease[r][c] = true;
      handleMediaPlaceholder(a.aux);
      break;

    case ACT_NONE:
    default:
      break;
  }
}

void onKeyReleased(uint8_t r, uint8_t c) {
  const Action &a = keymap[activeLayer][r][c];

  if (!stableSelector && !suppressUntilRelease[r][c]) {
    if (a.type == ACT_KEY) {
      releaseKeyboardAction(a);
    }
  }

  suppressUntilRelease[r][c] = false;
}

// =====================================================
// Setup / Loop
// =====================================================

void setup() {
  Serial.begin(115200);
  Keyboard.begin();

  pinMode(SELECTOR_PIN, INPUT_PULLUP);

  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }

  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  rawSelector = readSelectorPressed();
  stableSelector = rawSelector;
  selectorLastChange = millis();

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
  Serial.print("Layer -> ");
  Serial.println(layerName(activeLayer));
}

void loop() {
  uint32_t now = millis();

  // Selector
  bool selectorReading = readSelectorPressed();

  if (selectorReading != rawSelector) {
    rawSelector = selectorReading;
    selectorLastChange = now;
  }

  if ((now - selectorLastChange) > DEBOUNCE_MS) {
    if (stableSelector != rawSelector) {
      stableSelector = rawSelector;

      if (stableSelector) {
        enterSelectorMode();
      } else {
        leaveSelectorMode();
      }
    }
  }

  // Matrix
  bool scanNow[ROWS][COLS];
  scanMatrix(scanNow);

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (scanNow[r][c] != rawState[r][c]) {
        rawState[r][c] = scanNow[r][c];
        lastChange[r][c] = now;
      }

      if ((now - lastChange[r][c]) > DEBOUNCE_MS) {
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
