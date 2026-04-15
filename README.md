# esphome-components
The external components for esphome

To use these components, use the [external components](https://esphome.io/components/external_components.html).

Example:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/muxiaozi/esphome-components
    components: [ advanced_status_led ]
```

| Component | Description |
|-----------|-------------|
| advanced_status_led | 可以区分网络状态、API连接状态、MQTT连接状态以及错误和其他警告的状态指示器 |