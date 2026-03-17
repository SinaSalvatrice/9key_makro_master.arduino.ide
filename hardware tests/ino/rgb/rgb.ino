#include <Adafruit_NeoPixel.h>

#define RGB_PIN 12
#define NUM_PIXELS 9

Adafruit_NeoPixel pixels(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("RGB TEST START");

  pixels.begin();
  pixels.clear();
  pixels.show();
}

void loop() {
  Serial.println("RED");
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show();
  delay(500);

  Serial.println("GREEN");
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  delay(500);

  Serial.println("BLUE");
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
  delay(500);

  Serial.println("OFF");
  pixels.clear();
  pixels.show();
  delay(500);
}