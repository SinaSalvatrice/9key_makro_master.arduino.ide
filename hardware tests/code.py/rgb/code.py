import time
import board
import neopixel

# =========================
# PIN ANPASSEN
# =========================
RGB_PIN = board.GP13   # nice!nano entsprechend ersetzen
NUM_PIXELS = 1
BRIGHTNESS = 0.2

pixels = neopixel.NeoPixel(
    RGB_PIN,
    NUM_PIXELS,
    brightness=BRIGHTNESS,
    auto_write=False
)

print("RGB TEST START")

while True:
    print("RED")
    pixels.fill((255, 0, 0))
    pixels.show()
    time.sleep(0.5)

    print("GREEN")
    pixels.fill((0, 255, 0))
    pixels.show()
    time.sleep(0.5)

    print("BLUE")
    pixels.fill((0, 0, 255))
    pixels.show()
    time.sleep(0.5)

    print("OFF")
    pixels.fill((0, 0, 0))
    pixels.show()
    time.sleep(0.5)