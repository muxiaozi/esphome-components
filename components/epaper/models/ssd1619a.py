"""Monochrome e-paper displays driven by a single SSD1619A controller.

The SSD1619A has 400 source and 300 gate outputs, so a single chip drives a
4.2" 400x300 panel (e.g. HINK-E042A13-A0) - unlike the dual-SSD1608 panels
handled by :mod:`ssd1608_dual`. It therefore needs one chip select and the
normal SPI bus CS handling.

RAM layout: X is the source direction (one byte per 8 sources, MSB first),
Y the gate direction. With data entry mode 0x03 (X/Y increment) a row is
width/8 bytes, matching ESPHome's 1bpp buffer directly.

The controller has separate BW and RED RAM planes. On a black/white panel
the RED plane is also the previous frame used by the differential waveform,
which the C++ driver keeps in sync after every refresh.
"""

from . import SSD1619AModel


class SSD1619A(SSD1619AModel):
    """EpaperModel for single-SSD1619A monochrome e-paper displays."""

    def get_init_sequence(self, config: dict) -> tuple:
        """Panel configuration for the SSD1619A's own registers.

        VCOM is loaded from OTP by the software reset and the waveform by the
        refresh sequence (0x22 = 0xF7), so neither is written here. 0x18
        selects the internal temperature sensor that 0xF7 reads.
        """
        _width, height = self.get_dimensions(config)
        return (
            # Driver output control: MUX = gate lines - 1, GD=0/SM=0/TB=0
            (0x01, (height - 1) % 256, (height - 1) // 256, 0x00),
            # Border waveform
            (0x3C, 0x01),
            # Use the internal temperature sensor
            (0x18, 0x80),
            # Data entry mode: address counter updates in X direction, X/Y increment
            (0x11, 0x03),
        )


ssd1619a = SSD1619A(
    "ssd1619a",
    width=400,
    height=300,
    minimum_update_interval="10s",
)

# Holitech/Hanshow HINK-E042A13-A0 4.2" 400x300 mono, single SSD1619A.
hink_e042a13_a0 = ssd1619a.extend("hink-e042a13-a0")
