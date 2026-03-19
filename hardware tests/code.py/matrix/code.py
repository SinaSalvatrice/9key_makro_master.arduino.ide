import time
import board
import digitalio

# =========================
# PINS ANPASSEN
# =========================
ROW_PINS = [board.GP2, board.GP3, board.GP4]   # nice!nano ersetzen
COL_PINS = [board.GP5, board.GP6, board.GP7]   # nice!nano ersetzen

DEBOUNCE_S = 0.03

print("MATRIX TEST START")

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

last_state = [[False for _ in COL_PINS] for _ in ROW_PINS]

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

while True:
    current_pressed = scan_matrix()
    current_state = [[False for _ in COL_PINS] for _ in ROW_PINS]

    for r, c in current_pressed:
        current_state[r][c] = True

    for r in range(len(ROW_PINS)):
        for c in range(len(COL_PINS)):
            if current_state[r][c] != last_state[r][c]:
                if current_state[r][c]:
                    print("PRESS  r={} c={}".format(r, c))
                else:
                    print("RELEASE r={} c={}".format(r, c))

    last_state = current_state
    time.sleep(DEBOUNCE_S)