#include <Keyboard.h>

// =====================================================
// RP2040 / Arduino - 3x3 direct-wired keypad example
// Buttons go from pin -> GND
// Pin numbers an deine Verdrahtung anpassen
// =====================================================

const uint8_t ROWS = 3;
const uint8_t COLS = 3;

// Beispiel-Pins für 9 einzelne Taster (kein Matrixscan)
const uint8_t keyPins[ROWS][COLS] = {
  {2, 3, 4},
  {5, 6, 7},
  {8, 9, 10}
};

const uint16_t DEBOUNCE_MS = 20;

// =====================================================
// Layers
// =====================================================

enum Layer : uint8_t {
  _BASE,
  _NAV,
  _EDIT,
  _MEDIA,
  _FN,
  _RGB,
  _SELECT,
  _LAYER_COUNT
};

Layer currentLayer = _BASE;

// =====================================================
// Custom action system
// =====================================================

enum ActionType : uint8_t {
  ACT_NONE,
  ACT_KEY,        // normal press/release key
  ACT_CHORD,      // hold modifiers + key while pressed
  ACT_TO_LAYER,   // persistent layer switch
  ACT_MEDIA,      // placeholder
  ACT_RGB,        // placeholder
  ACT_FKEY_EXT    // placeholder / optional
};

struct Action {
  ActionType type;
  uint8_t a;
  uint8_t b;
  uint8_t c;
};

// Macros zum bequemen Definieren
#define NO_ACT          {ACT_NONE, 0, 0, 0}
#define KEY_ACT(k)      {ACT_KEY, (uint8_t)(k), 0, 0}
#define CHORD(k,m1,m2)  {ACT_CHORD, (uint8_t)(k), (uint8_t)(m1), (uint8_t)(m2)}
#define TO_ACT(layer)   {ACT_TO_LAYER, (uint8_t)(layer), 0, 0}
#define MEDIA_ACT(id)   {ACT_MEDIA, (uint8_t)(id), 0, 0}
#define RGB_ACT(id)     {ACT_RGB, (uint8_t)(id), 0, 0}
#define FKEY_ACT(idx)   {ACT_FKEY_EXT, (uint8_t)(idx), 0, 0}   // idx 0 => F13

// =====================================================
// Placeholder IDs
// =====================================================

enum MediaId : uint8_t {
  M_MPRV,
  M_MSEL,
  M_MNXT,
  M_MRWD,
  M_MPLY,
  M_MFFD,
  M_MSTP
};

enum RgbId : uint8_t {
  RGB_SPDU,
  RGB_SPDD,
  RGB_TOGG,
  RGB_HUEU,
  RGB_HUED,
  RGB_VALU,
  RGB_SATU,
  RGB_SATD,
  RGB_VALD
};

// =====================================================
// Keymap
// =====================================================

const Action keymap[_LAYER_COUNT][ROWS][COLS] = {

  // [_BASE] = LAYOUT(
  //   LGUI(KC_TAB),  KC_UP,    LALT(KC_TAB),
  //   KC_LEFT,       KC_ENT,   KC_RGHT,
  //   LCTL(KC_Z),    KC_DOWN,  LCTL(KC_R)
  // )
  [_BASE] = {
    { CHORD(KEY_TAB, KEY_LEFT_GUI, 0),   KEY_ACT(KEY_UP_ARROW),         CHORD(KEY_TAB, KEY_LEFT_ALT, 0) },
    { KEY_ACT(KEY_LEFT_ARROW),           KEY_ACT(KEY_RETURN),           KEY_ACT(KEY_RIGHT_ARROW)         },
    { CHORD('z', KEY_LEFT_CTRL, 0),      KEY_ACT(KEY_DOWN_ARROW),       CHORD('r', KEY_LEFT_CTRL, 0)    }
  },

  // [_NAV] gleich wie BASE
  [_NAV] = {
    { CHORD(KEY_TAB, KEY_LEFT_GUI, 0),   KEY_ACT(KEY_UP_ARROW),         CHORD(KEY_TAB, KEY_LEFT_ALT, 0) },
    { KEY_ACT(KEY_LEFT_ARROW),           KEY_ACT(KEY_RETURN),           KEY_ACT(KEY_RIGHT_ARROW)         },
    { CHORD('z', KEY_LEFT_CTRL, 0),      KEY_ACT(KEY_DOWN_ARROW),       CHORD('r', KEY_LEFT_CTRL, 0)    }
  },

  // [_EDIT] = LAYOUT(
  //   LCTL(KC_A),         LCTL(KC_C),   LCTL(KC_V),
  //   LCTL(KC_X),         LCTL(KC_ENT), KC_NO,
  //   LCTL(LSFT(KC_Z)),   KC_SPC,       KC_BSPC
  // )
  [_EDIT] = {
    { CHORD('a', KEY_LEFT_CTRL, 0),      CHORD('c', KEY_LEFT_CTRL, 0),        CHORD('v', KEY_LEFT_CTRL, 0)       },
    { CHORD('x', KEY_LEFT_CTRL, 0),      CHORD(KEY_RETURN, KEY_LEFT_CTRL, 0), NO_ACT                               },
    { CHORD('z', KEY_LEFT_CTRL, KEY_LEFT_SHIFT), KEY_ACT(' '),                 KEY_ACT(KEY_BACKSPACE)              }
  },

  // [_MEDIA]
  [_MEDIA] = {
    { MEDIA_ACT(M_MPRV),  MEDIA_ACT(M_MSEL),  MEDIA_ACT(M_MNXT) },
    { MEDIA_ACT(M_MRWD),  MEDIA_ACT(M_MPLY),  MEDIA_ACT(M_MFFD) },
    { KEY_ACT(KEY_DOWN_ARROW), MEDIA_ACT(M_MSTP), KEY_ACT(KEY_UP_ARROW) }
  },

  // [_FN] = F13 ... F21
  [_FN] = {
    { FKEY_ACT(0), FKEY_ACT(1), FKEY_ACT(2) },
    { FKEY_ACT(3), FKEY_ACT(4), FKEY_ACT(5) },
    { FKEY_ACT(6), FKEY_ACT(7), FKEY_ACT(8) }
  },

  // [_RGB]
  [_RGB] = {
    { RGB_ACT(RGB_SPDU), RGB_ACT(RGB_SPDD), RGB_ACT(RGB_TOGG) },
    { RGB_ACT(RGB_HUEU), RGB_ACT(RGB_HUED), RGB_ACT(RGB_VALU) },
    { RGB_ACT(RGB_SATU), RGB_ACT(RGB_SATD), RGB_ACT(RGB_VALD) }
  },

  // [_SELECT] = LAYOUT(
  //   TO(1),  TO(2),  TO(3),
  //   TO(4),  TO(0),  KC_NO,
  //   KC_NO,  KC_NO,  KC_NO
  // )
  [_SELECT] = {
    { TO_ACT(_NAV),   TO_ACT(_EDIT),  TO_ACT(_MEDIA) },
    { TO_ACT(_FN),    TO_ACT(_BASE),  NO_ACT         },
    { NO_ACT,         NO_ACT,         NO_ACT         }
  }
};

// =====================================================
// Debounce / state
// =====================================================

bool rawState[ROWS][COLS];
bool stableState[ROWS][COLS];
uint32_t lastChange[ROWS][COLS];

// =====================================================
// Debug helpers
// =====================================================

const char* layerName(Layer l) {
  switch (l) {
    case _BASE:   return "BASE";
    case _NAV:    return "NAV";
    case _EDIT:   return "EDIT";
    case _MEDIA:  return "MEDIA";
    case _FN:     return "FN";
    case _RGB:    return "RGB";
    case _SELECT: return "SELECT";
    default:      return "?";
  }
}

// =====================================================
// Placeholder handlers
// =====================================================

void handleMedia(uint8_t id) {
  Serial.print("MEDIA action: ");
  Serial.println(id);
  // Hier später echte Consumer-Control einsetzen
  // z.B. mit TinyUSB / HID-Consumer
}

void handleRgb(uint8_t id) {
  Serial.print("RGB action: ");
  Serial.println(id);
  // Hier später deine RGB-Funktionen aufrufen
}

void handleExtendedFKey(uint8_t idx) {
  Serial.print("F");
  Serial.println(13 + idx);

  // Falls dein Core KEY_F13 ... KEY_F21 unterstützt,
  // kannst du das hier direkt ersetzen.
  //
  // Beispiel:
  // Keyboard.press(KEY_F13 + idx);
  // delay(5);
  // Keyboard.release(KEY_F13 + idx);
}

// =====================================================
// Action execution
// =====================================================

void pressAction(const Action &act) {
  switch (act.type) {
    case ACT_NONE:
      break;

    case ACT_KEY:
      Keyboard.press(act.a);
      break;

    case ACT_CHORD:
      if (act.b) Keyboard.press(act.b);
      if (act.c) Keyboard.press(act.c);
      Keyboard.press(act.a);
      break;

    case ACT_TO_LAYER:
      currentLayer = (Layer)act.a;
      Serial.print("Layer -> ");
      Serial.println(layerName(currentLayer));
      break;

    case ACT_MEDIA:
      handleMedia(act.a);
      break;

    case ACT_RGB:
      handleRgb(act.a);
      break;

    case ACT_FKEY_EXT:
      handleExtendedFKey(act.a);
      break;
  }
}

void releaseAction(const Action &act) {
  switch (act.type) {
    case ACT_NONE:
      break;

    case ACT_KEY:
      Keyboard.release(act.a);
      break;

    case ACT_CHORD:
      Keyboard.release(act.a);
      if (act.c) Keyboard.release(act.c);
      if (act.b) Keyboard.release(act.b);
      break;

    case ACT_TO_LAYER:
    case ACT_MEDIA:
    case ACT_RGB:
    case ACT_FKEY_EXT:
      // one-shot / placeholder
      break;
  }
}

// =====================================================
// Key events
// =====================================================

void onKeyPressed(uint8_t row, uint8_t col) {
  const Action &act = keymap[currentLayer][row][col];
  pressAction(act);
}

void onKeyReleased(uint8_t row, uint8_t col) {
  const Action &act = keymap[currentLayer][row][col];
  releaseAction(act);
}

// =====================================================
// Setup / loop
// =====================================================

void setup() {
  Serial.begin(115200);
  Keyboard.begin();

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      pinMode(keyPins[r][c], INPUT_PULLUP); // Taster gegen GND
      bool pressed = (digitalRead(keyPins[r][c]) == LOW);
      rawState[r][c] = pressed;
      stableState[r][c] = pressed;
      lastChange[r][c] = 0;
    }
  }

  Serial.println("3x3 RP2040 keymap loaded");
  Serial.print("Layer -> ");
  Serial.println(layerName(currentLayer));
}

void loop() {
  uint32_t now = millis();

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      bool reading = (digitalRead(keyPins[r][c]) == LOW);

      if (reading != rawState[r][c]) {
        rawState[r][c] = reading;
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