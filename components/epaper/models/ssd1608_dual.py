"""Monochrome e-paper displays driven by two cascaded SSD1608 controllers.

The SSD1608 is a 240x320 controller; larger panels use two of them in the
cascade mode described in the SSD1608 datasheet (section 6.11): the master
chip (M/S# tied to VDDIO) provides the clock and all booster voltages, while
the slave chip (M/S# tied to VSS) has its oscillator and booster disabled and
its BUSY pin left floating. Each chip drives half of the horizontal pixels
and has its own chip select; SCLK, MOSI, D/C and RESET are shared.

The MCU's BUSY line is therefore the *master's* BUSY output. ``master``
selects which chip select is connected to the master; the other chip is
activated first during a refresh so that when the master reports idle, the
slave has already finished.

Supported models:
- hink-e042a03-a1: Holitech/Hanshow HINK-E042A03-A1, 4.2" 400x300 mono,
  two SSD1608s each driving 200x300.

Both chips are assumed to be mounted with the same orientation. If one half
of the panel ends up rotated 180 degrees (chips mounted facing each other),
set ``flip_cs``/``flip_cs1`` to compensate.
"""

from . import CONF_USE_OTP_LUT, SSD1608Model

# Waveform LUT from Adafruit's SSD1608 driver (adafruit_epd/ssd1608.py).
# 30 bytes; VSH/VSL and the dummy byte are not part of the 0x32 payload.
# Only used when ``use_otp_lut: false``.
SSD1608_LUT = (
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22, 0x66, 0x69,
    0x69, 0x59, 0x58, 0x99, 0x99, 0x88, 0x00, 0x00, 0x00, 0x00,
    0xF8, 0xB4, 0x13, 0x51, 0x35, 0x51, 0x51, 0x19, 0x01, 0x00,
)


class SSD1608DualModel(SSD1608Model):
    """EpaperModel for dual-SSD1608 monochrome e-paper displays."""

    def get_init_sequence(self, config: dict) -> tuple:
        """Panel configuration for the SSD1608's own registers only.

        VCOM and the waveform LUT are loaded from the panel OTP by hardware
        reset and must not be overwritten (0x2C/0x32). Registers 0x18/0x1A of
        the SSD1680 family do not exist here (0x18 is reserved on the SSD1608).
        """
        _width, height = self.get_dimensions(config)
        sequence = [
            # Driver output control: MUX = gate lines - 1, GD=0/SM=0/TB=0
            (0x01, (height - 1) % 256, (height - 1) // 256, 0x00),
            # Dummy line period and gate line width. POR is 0x02/0x08, but
            # vendor reference code (Adafruit SSD1608) uses 0x1B/0x0B; these
            # parameters shape the gate waveform.
            (0x3A, 0x1B),
            (0x3B, 0x0B),
            # Border waveform: GS transition GS1/GS1
            (0x3C, 0x33),
            # Data entry mode: address counter updates in X direction, X/Y increment
            (0x11, 0x03),
        ]
        if not config.get(CONF_USE_OTP_LUT, True):
            # Controllers whose OTP is blank never finish an update sequence
            # that loads the waveform from OTP. Write the LUT and VCOM
            # explicitly instead; refresh_screen() then uses 0x22=0xC7.
            sequence += [
                (0x2C, 0x70),  # VCOM
                (0x32, *SSD1608_LUT),  # LUT
            ]
        return tuple(sequence)


ssd1608_dual = SSD1608DualModel(
    "ssd1608-dual",
    minimum_update_interval="10s",
)

# Holitech/Hanshow HINK-E042A03-A1 4.2" 400x300 mono, dual SSD1608.
# The controllers on this panel have a blank OTP, so the waveform must be
# written explicitly (use_otp_lut: false); loading it from OTP leaves BUSY
# high forever.
hink_e042a03_a1 = ssd1608_dual.extend(
    "hink-e042a03-a1",
    width=400,
    height=300,
    use_otp_lut=False,
)
