# 志远继电器

一个通过 UART 以及自定义协议控制的继电器模块

Example:

```yaml

uart:
  id: uart_relay
  rx_pin: 5
  tx_pin: 17
  baud_rate: 9600

zhiyuan_relay:
  uart_id: uart_relay
  address: 1
  num_channels: 8

```
