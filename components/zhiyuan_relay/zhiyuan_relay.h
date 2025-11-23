#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/switch/switch.h"

#include <cinttypes>
#include <vector>

namespace esphome {
namespace zhiyuan_relay {

class ZhiyuanRelay;

class RelayChannel : public switch_::Switch, public Parented<ZhiyuanRelay> {
 public:
  RelayChannel(uint8_t channel) : channel_(channel) {}

 protected:
  void write_state(bool state) override;
  
  uint8_t channel_{0};
};

class ZhiyuanRelay : public uart::UARTDevice, public Component {
 public:
  void dump_config() override;

  void add_channel(RelayChannel *channel) { this->channels_.push_back(channel); }

  void set_address(uint8_t address) { this->address_ = address; }

  bool write_channel_state(uint8_t channel, bool state);

 protected:
  bool write_command_(uint8_t addr, uint8_t func, uint32_t data, uint32_t *recv_data);
  
  std::vector<RelayChannel *> channels_;
  uint8_t address_;
};

}  // namespace zhiyuan_relay
}  // namespace esphome
