# CSMN Emitter Component

Example:

```yaml

uart:
  id: uart_main
  rx_pin: 6
  tx_pin: 7
  baud_rate: 38400

csmn_emitter:
  type: coil # coil, motor
  uart_id: uart_main
  trigger_pin:
    number: 4
    inverted: True
    mode:
      input: True
      pullup: True
  trigger_mode_pin:
    number: 5
    inverted: True
    mode:
      input: True
      pullup: True
  vibration_intensity: # 震动力度 (Number)
    name: "Vibration Intensity"
  num_bullets: # 子弹数量 (Number)
    name: "Number of Bullets"
  emitter_type: # 发射器类型 (Text Sensor)
    name: "Emitter Type"
  trigger_mode: # 触发模式 (Select)
    name: "Trigger Mode"
  trigger: # 扳机 (Binary Sensor)
    name: "Trigger"
  bolt: # 枪栓 (Binary Sensor)
    name: "Bolt"
  battery_level: # 电池电量 (Sensor)
    name: "Battery"
```
