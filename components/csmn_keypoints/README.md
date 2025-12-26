# CSMN Key Points

Example:

```yaml
i2s_audio:
  i2s_lrclk_pin: GPIO18
  i2s_bclk_pin: GPIO17

microphone:
- platform: i2s_audio
  id: external_mic
  adc_type: external
  i2s_din_pin: GPIO16
  pdm: false
  channel: left
  use_apll: true
  bits_per_sample: 16bit

csmn_keypoints:
  id: keypoints_recorder
  base_url: "https://open.cheshuimanong.com/keypoints/v1"
  uuid: c6235286-19e0-416e-bd22-1f774bcdfa6e
  miso_pin: GPIO13
  mosi_pin: GPIO11
  cs_pin: GPIO10
  clk_pin: GPIO12

binary_sensor:
- platform: gpio
  name: System Button
  pin:
    number: GPIO5
    inverted: True
    mode:
      input: True
      pullup: True
  internal: True
  on_click:
  - logger.log: "Button clicked! Toggling keypoints recorder."
  - csmn_keypoints.toggle: keypoints_recorder
```
