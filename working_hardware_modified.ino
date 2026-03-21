// =============================================================
// 9-KEY MACRO MASTER  –  Full Macropad Firmware
// Board  : Waveshare RP2040 Zero
// Matrix : 3×3 (9 keys), rows GP2/3/4, cols GP5/6/7
// Encoder: A=GP8, B=GP9, BTN=GP10
// RGB    : GP13, 10 NeoPixels (9 keys + 1 indicator)
// OLED   : SSD1306 128×64, I2C SDA=GP0 SCL=GP1
//
// Board package required : earlephilhower/arduino-pico
//   https://github.com/earlephilhower/arduino-pico
// Libraries required     : Adafruit_SSD1306, Adafruit_GFX,
//                          Adafruit_NeoPixel
//
// Media keys (MEDIA layer & encoder vol):
//   Requires ConsumerKeyboard from the arduino-pico package.
//   Set USE_CONSUMER_KEYBOARD 1 below if available.
//   If your build fails, set it to 0 (media key presses will
//   be silently skipped until you enable it).
// =============================================================

#define USE_CONSUMER_KEYBOARD 0   // set to 1 when ConsumerKeyboard.h is available

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Keyboard.h>
#include <Mouse.h>
#if USE_CONSUMER_KEYBOARD
  #include <ConsumerKeyboard.h>
#endif

// =============================================================
// PIN CONFIGURATION
// =============================================================
#define SDA_PIN    0
#define SCL_PIN    1
#define RGB_PIN   13
#define NUM_PIXELS 10   // 9 key LEDs + 1 encoder-button indicator

const uint8_t ROWS = 3;
const uint8_t COLS = 3;
const uint8_t rowPins[ROWS] = {2, 3, 4};
const uint8_t colPins[COLS] = {5, 6, 7};
const uint8_t ENC_A   =  8;
const uint8_t ENC_B   =  9;
const uint8_t ENC_BTN = 10;

// =============================================================
// DISPLAY
// =============================================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_ADDR    0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// =============================================================
// LAYER DEFINITIONS
// =============================================================
#define LAYER_BASE   0
#define LAYER_NAV    1
#define LAYER_EDIT   2
#define LAYER_MEDIA  3
#define LAYER_FN     4
#define LAYER_RGB    5
#define LAYER_SELECT 6
#define NUM_LAYERS   7

uint8_t currentLayer = LAYER_BASE;
bool    selectMode   = false;   // true while encoder button held

// Short names used on OLED (4 chars for grid cells)
const char* const layerNames[NUM_LAYERS] = {
  "BASE", "NAV ", "EDIT", "MEDI", "FN  ", "RGB ", "SLCT"
};

// =============================================================
// KEY ACTION TYPES
// =============================================================
#define ACT_NONE    0   // KC_NO – do nothing
#define ACT_KEY     1   // keyboard key + optional modifiers
#define ACT_MEDIA   2   // Consumer Control key
#define ACT_LAYER   3   // switch to layer (TO)
#define ACT_RGB_CTL 4   // RGB parameter change

// Modifier bitmasks for ACT_KEY
#define MOD_CTRL   0x01
#define MOD_SHIFT  0x02
#define MOD_ALT    0x04
#define MOD_GUI    0x08

// Consumer-control sub-codes (USB HID Usage Tables, Consumer Page 0x0C)
#define MK_PREV    0x01   // Scan Previous Track  0x00B6
#define MK_SEL     0x02   // Media Select         0x0183
#define MK_NEXT    0x03   // Scan Next Track      0x00B5
#define MK_REW     0x04   // Rewind               0x00B4
#define MK_PLAY    0x05   // Play/Pause           0x00CD
#define MK_FFAST   0x06   // Fast Forward         0x00B3
#define MK_STOP    0x07   // Stop                 0x00B7
#define MK_VOLUP   0x08   // Volume Increment     0x00E9
#define MK_VOLDN   0x09   // Volume Decrement     0x00EA

// RGB control sub-codes
#define RGBC_SPD_UP 0x00
#define RGBC_SPD_DN 0x01
#define RGBC_TOG    0x02
#define RGBC_HUE_UP 0x03
#define RGBC_HUE_DN 0x04
#define RGBC_VAL_UP 0x05
#define RGBC_VAL_DN 0x06
#define RGBC_SAT_UP 0x07
#define RGBC_SAT_DN 0x08

// F13-F21 (arduino-pico extends Keyboard.h with these;
// values continue sequentially after KEY_F12 = 0xCD)
#ifndef KEY_F13
  #define KEY_F13 0xCE
  #define KEY_F14 0xCF
  #define KEY_F15 0xD0
  #define KEY_F16 0xD1
  #define KEY_F17 0xD2
  #define KEY_F18 0xD3
  #define KEY_F19 0xD4
  #define KEY_F20 0xD5
  #define KEY_F21 0xD6
#endif

// =============================================================
// KEY MAP
// =============================================================
struct KeyAction {
  uint8_t  type;
  uint16_t code;   // keycode / layer index / control code
  uint8_t  mods;   // modifier bitmask (ACT_KEY only)
};

const KeyAction keyMap[NUM_LAYERS][ROWS * COLS] = {
  // -- LAYER_BASE (0) -------------------------------------------
  {
    {ACT_KEY, KEY_TAB,          MOD_GUI  },  // r0c0  Win+Tab
    {ACT_KEY, KEY_UP_ARROW,     0        },  // r0c1  Up
    {ACT_KEY, KEY_TAB,          MOD_ALT  },  // r0c2  Alt+Tab
    {ACT_KEY, KEY_LEFT_ARROW,   0        },  // r1c0  Left
    {ACT_KEY, KEY_RETURN,       0        },  // r1c1  Enter
    {ACT_KEY, KEY_RIGHT_ARROW,  0        },  // r1c2  Right
    {ACT_KEY, 'z',              MOD_CTRL },  // r2c0  Ctrl+Z
    {ACT_KEY, KEY_DOWN_ARROW,   0        },  // r2c1  Down
    {ACT_KEY, 'r',              MOD_CTRL },  // r2c2  Ctrl+R
  },
  // -- LAYER_NAV (1) --------------------------------------------
  {
    {ACT_KEY, KEY_TAB,          MOD_GUI  },
    {ACT_KEY, KEY_UP_ARROW,     0        },
    {ACT_KEY, KEY_TAB,          MOD_ALT  },
    {ACT_KEY, KEY_LEFT_ARROW,   0        },
    {ACT_KEY, KEY_RETURN,       0        },
    {ACT_KEY, KEY_RIGHT_ARROW,  0        },
    {ACT_KEY, 'z',              MOD_CTRL },
    {ACT_KEY, KEY_DOWN_ARROW,   0        },
    {ACT_KEY, 'r',              MOD_CTRL },
  },
  // -- LAYER_EDIT (2) -------------------------------------------
  {
    {ACT_KEY, 'a',              MOD_CTRL             },  // Ctrl+A
    {ACT_KEY, 'c',              MOD_CTRL             },  // Ctrl+C
    {ACT_KEY, 'v',              MOD_CTRL             },  // Ctrl+V
    {ACT_KEY, 'x',              MOD_CTRL             },  // Ctrl+X
    {ACT_KEY, KEY_RETURN,       MOD_CTRL             },  // Ctrl+Enter
    {ACT_NONE, 0,               0                    },  // KC_NO
    {ACT_KEY, 'z',              MOD_CTRL | MOD_SHIFT },  // Ctrl+Shift+Z
    {ACT_KEY, ' ',              0                    },  // Space
    {ACT_KEY, KEY_BACKSPACE,    0                    },  // Backspace
  },
  // -- LAYER_MEDIA (3) ------------------------------------------
  {
    {ACT_MEDIA, MK_PREV,  0},   // Prev track
    {ACT_MEDIA, MK_SEL,   0},   // Media select
    {ACT_MEDIA, MK_NEXT,  0},   // Next track
    {ACT_MEDIA, MK_REW,   0},   // Rewind
    {ACT_MEDIA, MK_PLAY,  0},   // Play/Pause
    {ACT_MEDIA, MK_FFAST, 0},   // Fast-forward
    {ACT_KEY,   KEY_DOWN_ARROW, 0},
    {ACT_MEDIA, MK_STOP,  0},   // Stop
    {ACT_KEY,   KEY_UP_ARROW,   0},
  },
  // -- LAYER_FN (4) ---------------------------------------------
  {
    {ACT_KEY, KEY_F13, 0},
    {ACT_KEY, KEY_F14, 0},
    {ACT_KEY, KEY_F15, 0},
    {ACT_KEY, KEY_F16, 0},
    {ACT_KEY, KEY_F17, 0},
    {ACT_KEY, KEY_F18, 0},
    {ACT_KEY, KEY_F19, 0},
    {ACT_KEY, KEY_F20, 0},
    {ACT_KEY, KEY_F21, 0},
  },
  // -- LAYER_RGB (5) --------------------------------------------
  {
    {ACT_RGB_CTL, RGBC_SPD_UP, 0},
    {ACT_RGB_CTL, RGBC_SPD_DN, 0},
    {ACT_RGB_CTL, RGBC_TOG,    0},
    {ACT_RGB_CTL, RGBC_HUE_UP, 0},
    {ACT_RGB_CTL, RGBC_HUE_DN, 0},
    {ACT_RGB_CTL, RGBC_VAL_UP, 0},
    {ACT_RGB_CTL, RGBC_SAT_UP, 0},
    {ACT_RGB_CTL, RGBC_SAT_DN, 0},
    {ACT_RGB_CTL, RGBC_VAL_DN, 0},
  },
  // -- LAYER_SELECT (6) -----------------------------------------
  // Grid positions:
  //  (0,0)=NAV  (0,1)=EDIT (0,2)=MEDIA
  //  (1,0)=FN   (1,1)=BASE (1,2)=---
  //  (2,x)=---
  {
    {ACT_LAYER, 1, 0},
    {ACT_LAYER, 2, 0},
    {ACT_LAYER, 3, 0},
    {ACT_LAYER, 4, 0},
    {ACT_LAYER, 0, 0},
    {ACT_NONE,  0, 0},
    {ACT_NONE,  0, 0},
    {ACT_NONE,  0, 0},
    {ACT_NONE,  0, 0},
  },
};

// Layer index at each SELECT-grid cell (-1 = empty)
const int8_t selectGridLayer[ROWS * COLS] = { 1, 2, 3, 4, 0, -1, -1, -1, -1 };

// =============================================================
// MATRIX STATE + DEBOUNCE
// =============================================================
#define DEBOUNCE_MS 20

bool     rawState[ROWS][COLS]    = {};
bool     stableState[ROWS][COLS] = {};
uint32_t lastChangeMs[ROWS][COLS]= {};

uint8_t lastPressRow = 255;   // 255 = no key pressed yet
uint8_t lastPressCol = 255;

// =============================================================
// ENCODER STATE
// =============================================================
int      lastEncA      = HIGH;
int      lastEncBtn    = HIGH;
uint32_t encBtnChangeMs= 0;     // for non-blocking button debounce
int      rawEncBtn     = HIGH;

// =============================================================
// RGB ANIMATION STATE
// =============================================================
bool     rgbEnabled = true;
uint8_t  rgbVal     = 150;   // brightness  0-255
uint8_t  rgbSat     = 240;   // saturation  0-255
uint8_t  rgbSpeed   =   3;   // 1 (slow) - 10 (fast)
uint32_t animTick   =   0;
uint32_t lastRgbMs  =   0;

// Per-layer hue ranges for the wandering gradient (NeoPixel HSV 0-65535)
//   0     = red,  10922 = yellow, 21845 = green,
//   32768 = cyan, 43690 = blue,   54613 = magenta/pink
const uint16_t layerHue1[NUM_LAYERS] = {
      0,   // BASE  : rainbow (special-cased)
  43690,   // NAV   : blue   (~240 deg)
  21845,   // EDIT  : green  (~120 deg)
  54613,   // MEDIA : pink   (~300 deg)
   5461,   // FN    : orange (~30  deg)
  49151,   // RGB   : purple (~270 deg)
      0,   // SELECT: red
};
const uint16_t layerHue2[NUM_LAYERS] = {
  65535,   // BASE  : (unused)
  32768,   // NAV   : cyan    (~180 deg)
  10922,   // EDIT  : yellow  (~60  deg)
  10922,   // MEDIA : yellow  (~60  deg)
  10922,   // FN    : yellow  (~60  deg)
   5461,   // RGB   : orange  (~30  deg)
      0,   // SELECT: red
};

// =============================================================
// HID - KEYBOARD
// =============================================================
static void pressKeyWithMods(uint16_t code, uint8_t mods) {
  if (mods & MOD_GUI)   Keyboard.press(KEY_LEFT_GUI);
  if (mods & MOD_ALT)   Keyboard.press(KEY_LEFT_ALT);
  if (mods & MOD_SHIFT) Keyboard.press(KEY_LEFT_SHIFT);
  if (mods & MOD_CTRL)  Keyboard.press(KEY_LEFT_CTRL);
  Keyboard.press(code);
}

// Tap a single key (press + 10ms + releaseAll).
static void tapKey(uint16_t code) {
  Keyboard.press(code); delay(10); Keyboard.releaseAll();
}

// Tap a key with modifiers.
static void tapKeyWithMods(uint16_t code, uint8_t mods) {
  pressKeyWithMods(code, mods); delay(10); Keyboard.releaseAll();
}

// =============================================================
// HID - CONSUMER CONTROL (MEDIA KEYS)
// =============================================================
static void sendMediaKey(uint8_t mk) {
#if USE_CONSUMER_KEYBOARD
  uint16_t usage = 0;
  switch (mk) {
    case MK_PREV:  usage = 0x00B6; break;
    case MK_SEL:   usage = 0x0183; break;
    case MK_NEXT:  usage = 0x00B5; break;
    case MK_REW:   usage = 0x00B4; break;
    case MK_PLAY:  usage = 0x00CD; break;
    case MK_FFAST: usage = 0x00B3; break;
    case MK_STOP:  usage = 0x00B7; break;
    case MK_VOLUP: usage = 0x00E9; break;
    case MK_VOLDN: usage = 0x00EA; break;
  }
  if (usage) {
    ConsumerKeyboard.press(usage);
    delay(10);
    ConsumerKeyboard.release();
  }
#else
  (void)mk;   // media keys disabled - set USE_CONSUMER_KEYBOARD 1 to enable
#endif
}

// =============================================================
// RGB CONTROL ACTIONS
// =============================================================
static void handleRgbControl(uint8_t code) {
  switch (code) {
    case RGBC_SPD_UP: rgbSpeed = (uint8_t)constrain(rgbSpeed + 1,  1, 10); break;
    case RGBC_SPD_DN: rgbSpeed = (uint8_t)constrain(rgbSpeed - 1,  1, 10); break;
    case RGBC_TOG:    rgbEnabled = !rgbEnabled;                              break;
    case RGBC_VAL_UP: rgbVal = (uint8_t)constrain(rgbVal + 20,   0, 255);   break;
    case RGBC_VAL_DN: rgbVal = (uint8_t)constrain(rgbVal - 20,   0, 255);   break;
    case RGBC_SAT_UP: rgbSat = (uint8_t)constrain(rgbSat + 20,   0, 255);   break;
    case RGBC_SAT_DN: rgbSat = (uint8_t)constrain(rgbSat - 20,   0, 255);   break;
    case RGBC_HUE_UP: break;   // hue shift is visual-only, baked into animTick
    case RGBC_HUE_DN: break;
  }
}

// =============================================================
// KEY ACTION EXECUTION
// =============================================================
static void executeKeyAction(const KeyAction& ka, bool pressed) {
  switch (ka.type) {
    case ACT_NONE:
      break;
    case ACT_KEY:
      if (pressed) {
        pressKeyWithMods(ka.code, ka.mods);
      } else {
        Keyboard.releaseAll();
      }
      break;
    case ACT_MEDIA:
      if (pressed) sendMediaKey((uint8_t)ka.code);
      break;
    case ACT_LAYER:
      if (pressed) {
        currentLayer = (uint8_t)ka.code;
        selectMode   = false;   // exit select after choosing
      }
      break;
    case ACT_RGB_CTL:
      if (pressed) handleRgbControl((uint8_t)ka.code);
      break;
  }
}

// =============================================================
// RGB ANIMATION
// =============================================================

// Breathing brightness: returns a value in [30..rgbVal] for phase 0-255.
static uint8_t breathe(uint8_t phase) {
  float s = sinf(phase * 2.0f * (float)M_PI / 256.0f);  // -1 .. 1
  int16_t v = (int16_t)(s * 100.0f) + 100;             //  0..200
  v = constrain(v, 0, 200);
  return (uint8_t)map(v, 0, 200, 30, rgbVal);
}

static void updateRgb() {
  if (!rgbEnabled) {
    pixels.clear();
    pixels.show();
    return;
  }

  uint8_t layer = selectMode ? LAYER_SELECT : currentLayer;

  // -- SELECT: red double-blink (blink-blink, long pause) -------
  if (layer == LAYER_SELECT) {
    uint32_t t = millis() % 1000u;
    // ON 0-100 ms, OFF 100-200 ms, ON 200-300 ms, OFF 300-1000 ms
    bool on = (t < 100u) || (t >= 200u && t < 300u);
    uint32_t c = on ? pixels.Color(255, 0, 0) : 0u;
    for (int i = 0; i < NUM_PIXELS; i++) pixels.setPixelColor(i, c);
    pixels.show();
    return;
  }

  animTick++;
  // breathe phase: a full sine cycle at speed 3 takes ~5 s
  uint8_t bPhase  = (uint8_t)((animTick * (uint32_t)rgbSpeed) >> 2);
  uint8_t bv      = breathe(bPhase);
  // wander offset: shifts gradient across pixels over time
  uint16_t wander = (uint16_t)(animTick * (uint32_t)rgbSpeed * 5u);

  if (layer == LAYER_BASE) {
    // Rainbow: each pixel at a different hue, hue offset drifts
    uint16_t hueOffset = (uint16_t)(animTick * (uint32_t)rgbSpeed * 16u);
    for (int i = 0; i < NUM_PIXELS; i++) {
      uint16_t hue = hueOffset + (uint16_t)((uint32_t)i * 65536u / NUM_PIXELS);
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(hue, rgbSat, bv)));
    }
  } else {
    int32_t h1 = (int32_t)layerHue1[layer];
    int32_t h2 = (int32_t)layerHue2[layer];
    for (int i = 0; i < NUM_PIXELS; i++) {
      // phase of this pixel is staggered by position, creating a travelling wave
      uint8_t pixPhase = (uint8_t)(wander + (uint8_t)((uint16_t)i * 28u));
      uint8_t mix      = breathe(pixPhase);
      int32_t hue32    = h1 + (h2 - h1) * (int32_t)mix / 255;
      // hue32 is always in [min(h1,h2)..max(h1,h2)] subset of [0,65535]
      uint16_t hue     = (uint16_t)hue32;
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(hue, rgbSat, bv)));
    }
  }
  pixels.show();
}

// =============================================================
// OLED - KEY LABEL HELPER
// =============================================================
static String getKeyLabel(const KeyAction& ka) {
  if (ka.type == ACT_NONE) return "---";
  if (ka.type == ACT_LAYER) {
    String s = "TO(";
    s += layerNames[ka.code];
    s += ")";
    return s;
  }
  if (ka.type == ACT_MEDIA) {
    switch (ka.code) {
      case MK_PREV:  return "PREV";
      case MK_SEL:   return "SEL";
      case MK_NEXT:  return "NEXT";
      case MK_REW:   return "REW";
      case MK_PLAY:  return "PLAY";
      case MK_FFAST: return "FFW";
      case MK_STOP:  return "STOP";
      case MK_VOLUP: return "VOL+";
      case MK_VOLDN: return "VOL-";
    }
  }
  if (ka.type == ACT_RGB_CTL) {
    switch (ka.code) {
      case RGBC_SPD_UP: return "SPD+";
      case RGBC_SPD_DN: return "SPD-";
      case RGBC_TOG:    return "TOGG";
      case RGBC_HUE_UP: return "HUE+";
      case RGBC_HUE_DN: return "HUE-";
      case RGBC_VAL_UP: return "BRI+";
      case RGBC_VAL_DN: return "BRI-";
      case RGBC_SAT_UP: return "SAT+";
      case RGBC_SAT_DN: return "SAT-";
    }
  }
  if (ka.type == ACT_KEY) {
    String s = "";
    if (ka.mods & MOD_GUI)   s += "G+";
    if (ka.mods & MOD_CTRL)  s += "C+";
    if (ka.mods & MOD_SHIFT) s += "S+";
    if (ka.mods & MOD_ALT)   s += "A+";
    if      (ka.code == KEY_UP_ARROW)    s += "UP";
    else if (ka.code == KEY_DOWN_ARROW)  s += "DN";
    else if (ka.code == KEY_LEFT_ARROW)  s += "LT";
    else if (ka.code == KEY_RIGHT_ARROW) s += "RT";
    else if (ka.code == KEY_RETURN)      s += "ENT";
    else if (ka.code == KEY_TAB)         s += "TAB";
    else if (ka.code == KEY_BACKSPACE)   s += "BSP";
    else if (ka.code == ' ')             s += "SPC";
    else if (ka.code >= (uint16_t)'a' && ka.code <= (uint16_t)'z') s += (char)ka.code;
    else { s += "[0x"; s += String(ka.code, HEX); s += "]"; }
    return s;
  }
  return "?";
}

// =============================================================
// OLED - NORMAL MODE
// Header: active layer name.
// Body  : last pressed key label, hex code, row/col.
// =============================================================
static void displayNormal() {
  display.clearDisplay();
  display.setTextSize(1);

  // Header bar
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print("Layer: ");
  display.print(layerNames[currentLayer]);
  display.setTextColor(SSD1306_WHITE);

  // Key info
  if (lastPressRow < ROWS) {
    uint8_t idx = lastPressRow * COLS + lastPressCol;
    const KeyAction& ka = keyMap[currentLayer][idx];
    String label = getKeyLabel(ka);

    display.setCursor(0, 14);
    display.print("Key : ");
    display.println(label);

    display.setCursor(0, 26);
    display.print("Code: 0x");
    display.println(ka.code, HEX);

    display.setCursor(0, 38);
    display.print("Pos : r");
    display.print(lastPressRow);
    display.print(" c");
    display.println(lastPressCol);
  } else {
    display.setCursor(0, 24);
    display.println("  Press a key...");
  }

  display.display();
}

// =============================================================
// OLED - SELECT MODE
// 3x3 grid of layer names; active layer inverted.
// Coordinate hint at bottom.
// =============================================================
static void displaySelect() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Select Layer:");

  // Grid: y=11..52, x=0..127
  const int gX = 0, gY = 11;
  const int cW = 42, cH = 13;   // cell width / height in pixels

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      int x = gX + c * cW;
      int y = gY + r * cH;
      int8_t gl = selectGridLayer[r * COLS + c];
      bool isActive = (gl >= 0 && (uint8_t)gl == currentLayer);

      display.drawRect(x, y, cW, cH, SSD1306_WHITE);
      if (isActive) {
        display.fillRect(x + 1, y + 1, cW - 2, cH - 2, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }

      display.setCursor(x + 2, y + 3);
      if (gl >= 0) {
        display.print(layerNames[gl]);
      } else {
        display.print("    ");
      }
      display.setTextColor(SSD1306_WHITE);
    }
  }

  // Coordinate hint
  display.setCursor(0, 55);
  if (lastPressRow < ROWS) {
    display.print("r");
    display.print(lastPressRow);
    display.print(" c");
    display.print(lastPressCol);
    display.print("  -> ");
    display.print(layerNames[currentLayer]);
  } else {
    display.print("turn enc / press key");
  }

  display.display();
}

static void updateDisplay() {
  if (selectMode) {
    displaySelect();
  } else {
    displayNormal();
  }
}

// =============================================================
// SETUP
// =============================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("9KEY MACRO MASTER - BOOT");

  // RGB
  pixels.begin();
  pixels.clear();
  pixels.show();

  // OLED
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init FAILED");
    while (1) delay(10);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Matrix: rows OUTPUT idle-HIGH, cols INPUT_PULLUP
  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }
  // Seed debounce state from an initial scan
  uint32_t now = millis();
  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);
    for (uint8_t c = 0; c < COLS; c++) {
      bool v = (digitalRead(colPins[c]) == LOW);
      rawState[r][c]     = v;
      stableState[r][c]  = v;
      lastChangeMs[r][c] = now;
    }
    digitalWrite(rowPins[r], HIGH);
  }

  // Encoder
  pinMode(ENC_A,   INPUT_PULLUP);
  pinMode(ENC_B,   INPUT_PULLUP);
  pinMode(ENC_BTN, INPUT_PULLUP);
  lastEncA       = digitalRead(ENC_A);
  rawEncBtn      = digitalRead(ENC_BTN);
  lastEncBtn     = rawEncBtn;
  encBtnChangeMs = millis();

  // HID
  Keyboard.begin();
  Mouse.begin();
#if USE_CONSUMER_KEYBOARD
  ConsumerKeyboard.begin();
#endif

  updateDisplay();
  Serial.println("BOOT OK");
}

// =============================================================
// LOOP
// =============================================================
void loop() {
  uint32_t now = millis();

  // -- MATRIX SCAN + DEBOUNCE ----------------------------------
  bool scan[ROWS][COLS] = {};
  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);
    for (uint8_t c = 0; c < COLS; c++) {
      scan[r][c] = (digitalRead(colPins[c]) == LOW);
    }
    digitalWrite(rowPins[r], HIGH);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (scan[r][c] != rawState[r][c]) {
        rawState[r][c]     = scan[r][c];
        lastChangeMs[r][c] = now;
      }
      if ((now - lastChangeMs[r][c]) >= DEBOUNCE_MS &&
          stableState[r][c] != rawState[r][c]) {
        stableState[r][c] = rawState[r][c];
        bool pressed = stableState[r][c];

        Serial.print(pressed ? "PRESS" : "REL  ");
        Serial.print(" r="); Serial.print(r);
        Serial.print(" c="); Serial.println(c);

        if (pressed) {
          lastPressRow = r;
          lastPressCol = c;
        }

        uint8_t actionLayer = selectMode ? LAYER_SELECT : currentLayer;
        const KeyAction& ka = keyMap[actionLayer][r * COLS + c];
        executeKeyAction(ka, pressed);
      }
    }
  }

  // -- ENCODER ROTATION ----------------------------------------
  int encA = digitalRead(ENC_A);
  int encB = digitalRead(ENC_B);
  if (encA != lastEncA) {
    if (encA == LOW) {
      bool cw = (encB == HIGH);
      Serial.print(cw ? "ENC CW" : "ENC CCW");

      if (selectMode) {
        // Cycle through layers 0..(NUM_LAYERS-2), skip LAYER_SELECT
        uint8_t n = NUM_LAYERS - 1;
        if (cw) {
          currentLayer = (currentLayer + 1) % n;
        } else {
          currentLayer = (currentLayer + n - 1) % n;
        }
        Serial.print(" -> "); Serial.println(layerNames[currentLayer]);
      } else {
        switch (currentLayer) {
          case LAYER_BASE:
            Mouse.move(0, 0, cw ? 1 : -1);
            break;
          case LAYER_NAV:
            if (cw) {
              tapKeyWithMods(KEY_TAB, MOD_ALT);
            } else {
              tapKeyWithMods(KEY_TAB, MOD_ALT | MOD_SHIFT);
            }
            break;
          case LAYER_EDIT:
            tapKeyWithMods(cw ? KEY_RIGHT_ARROW : KEY_LEFT_ARROW, MOD_SHIFT);
            break;
          case LAYER_MEDIA:
            sendMediaKey(cw ? MK_VOLUP : MK_VOLDN);
            break;
          case LAYER_FN:
            tapKey(cw ? KEY_RIGHT_ARROW : KEY_LEFT_ARROW);
            break;
          case LAYER_RGB:
            handleRgbControl(cw ? RGBC_VAL_UP : RGBC_VAL_DN);
            break;
        }
      }
    }
    lastEncA = encA;
  }

  // -- ENCODER BUTTON (MO SELECT) – non-blocking debounce -----
  int encBtnRaw = digitalRead(ENC_BTN);
  if (encBtnRaw != rawEncBtn) {
    rawEncBtn      = encBtnRaw;
    encBtnChangeMs = now;
  }
  if ((now - encBtnChangeMs) >= DEBOUNCE_MS && encBtnRaw != lastEncBtn) {
    lastEncBtn = encBtnRaw;
    if (encBtnRaw == LOW) {
      Serial.println("BTN -> SELECT MODE");
      selectMode   = true;
      lastPressRow = 255;   // clear display hint
    } else {
      Serial.println("BTN -> NORMAL MODE");
      selectMode = false;
    }
  }

  // -- RGB UPDATE (~50 Hz) -------------------------------------
  if (now - lastRgbMs >= 20u) {
    lastRgbMs = now;
    updateRgb();
  }

  // -- DISPLAY UPDATE ------------------------------------------
  updateDisplay();

  delay(10);
}
