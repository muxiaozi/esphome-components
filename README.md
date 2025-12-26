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
| advanced_status_led | A status indicator that can distinguish network status, API connection status, MQTT connection status, as well as errors and other warnings |
| zhiyuan_relay | 志远继电器 |
| buruiter_rotary | 布瑞特旋转编码器 |
| csmn_keypoints | 划重点组件，用于关键点记录和管理 |