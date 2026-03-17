import time
import board
import busio
import displayio
from adafruit_display_text import label
import terminalio
import adafruit_displayio_ssd1306

# =========================
# PINS ANPASSEN
# =========================
I2C_SCL = board.GP1   # nice!nano ersetzen
I2C_SDA = board.GP0   # nice!nano ersetzen

DISPLAY_WIDTH = 128
DISPLAY_HEIGHT = 64

print("DISPLAY TEST START")

displayio.release_displays()

i2c = busio.I2C(I2C_SCL, I2C_SDA)
display_bus = displayio.I2CDisplay(i2c, device_address=0x3C)
display = adafruit_displayio_ssd1306.SSD1306(
    display_bus,
    width=DISPLAY_WIDTH,
    height=DISPLAY_HEIGHT
)

main_group = displayio.Group()
display.root_group = main_group

text = label.Label(
    terminalio.FONT,
    text="DISPLAY OK",
    color=0xFFFFFF,
    x=10,
    y=16
)
main_group.append(text)

counter = 0

while True:
    text.text = "DISPLAY OK\nCOUNT: {}".format(counter)
    print("DISPLAY COUNT:", counter)
    counter += 1
    time.sleep(1)