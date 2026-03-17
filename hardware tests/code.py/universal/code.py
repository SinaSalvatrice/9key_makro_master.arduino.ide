import time
import board
import busio
import displayio
import digitalio
import rotaryio
import neopixel
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306

# =========================
# PINS ANPASSEN
# =========================
RGB_PIN = board.GP13

ROW_PINS = [board.GP2, board.GP3, board.GP4]
COL_PINS = [board.GP5, board.GP6, board.GP7]

ENC_A = board.GP8
ENC_B = board.GP9
ENC_BTN = board.GP10

I2C_SCL = board.GP1
I2C_SDA = board.GP0

# =========================
# CONFIG
# =========================
NUM_PIXELS = 1
BRIGHTNESS = 0.15
DISPLAY_WIDTH = 128
DISPLAY_HEIGHT = 64
DEBOUNCE_S = 0.02

print("ALL-IN-ONE TEST START")

# =========================
# RGB
# =========================
pixels = neopixel.NeoPixel(
    RGB_PIN,
    NUM_PIXELS,
    brightness=BRIGHTNESS,
    auto_write=False
)
pixels.fill((0, 0, 0))
pixels.show()

# =========================
# DISPLAY
# =========================
displayio.release_displays()
i2c = busio.I2C(I2C_SCL, I2C_SDA)
display_bus = displayio.I2CDisplay(i2c, device_address=0x3C)
display = adafruit_displayio_ssd1306.SSD1306(
    display_bus,
    width=DISPLAY_WIDTH,
    height=DISPLAY_HEIGHT
)

group = displayio.Group()
display.root_group = group

text = label.Label(
    terminalio.FONT,
    text="BOOT...",
    color=0xFFFFFF,
    x=0,
    y=8
)
group.append(text)

# =========================
# MATRIX
# =========================
rows = []
for pin in ROW_PINS:
    r = digitalio.DigitalInOut(pin)
    r.direction = digitalio.Direction.OUTPUT
    r.value = False
    rows.append(r)

cols = []
for pin in COL_PINS:
    c = digitalio.DigitalInOut(pin)
    c.direction = digitalio.Direction.INPUT
    c.pull = digitalio.Pull.DOWN
    cols.append(c)

last_matrix = [[False for _ in COL_PINS] for _ in ROW_PINS]

# =========================
# ENCODER
# =========================
encoder = rotaryio.IncrementalEncoder(ENC_A, ENC_B)

btn = digitalio.DigitalInOut(ENC_BTN)
btn.direction = digitalio.Direction.INPUT
btn.pull = digitalio.Pull.UP

last_pos = encoder.position
last_btn = btn.value

last_event = "NONE"
last_update = 0

def scan_matrix():
    pressed = []
    for r_idx, row in enumerate(rows):
        row.value = True
        time.sleep(0.001)
        for c_idx, col in enumerate(cols):
            if col.value:
                pressed.append((r_idx, c_idx))
        row.value = False
    return pressed

def set_rgb_mode(step):
    mode = step % 4
    if mode == 0:
        pixels.fill((255, 0, 0))
    elif mode == 1:
        pixels.fill((0, 255, 0))
    elif mode == 2:
        pixels.fill((0, 0, 255))
    else:
        pixels.fill((0, 0, 0))
    pixels.show()

set_rgb_mode(0)

while True:
    now = time.monotonic()

    # -------- MATRIX --------
    current_pressed = scan_matrix()
    current_matrix = [[False for _ in COL_PINS] for _ in ROW_PINS]

    for r, c in current_pressed:
        current_matrix[r][c] = True

    for r in range(len(ROW_PINS)):
        for c in range(len(COL_PINS)):
            if current_matrix[r][c] != last_matrix[r][c]:
                if current_matrix[r][c]:
                    last_event = "KEY r{} c{}".format(r, c)
                    print("PRESS  r={} c={}".format(r, c))
                else:
                    last_event = "REL r{} c{}".format(r, c)
                    print("RELEASE r={} c={}".format(r, c))

    last_matrix = current_matrix

    # -------- ENCODER --------
    pos = encoder.position
    if pos != last_pos:
        direction = "CW" if pos > last_pos else "CCW"
        last_event = "ENC {} {}".format(direction, pos)
        print("ENC:", pos, direction)
        set_rgb_mode(pos)
        last_pos = pos

    # -------- ENCODER BUTTON --------
    val = btn.value
    if val != last_btn:
        if not val:
            last_event = "BTN DOWN"
            print("BTN: GEDRUECKT")
            pixels.fill((255, 255, 255))
            pixels.show()
        else:
            last_event = "BTN UP"
            print("BTN: LOS")
            set_rgb_mode(last_pos)
        last_btn = val

    # -------- DISPLAY --------
    if now - last_update > 0.1:
        text.text = (
            "ALL OK\n"
            "ENC: {}\n"
            "BTN: {}\n"
            "LAST: {}".format(
                last_pos,
                "DOWN" if not btn.value else "UP",
                last_event
            )
        )
        last_update = now

    time.sleep(DEBOUNCE_S)