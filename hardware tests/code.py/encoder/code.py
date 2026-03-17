import time
import board
import digitalio
import rotaryio

# =========================
# PINS ANPASSEN
# =========================
ENC_A = board.GP8      # nice!nano ersetzen
ENC_B = board.GP9      # nice!nano ersetzen
ENC_BTN = board.GP10   # nice!nano ersetzen

print("ENCODER TEST START")

encoder = rotaryio.IncrementalEncoder(ENC_A, ENC_B)

btn = digitalio.DigitalInOut(ENC_BTN)
btn.direction = digitalio.Direction.INPUT
btn.pull = digitalio.Pull.UP

last_pos = encoder.position
last_btn = btn.value
last_live = time.monotonic()

while True:
    pos = encoder.position
    if pos != last_pos:
        print("ENC:", pos)
        last_pos = pos

    val = btn.value
    if val != last_btn:
        print("BTN:", "LOS" if val else "GEDRUECKT")
        last_btn = val

    if time.monotonic() - last_live > 2:
        print("LIVE")
        last_live = time.monotonic()

    time.sleep(0.01)