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