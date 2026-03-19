import board
import digitalio
import storage

button = digitalio.DigitalInOut(board.KEY1)
button.pull = digitalio.Pull.UP

if button.value:
    storage.disable_usb_drive()