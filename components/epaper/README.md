# epaper

External ESPHome display platform for e-paper panels driven by:

- **two cascaded SSD1608 controllers** (e.g. Holitech/Hanshow HINK-E042A03-A1,
  4.2" 400x300), or
- **a single SSD1619A controller** (e.g. HINK-E042A13-A0, 4.2" 400x300).

It reuses the built-in [`epaper_spi`](../../../esphome/components/epaper_spi)
C++ base class (`epaper_spi::EPaperBase`) and therefore needs ESPHome
**2026.9.0 or newer** (the version this component was developed against).

## Usage

### Dual SSD1608

```yaml
external_components:
  - source:
      type: local
      path: components

spi:
  clk_pin: GPIO12
  mosi_pin: GPIO11

display:
  - platform: epaper
    id: my_display
    model: hink-e042a03-a1
    cs_pin: GPIO10        # master or slave, see `master`
    cs1_pin: GPIO14       # the other controller
    dc_pin: GPIO8
    busy_pin: GPIO9       # wired to the master controller only
    reset_pin: GPIO7
    rotation: 0
    full_update_every: 30
```

### Single SSD1619A

```yaml
spi:
  clk_pin: GPIO12
  mosi_pin: GPIO11

display:
  - platform: epaper
    id: my_display
    model: ssd1619a       # or hink-e042a13-a0
    cs_pin: GPIO10
    dc_pin: GPIO8
    busy_pin: GPIO9
    reset_pin: GPIO7
    rotation: 0
    full_update_every: 30
```

For the dual-SSD1608 models, `cs_pin`, `cs1_pin`, `reset_pin` and `busy_pin`
are required. `master` (`cs` or `cs1`, default `cs`) tells the driver which
chip select is connected to the controller that drives the panel's BUSY
output; the other chip is activated first during a refresh.

`use_otp_lut` (default `true`) controls where the waveform comes from:

- `true`: refresh with `0x22=0xFF/0xF7`, which loads the LUT (and temperature)
  from the controller's OTP. Requires OTP-programmed chips.
- `false`: write a hard-coded LUT (`0x32`) and VCOM (`0x2C=0x70`) during init
  and refresh with `0x22=0xC7`, like Adafruit's SSD1608 driver. Use this when
  the chips' OTP is blank — with a blank OTP the update sequence never
  finishes and BUSY stays high forever.

The `hink-e042a03-a1` model defaults to `use_otp_lut: false`: the SSD1608s on
that panel have a blank OTP. The generic `ssd1608-dual` model defaults to
`true`.

If one half of the panel is mounted rotated 180 degrees (the two COB chips
face each other), set `flip_cs` / `flip_cs1` to compensate.

The SSD1619A models take a single `cs_pin` (driven by the SPI bus), a
`dc_pin`, a `reset_pin` and a `busy_pin`; `cs1_pin`, `master`, `flip_*` and
`use_otp_lut` do not apply to them. A full refresh writes a fallback waveform
and runs `0x22=0xF7`, which loads the waveform and temperature from the OTP
when it is programmed. Partial refreshes
(`full_update_every` > 1) load a partial LUT explicitly, and the driver keeps
the RED RAM plane in sync with the displayed image so the differential
waveform has the correct previous frame. Deep sleep uses mode 1 (RAM
retained) when partial updates are enabled.

## Supported models

| model | panel | size |
| --- | --- | --- |
| `hink-e042a03-a1` | HINK-E042A03-A1 (dual SSD1608) | 400x300 mono |
| `ssd1619a` | generic SSD1619A panel | 400x300 mono |
| `hink-e042a13-a0` | HINK-E042A13-A0 (SSD1619A) | 400x300 mono |

`ssd1608-dual` is the generic model for dual-SSD1608 panels; give it explicit
`dimensions:` when using a different panel. `ssd1619a` already carries
400x300 dimensions; pass `dimensions:` to override them.

## Notes

- For the dual-SSD1608 models, both chip-selects are driven by the driver
  itself; `cs_pin` is removed from the config before the SPI bus sees it, so
  the bus never toggles it. The SSD1619A models use the normal bus-managed
  `cs_pin`.
- `AUTO_LOAD = ["epaper_spi", "split_buffer"]` makes the build copy the
  built-in base driver sources. The other drivers living in that component
  directory are compiled as well but dropped by the linker when unused.
- The C++ unit tests under `test/` are not picked up by ESPHome's core test
  harness (it only discovers `esphome/components/<name>`). They are kept here
  for a standalone gtest/PlatformIO run.
- `test/example.yaml` is a minimal compile smoke test:
  `esphome config components/epaper/test/example.yaml`.
