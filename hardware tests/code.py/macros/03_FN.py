from adafruit_hid.consumer_control_code import ConsumerControlCode
from adafruit_hid.keycode import Keycode

app = {  # REQUIRED dict, must be named 'app'
    "name": "FN Row",  # Application name
    "macros": [  # List of button macros...
        # COLOR    LABEL    KEY SEQUENCE
        # top row
        (0x000000, "F9-F12", [Keycode.F9]),
        (0x000000, "", [Keycode.F10]),
        (0x000000, "", [Keycode.F11]),
        (0x000000, "", [Keycode.F12]),
        # middle row
        (0x000000, "F5-F8", [Keycode.F5]),
        (0x000000, "", [Keycode.F6]),
        (0x000000, "", [Keycode.F7]),
        (0x000000, "", [Keycode.F8]),
        # bottom row
        (0x000000, "F1-F4", [Keycode.F1]),
        (0x000000, "", [Keycode.F2]),
        (0x000000, "", [Keycode.F3]),
        (0x000000, "", [Keycode.F4]),
        # Encoder button
        (0x000000, "MUTE", [[ConsumerControlCode.MUTE]]),
    ],
}
