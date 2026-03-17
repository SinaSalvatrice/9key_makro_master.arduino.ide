from adafruit_hid.consumer_control_code import ConsumerControlCode
from adafruit_hid.keycode import Keycode

app = {  # REQUIRED dict, must be named 'app'
    "name": "Media",  # Application name
    "macros": [  # List of button macros...
        # COLOR    LABEL    KEY SEQUENCE
        # top row
        (0x000000, "Music", [Keycode.CONTROL, Keycode.SHIFT, 'M']),
        (0x000000, "", [[ConsumerControlCode.SCAN_PREVIOUS_TRACK]]),
        (0x000000, "", [[ConsumerControlCode.PLAY_PAUSE]]),
        (0x000000, "", [[ConsumerControlCode.SCAN_NEXT_TRACK]]),
        # middle row
        (0x000000, "Volume", [[ConsumerControlCode.MUTE]]),
        (0x000000, "", [[ConsumerControlCode.VOLUME_DECREMENT]]),
        (0x000000, "", [[ConsumerControlCode.VOLUME_INCREMENT]]),
        (0x000000, "", [Keycode.GUI]),
        # bottom row
        (0x000000, "Discord", [Keycode.F13]),
        (0x000000, "", [Keycode.F14]),
        (0x000000, "", [Keycode.F15]),
        (0x000000, "", [Keycode.F16]),
        # Encoder button
        (0x000000, "MUTE", [[ConsumerControlCode.MUTE]]),
    ],
}
