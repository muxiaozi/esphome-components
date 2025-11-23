# Advanced Status LED Component

[Demo](https://pan.cheshuimanong.com/f/9g0f3/advanced_status_led_demo.mp4)

Example:

```yaml

output:
- platform: ledc
  id: output_status_led
  pin: GPIO4
  frequency: 1220

light:
- platform: advanced_status_led
  id: light_status_led
  output: output_status_led
  effects:
  - pulse:
      name: Error
      transition_length: 0s
      update_interval: 0.1s
  - pulse:
      name: Warning
      transition_length: 0s
      update_interval: 0.5s
  - strobe:
      name: No Network
      colors:
      - state: true
        brightness: 100%
        duration: 100ms
      - state: false
        duration: 1s
  - strobe:
      name: No Mqtt Connection
      colors:
      - state: true
        brightness: 100%
        duration: 100ms
      - state: false
        duration: 100ms
      - state: true
        brightness: 100%
        duration: 100ms
      - state: false
        duration: 1s
  - pulse:
      name: No Api Connection
      transition_length: 1s
      update_interval: 1s

```
