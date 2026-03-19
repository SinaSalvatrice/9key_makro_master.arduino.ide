from adafruit_hid.consumer_control_code import ConsumerControlCode
from adafruit_hid.keycode import Keycode

app = {  # REQUIRED dict, must be named 'app'
    "name": "OS",  # Application name
    "macros": [  # List of button macros...
        # COLOR    LABEL    KEY SEQUENCE
        # top row
        (0x000000, "Audio", [Keycode.CONTROL, Keycode.SHIFT, 'M']),
        (0x000000, "", [[ConsumerControlCode.SCAN_PREVIOUS_TRACK]]),
        (0x000000, "", [[ConsumerControlCode.PLAY_PAUSE]]),
        (0x000000, "", [[ConsumerControlCode.SCAN_NEXT_TRACK]]),
        # middle row
        (0x000000, "Delete", [[ConsumerControlCode.MUTE]]),
        (0x000000, "", [Keycode.BACKSPACE]),
        (0x000000, "", [Keycode.DELETE]),
        (0x000000, "", [Keycode.PRINT_SCREEN]),
        # bottom row
        (0x000000, "Copy", [Keycode.CONTROL, Keycode.C]),
        (0x000000, "", [Keycode.CONTROL, Keycode.V]),
        (0x000000, "", [Keycode.CONTROL, Keycode.X]),
        (0x000000, "", [Keycode.CONTROL, Keycode.A]),
        # Encoder button
        (0x000000, "MUTE", [[ConsumerControlCode.MUTE]]),
    ],
}
