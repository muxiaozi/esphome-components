"""Model registry for the ``epaper`` external component.

The built-in ``epaper_spi`` component keeps a single global model registry
that external components have no public API to extend. This component
therefore owns separate registries: models still derive from
:class:`~esphome.components.epaper_spi.models.EpaperModel` so the familiar
option/init-sequence helpers are reused, but every instance registers itself
here instead of in the built-in table. The platform schema sees the merged
``MODELS`` table at the bottom of this module.
"""

from typing import Any

from esphome import pins
import esphome.codegen as cg
from esphome.components.epaper_spi.models import EpaperModel
import esphome.config_validation as cv
from esphome.const import CONF_BUSY_PIN, CONF_CS_PIN, CONF_DATA_RATE, CONF_RESET_PIN
from esphome.cpp_generator import MockObj

CONF_CS1_PIN = "cs1_pin"
CONF_MASTER = "master"
CONF_FLIP_CS = "flip_cs"
CONF_FLIP_CS1 = "flip_cs1"
CONF_USE_OTP_LUT = "use_otp_lut"


class SSD1608Model(EpaperModel):
    """EpaperModel for monochrome panels driven by cascaded SSD1608 chips."""

    models: dict[str, "SSD1608Model"] = {}

    # The driver drives CS and CS1 directly to address each chip.
    manages_cs = True

    def __init__(
        self, name: str, class_name: str = "EPaperSSD1608Dual", **defaults: Any
    ) -> None:
        # The SSD1608 SPI interface is rated for a 250ns clock cycle (4MHz);
        # faster rates may work on individual boards but are out of spec.
        defaults.setdefault(CONF_DATA_RATE, "4MHz")
        super().__init__(name, class_name, **defaults)
        # EpaperModel.__init__ writes into the built-in component's global
        # table, which would make this model selectable under
        # ``platform: epaper_spi``. Move the entry into the local registry.
        EpaperModel.models.pop(self.name, None)
        SSD1608Model.models[self.name] = self

    def option(self, name, fallback=cv.UNDEFINED) -> cv.Optional | cv.Required:
        # The driver needs both chip selects plus a hardware reset (deep sleep
        # can only be exited by reset) and the master BUSY line.
        if name in (CONF_CS_PIN, CONF_RESET_PIN, CONF_BUSY_PIN):
            return cv.Required(name)
        return super().option(name, fallback)

    def get_config_options(self) -> dict:
        # CS1 is the second chip-select required by the dual-controller panel.
        # fallback=None makes it required unless the model provides a default.
        return {
            self.option(CONF_CS1_PIN, fallback=None): pins.gpio_output_pin_schema,
            # Which chip select is wired to the master controller (the one whose
            # BUSY output the busy_pin is connected to). The slave is activated
            # first on refresh so both chips are done when the master goes idle.
            cv.Optional(CONF_MASTER, default="cs"): cv.one_of("cs", "cs1", lower=True),
            # Set when the corresponding chip is mounted rotated 180 degrees
            # (e.g. the two COB chips face each other on the glass).
            cv.Optional(CONF_FLIP_CS, default=False): cv.boolean,
            cv.Optional(CONF_FLIP_CS1, default=False): cv.boolean,
            # When false, the driver writes a hard-coded LUT + VCOM (as
            # Adafruit's SSD1608 driver does) and refreshes with 0x22=0xC7
            # instead of loading the waveform from the controller's OTP.
            # Use this when the panel's OTP is not programmed; a controller
            # with an empty/blank OTP otherwise stays busy indefinitely.
            self.option(CONF_USE_OTP_LUT, fallback=True): cv.boolean,
        }

    async def to_code(self, var: MockObj, config: dict) -> dict:
        cs = await cg.gpio_pin_expression(config[CONF_CS_PIN])
        cs1 = await cg.gpio_pin_expression(config[CONF_CS1_PIN])
        cg.add(var.set_cs_pins(cs, cs1))
        cg.add(var.set_master(config[CONF_MASTER] == "cs1"))
        cg.add(var.set_flip(config[CONF_FLIP_CS], config[CONF_FLIP_CS1]))
        cg.add(var.set_use_otp_lut(config[CONF_USE_OTP_LUT]))
        # Remove the options handled here so the base class doesn't try to
        # handle them (in particular, cs_pin must not reach
        # spi.register_spi_device(), which would let the bus drive it).
        consumed = (
            CONF_CS_PIN,
            CONF_CS1_PIN,
            CONF_MASTER,
            CONF_FLIP_CS,
            CONF_FLIP_CS1,
        )
        return {k: v for k, v in config.items() if k not in consumed}


class SSD1619AModel(EpaperModel):
    """EpaperModel for monochrome panels driven by a single SSD1619A chip."""

    models: dict[str, "SSD1619AModel"] = {}

    # A single controller: the SPI bus drives CS like for any other device.
    manages_cs = False

    def __init__(
        self, name: str, class_name: str = "EPaperSSD1619A", **defaults: Any
    ) -> None:
        super().__init__(name, class_name, **defaults)
        # EpaperModel.__init__ writes into the built-in component's global
        # table, which would make this model selectable under
        # ``platform: epaper_spi``. Move the entry into the local registry.
        EpaperModel.models.pop(self.name, None)
        SSD1619AModel.models[self.name] = self

    def option(self, name, fallback=cv.UNDEFINED) -> cv.Optional | cv.Required:
        # The driver needs a chip select, a hardware reset (deep sleep can
        # only be exited by reset) and the BUSY line.
        if name in (CONF_CS_PIN, CONF_RESET_PIN, CONF_BUSY_PIN):
            return cv.Required(name)
        return super().option(name, fallback)


# Importing the concrete models registers them in the two registries above.
from .ssd1608_dual import hink_e042a03_a1, ssd1608_dual  # noqa: E402, F401
from .ssd1619a import hink_e042a13_a0, ssd1619a  # noqa: E402, F401

# Both drivers live in this component; the platform schema looks up one table.
MODELS = {**SSD1608Model.models, **SSD1619AModel.models}
