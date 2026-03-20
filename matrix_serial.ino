
const uint8_t ROWS = 3;
const uint8_t COLS = 3;

const uint8_t rowPins[ROWS] = {2, 3, 4};
const uint8_t colPins[COLS] = {5, 6, 7};

bool lastMatrix[ROWS][COLS];

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("MATRIX TEST START");

  for (uint8_t r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }

  for (uint8_t c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      lastMatrix[r][c] = false;
    }
  }
}

void loop() {
  bool currentMatrix[ROWS][COLS] = {false};

  for (uint8_t r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);

    for (uint8_t c = 0; c < COLS; c++) {
      currentMatrix[r][c] = (digitalRead(colPins[c]) == LOW);
    }

    digitalWrite(rowPins[r], HIGH);
  }

  for (uint8_t r = 0; r < ROWS; r++) {
    for (uint8_t c = 0; c < COLS; c++) {
      if (currentMatrix[r][c] != lastMatrix[r][c]) {
        if (currentMatrix[r][c]) {
          Serial.print("PRESS   r=");
          Serial.print(r);
          Serial.print(" c=");
          Serial.println(c);
        } else {
          Serial.print("RELEASE r=");
          Serial.print(r);
          Serial.print(" c=");
          Serial.println(c);
        }
        lastMatrix[r][c] = currentMatrix[r][c];
      }
    }
  }

  delay(10);
}
