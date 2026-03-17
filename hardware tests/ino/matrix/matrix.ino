const uint8_t ROWS = 3;
const uint8_t COLS = 3;

const uint8_t rowPins[ROWS] = {2, 3, 4};
const uint8_t colPins[COLS] = {5, 6, 7};

bool lastState[ROWS][COLS];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("MATRIX TEST START");

  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], LOW);
  }

  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLDOWN);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      lastState[r][c] = false;
    }
  }
}

void loop() {
  bool currentState[ROWS][COLS] = {false};

  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], HIGH);
    delayMicroseconds(500);

    for (uint8_t c = 0; c < COLS; c++) {
      currentState[r][c] = digitalRead(colPins[c]);
    }

    digitalWrite(rowPins[r], LOW);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (currentState[r][c] != lastState[r][c]) {
        if (currentState[r][c]) {
          Serial.print("PRESS  r=");
          Serial.print(r);
          Serial.print(" c=");
          Serial.println(c);
        } else {
          Serial.print("RELEASE r=");
          Serial.print(r);
          Serial.print(" c=");
          Serial.println(c);
        }
        lastState[r][c] = currentState[r][c];
      }
    }
  }

  delay(20);
}