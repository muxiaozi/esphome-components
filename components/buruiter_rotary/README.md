# 布瑞特编码器

Example:

```yaml

uart:
  id: uart_modbus_client
  rx_pin: GPIO5
  tx_pin: GPIO17
  baud_rate: 9600

modbus:
  id: modbus_rotary
  uart_id: uart_modbus_client
  flow_control_pin: GPIO33
  disable_crc: False
  role: CLIENT

buruiter_rotary:
  modbus_id: modbus_rotary
  address: 1
  resolution: 10
  single_cycle:
    name: "Single Cycle"
  angular_velocity:
    name: "Angular Velocity"
  direction:
    name: "Direction"
  reset_zero:
    name: "Reset Zero"
  reset_center:
    name: "Reset Center"
  update_interval: 50ms

```
