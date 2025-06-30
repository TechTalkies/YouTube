# SPDX-FileCopyrightText: 2017 Scott Shawcroft, written for Adafruit Industries
# SPDX-FileCopyrightText: Copyright (c) 2022 Radomir Dopieralski
#
# SPDX-License-Identifier: MIT
"""
`paj7620`
================================================================================

Driver for the PAJ7620 gesture sensor.


* Author(s): Radomir Dopieralski

Implementation Notes
--------------------

**Hardware:**

  * `Waveshare Breakout <https://www.waveshare.com/wiki/PAJ7620U2_Gesture_Sensor>`_"

**Software and Dependencies:**

* Adafruit CircuitPython firmware for the supported boards:
  https://circuitpython.org/downloads
* Adafruit's Bus Device library:
  https://github.com/adafruit/Adafruit_CircuitPython_BusDevice
"""

# imports

__version__ = "0.0.0+auto.0"
__repo__ = "https://github.com/deshipu/CircuitPython_paj7620.git"


from adafruit_bus_device.i2c_device import I2CDevice


UP = 0x01
DOWN = 0x02
LEFT = 0x04
RIGHT = 0x08
NEAR = 0x10
FAR = 0x20
CW = 0x40
CCW = 0x80
WAVE = 0x100

_ADDR = (
    b"\xefAB789BFGHIJLQ^`\x80\x81\x82\x8b\x90\x95\x96\x97\x9a\x9c"
    b"\xa5\xcc\xcd\xce\xcf\xd0\xef\x02\x03\x04%'()>^egijmnrstw\xef"
    b"AB"
)
_DATA = (
    b"\x00\x00\x00\x07\x17\x06\x01-\x0f<\x00\x1e\"\x10\x10'BD\x04"
    b"\x01\x06\n\x0c\x05\x14?\x19\x19\x0b\x13d!\x01\x0f\x10\x02\x01"
    b"9\x7f\x08\xff=\x96\x97\xcd\x01,\x01\x015\x00\x01\x00\xff\x01"
)


class PAJ7620Gesture:
    """Gesture sensor."""

    buf = bytearray(2)

    def __init__(self, i2c, addr=0x73):
        self.device = I2CDevice(i2c, addr)

        with self.device as device:
            for address, data in zip(_ADDR, _DATA):
                self.buf[0] = address
                self.buf[1] = data
                device.write(self.buf)

    def read(self):
        """Read and clear the gestures from the sensor."""

        with self.device as device:
            device.write_then_readinto(b"\x43", self.buf)
        return int.from_bytes(self.buf, "little")
