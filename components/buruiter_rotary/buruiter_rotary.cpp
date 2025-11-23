#include "buruiter_rotary.h"
#include "esphome/core/log.h"

namespace esphome {
namespace buruiter_rotary {

static const char *const TAG = "buruiter_rotary";

void BuruiterRotary::setup() {
  // BuruiterRotary 的该消息非标准
  // this->queue_command(ModbusCommandItem::info());
}

void BuruiterRotary::loop() {
  if (!this->incomming_queue_.empty()) {
    auto command = std::move(this->incomming_queue_.front());
    on_modbus_response(*command);
    this->incomming_queue_.pop();
  } else if (!this->command_queue_.empty() && !this->waiting_for_response()) {
    auto &command = this->command_queue_.front();
    this->send_raw(command->build(this->address_));
  }
}

void BuruiterRotary::dump_config() {
  ESP_LOGCONFIG(TAG, "BuruiterRotary:");
  ESP_LOGCONFIG(TAG, "  Address: %" PRIu8, this->address_);
  ESP_LOGCONFIG(TAG, "  Resolution: %" PRIu32, this->resolution_);
}

void BuruiterRotary::on_modbus_data(const std::vector<uint8_t> &data) {
  if (this->command_queue_.empty())
    return;
  auto command = std::move(this->command_queue_.front());
  command->payload_ = data;
  this->command_queue_.pop_front();
  this->incomming_queue_.push(std::move(command));
}

void BuruiterRotary::update() {
  this->queue_command(ModbusCommandItem::single_cycle());
  this->queue_command(ModbusCommandItem::angular_velocity());
}

void BuruiterRotary::queue_command(const ModbusCommandItem &command) {
  for (auto &item : this->command_queue_) {
    if (item->function_ == command.function_ && item->address_ == command.address_) {
      item->payload_ = command.payload_;
      return;
    }
  }
  this->command_queue_.push_back(std::make_unique<ModbusCommandItem>(command));
}

void BuruiterRotary::on_modbus_response(const ModbusCommandItem &command) {
  switch (command.address_) {
    case Address_Info: {
      if (command.payload_.size() == 4) {
        this->resolution_ = (command.payload_[2] << 8) | command.payload_[3];
        ESP_LOGD(TAG, "Device info - Mode: %d, Direction: %d, Resolution: %d", command.payload_[0], command.payload_[1],
                 this->resolution_);
      }
      ESP_LOGI(TAG, "BuruiterRotary initialized with resolution %d", this->resolution_);
      break;
    }
    case Address_SingleCycle: {
      if (command.payload_.size() == 2) {
        uint16_t value = (command.payload_[0] << 8) | command.payload_[1];
        if (this->last_single_cycle_ != value) {
          this->last_single_cycle_ = value;
          this->single_cycle_sensor_->publish_state(value * 360.0f / this->resolution_);
        }
      }
      break;
    }
    case Address_AngularVelocity: {
      if (command.payload_.size() == 4) {
        int32_t value = command.payload_.data()[0] << 24 | command.payload_.data()[1] << 16 |
                         command.payload_.data()[2] << 8 | command.payload_.data()[3];
        if (this->last_angular_velocity_ != value) {
          this->last_angular_velocity_ = value;
          this->angular_velocity_sensor_->publish_state((float) value / this->resolution_ / (0.1 / 60));
        }
      }
      break;
    }
    case Address_Direction: {
      if (command.payload_.size() == 4) {
        bool cw = (command.payload_[3] == 0);
        this->direction_select_->publish_state(cw ? "CW" : "CCW");
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace buruiter_rotary
}  // namespace esphome
