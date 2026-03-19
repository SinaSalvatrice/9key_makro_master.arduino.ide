import busio
import board
import digitalio
import rotaryio
import time
import neopixel
import adafruit_ssd1306

# Matrix pins
row_pins = [board.GP2, board.GP4, board.GP3]
col_pins = [board.GP5, board.GP6, board.GP7]

# RGB
rgb_pin = board.GP13
num_pixels = 10
pixels = neopixel.NeoPixel(rgb_pin, num_pixels, brightness=0.2, auto_write=True)

# Encoder
encoder = rotaryio.IncrementalEncoder(board.GP8, board.GP9)
last_position = encoder.position

# OLED
i2c = busio.I2C(board.GP1, board.GP0)
display = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c)
display.fill(0)
display.show()

# Matrix setup
rows = [digitalio.DigitalInOut(pin) for pin in row_pins]
for r in rows:
    r.direction = digitalio.Direction.OUTPUT
    r.value = True
cols = [digitalio.DigitalInOut(pin) for pin in col_pins]
for c in cols:
    c.direction = digitalio.Direction.INPUT
    c.pull = digitalio.Pull.UP

while True:
    # Matrix scan
    pressed_keys = []
    for row_idx, row in enumerate(rows):
        row.value = False
        time.sleep(0.001)
        for col_idx, col in enumerate(cols):
            if not col.value:
                pressed_keys.append((row_idx, col_idx))
        row.value = True

    # Encoder
    position = encoder.position
    if position != last_position:
        display.fill(0)
        display.text(f"Encoder: {position}", 0, 0, 1)
        display.show()
        last_position = position

    # RGB breathing effect
    color = (0, 0, 255)  # Blue
    for i in range(num_pixels):
        pixels[i] = color

    # OLED display
    display.fill(0)
    display.text("3x3 Macropad", 0, 0, 1)
    y = 8
    for key in pressed_keys:
        display.text(f"Key: {key}", 0, y, 1)
        y += 8
    display.show()

    time.sleep(0.05)