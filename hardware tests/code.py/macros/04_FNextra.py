from adafruit_hid.consumer_control_code import ConsumerControlCode
from adafruit_hid.keycode import Keycode

app = {  # REQUIRED dict, must be named 'app'
    "name": "FN Extra",  # Application name
    "macros": [  # List of button macros...
        # COLOR    LABEL    KEY SEQUENCE
        # top row
        (0x000000, "F21-F24", [Keycode.F21]),
        (0x000000, "", [Keycode.F22]),
        (0x000000, "", [Keycode.F23]),
        (0x000000, "", [Keycode.F24]),
        # middle row
        (0x000000, "F17-F20", [Keycode.F17]),
        (0x000000, "", [Keycode.F18]),
        (0x000000, "", [Keycode.F19]),
        (0x000000, "", [Keycode.F20]),
        # bottom row
        (0x000000, "F13-F16", [Keycode.F13]),
        (0x000000, "", [Keycode.F14]),
        (0x000000, "", [Keycode.F15]),
        (0x000000, "", [Keycode.F16]),
        # Encoder button
        (0x000000, "MUTE", [[ConsumerControlCode.MUTE]]),
    ],
}
