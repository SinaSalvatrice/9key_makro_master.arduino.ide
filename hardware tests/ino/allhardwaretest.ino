#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// =========================
// PIN CONFIG
// =========================
#define SDA_PIN 0
#define SCL_PIN 1

#define RGB_PIN 13
#define NUM_PIXELS 1

const uint8_t ROWS = 3;
const uint8_t COLS = 3;

const uint8_t rowPins[ROWS] = {2, 4, 3};
const uint8_t colPins[COLS] = {5, 6, 7};

const uint8_t ENC_A = 8;
const uint8_t ENC_B = 9;
const uint8_t ENC_BTN = 10;

// =========================
// DISPLAY
// =========================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// =========================
// STATE
// =========================
bool lastMatrix[ROWS][COLS];
int lastA = HIGH;
int lastBtn = HIGH;
long encPos = 0;
String lastEvent = "BOOT";

void setRgbMode(long pos) {
  int mode = ((pos % 4) + 4) % 4;

  if (mode == 0) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  } else if (mode == 1) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  } else if (mode == 2) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  } else {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  }
  pixels.show();
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("ALL OK");
  display.print("ENC: ");
  display.println(encPos);
  display.print("BTN: ");
  display.println(digitalRead(ENC_BTN) == LOW ? "DOWN" : "UP");
  display.print("LAST: ");
  display.println(lastEvent);
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ALL-IN-ONE TEST START");

  // RGB
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Display
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 init failed");
    while (1) delay(10);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // =========================
  // MATRIX (umgedrehte Scanrichtung)
  // Cols = OUTPUT, idle HIGH
  // Rows = INPUT_PULLUP
  // Active col = LOW
  // Pressed = LOW auf Row
  // =========================
  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(colPins[c], OUTPUT);
    digitalWrite(colPins[c], HIGH);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], INPUT_PULLUP);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      lastMatrix[r][c] = false;
    }
  }

  // Encoder
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_BTN, INPUT_PULLUP);

  lastA = digitalRead(ENC_A);
  lastBtn = digitalRead(ENC_BTN);

  setRgbMode(0);
  updateDisplay();
}

void loop() {
  // =========================
  // MATRIX
  // =========================
  bool currentMatrix[ROWS][COLS] = {false};

  for (uint8_t c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);   // diese Column aktiv
    delayMicroseconds(50);

    for (uint8_t r = 0; r < ROWS; r++) {
      currentMatrix[r][c] = (digitalRead(rowPins[r]) == LOW);
    }

    digitalWrite(colPins[c], HIGH);  // Column wieder deaktivieren
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (currentMatrix[r][c] != lastMatrix[r][c]) {
        if (currentMatrix[r][c]) {
          Serial.print("PRESS   r=");
          Serial.print(r);
          Serial.print(" c=");
          Serial.println(c);
          lastEvent = "KEY " + String(r) + "," + String(c);
        } else {
          Serial.print("RELEASE r=");
          Serial.print(r);
          Serial.print(" c=");
          Serial.println(c);
          lastEvent = "REL " + String(r) + "," + String(c);
        }
        lastMatrix[r][c] = currentMatrix[r][c];
      }
    }
  }

  // =========================
  // ENCODER
  // =========================
  int a = digitalRead(ENC_A);
  int b = digitalRead(ENC_B);

  if (a != lastA) {
    if (a == LOW) {
      if (b == HIGH) {
        encPos++;
        Serial.print("ENC CW  pos=");
        Serial.println(encPos);
        lastEvent = "ENC CW";
      } else {
        encPos--;
        Serial.print("ENC CCW pos=");
        Serial.println(encPos);
        lastEvent = "ENC CCW";
      }
      setRgbMode(encPos);
    }
    lastA = a;
  }

  // =========================
  // BUTTON
  // =========================
  int btn = digitalRead(ENC_BTN);
  if (btn != lastBtn) {
    if (btn == LOW) {
      Serial.println("BTN DOWN");
      lastEvent = "BTN DOWN";
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show();
    } else {
      Serial.println("BTN UP");
      lastEvent = "BTN UP";
      setRgbMode(encPos);
    }
    lastBtn = btn;
    delay(20);
  }

  updateDisplay();
  delay(20);
}
