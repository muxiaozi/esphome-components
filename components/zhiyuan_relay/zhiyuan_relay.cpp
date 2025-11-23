#include "zhiyuan_relay.h"
#include "esphome/core/log.h"

namespace esphome {
namespace zhiyuan_relay {

static const char *const TAG = "zhiyuan_relay";
static const uint8_t ZHIYUAN_MSG_LEN = 8;

enum Cmd : uint16_t {
  Cmd_ReadState = 0x10,
  Cmd_SetOn = 0x12,
  Cmd_SetOff = 0x11,
};

static uint8_t zhiyuan_crc(uint8_t *buf, uint32_t len) {
  uint32_t crc = 0x0;
  for (uint8_t i = 0; i < len; i++) {
    crc += buf[i];
  }
  return crc & 0xFF;
}

void RelayChannel::write_state(bool state) {
  if (!this->get_parent()) {
    ESP_LOGW(TAG, "RelayChannel '%s' has no parent.", this->get_name().c_str());
    return;
  }
  if (this->get_parent()->write_channel_state(this->channel_, state)) {
    this->publish_state(state);
  }
}

void ZhiyuanRelay::dump_config() {
  ESP_LOGCONFIG(TAG, "Zhiyuan Relay:");
  ESP_LOGCONFIG(TAG, "  Address: %" PRIu8, this->address_);
  ESP_LOGCONFIG(TAG, "  Channels: %" PRIuMAX, this->channels_.size());
  for (RelayChannel *channel : this->channels_) {
    ESP_LOGCONFIG(TAG, "    Channel '%s': '%s'", channel->get_object_id().c_str(), channel->get_name().c_str());
  }
}

bool ZhiyuanRelay::write_channel_state(uint8_t channel, bool state) {
  return write_command_(this->address_, state ? Cmd_SetOn : Cmd_SetOff, channel, nullptr);
}

bool ZhiyuanRelay::write_command_(uint8_t addr, uint8_t func, uint32_t data, uint32_t *recv_data) {
  while (this->available())
    this->read();

  uint8_t buf[ZHIYUAN_MSG_LEN] = {0x55,
                                  addr,
                                  func,
                                  static_cast<uint8_t>(data >> 24 & 0xFF),
                                  static_cast<uint8_t>(data >> 16 & 0xFF),
                                  static_cast<uint8_t>(data >> 8 & 0xFF),
                                  static_cast<uint8_t>(data & 0xFF),
                                  0x00};
  buf[7] = zhiyuan_crc(buf, 7);
  ESP_LOGD(TAG, "request [%02x %02x %02x %02x %02x %02x %02x %02x]", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
           buf[6], buf[7]);
  this->write_array(buf, sizeof(buf));
  this->flush();

  if (!this->read_array(buf, ZHIYUAN_MSG_LEN)) {
    return false;
  }

  ESP_LOGD(TAG, "response [%02x %02x %02x %02x %02x %02x %02x %02x]", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
           buf[6], buf[7]);

  if (buf[0] != 0x22) {
    ESP_LOGE(TAG, "response head error");
    return false;
  }

  if (buf[7] != zhiyuan_crc(buf, 7)) {
    ESP_LOGE(TAG, "response crc error");
    return false;
  }

  if (recv_data) {
    *recv_data = (buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | buf[6];
  }

  return true;
}

}  // namespace zhiyuan_relay
}  // namespace esphome
