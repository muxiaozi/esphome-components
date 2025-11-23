#pragma once

#include "esphome/core/component.h"
#include "esphome/components/modbus/modbus.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/button/button.h"
#include "esphome/components/select/select.h"

#include <cinttypes>
#include <vector>
#include <queue>
#include <memory>
#include <list>

namespace esphome {
namespace buruiter_rotary {

enum Function : uint8_t {
  Function_ReadHoldingRegisters = 0x03,
  Function_WriteSingleRegister = 0x06,
};

enum Address : uint16_t {
  Address_SingleCycle = 0x0,      // 3
  Address_AngularVelocity = 0x20,  // 3
  Address_BaudRate = 0x5,         // 6 (0x0: 9600, 0x01: 19200, 0x02: 38400, 0x03: 57600, 0x04: 115200)
  Address_Mode = 0x6,  // 6 (0x00：查询模式 0x01：自动回传编码 器单圈值 0x04：自动回传编码 器虚拟多圈值
                       // 0x05：自动回传编码 器角速度值)
  Address_ResetZero = 0x8,    // 6 (0x01: 当前位置设置为零点)
  Address_Direction = 0x9,    // 6 (0x00：顺时针 0x01：逆时针)
  Address_ResetCenter = 0xE,  // 6 (0x01: 当前位置设置为中点)
  Address_Info = 0x88, // 3 (返回4个寄存器，分别为：未知？、模式、方向、精度)
};

class ModbusCommandItem {
 public:
  static ModbusCommandItem info() {
    ModbusCommandItem item;
    item.function_ = Function_ReadHoldingRegisters;
    item.address_ = Address_Info;
    item.payload_ = {0, 2};
    return item;
  }

  static ModbusCommandItem single_cycle() {
    ModbusCommandItem item;
    item.function_ = Function_ReadHoldingRegisters;
    item.address_ = Address_SingleCycle;
    item.payload_ = {0, 1};
    return item;
  }

  static ModbusCommandItem angular_velocity() {
    ModbusCommandItem item;
    item.function_ = Function_ReadHoldingRegisters;
    item.address_ = Address_AngularVelocity;
    item.payload_ = {0, 2};
    return item;
  }

  static ModbusCommandItem direction(bool cw) {
    ModbusCommandItem item;
    item.function_ = Function_WriteSingleRegister;
    item.address_ = Address_Direction;
    if (cw) {
      item.payload_ = {0, 0};
    } else {
      item.payload_ = {0, 1};
    }
    return item;
  }

  static ModbusCommandItem reset_zero() {
    ModbusCommandItem item;
    item.function_ = Function_WriteSingleRegister;
    item.address_ = Address_ResetZero;
    item.payload_ = {0, 1};
    return item;
  }

  static ModbusCommandItem reset_center() {
    ModbusCommandItem item;
    item.function_ = Function_WriteSingleRegister;
    item.address_ = Address_ResetCenter;
    item.payload_ = {0, 1};
    return item;
  }

  std::vector<uint8_t> build(uint8_t slave_addr) const {
    auto result = std::vector<uint8_t>{
        slave_addr,
        this->function_,
        static_cast<uint8_t>(this->address_ >> 8 & 0xFF),
        static_cast<uint8_t>(this->address_ & 0xFF),
    };
    result.push_back(this->payload_[0]);
    result.push_back(this->payload_[1]);
    return result;
  }

 public:
  uint8_t function_;
  uint16_t address_;
  std::vector<uint8_t> payload_;
};

class BuruiterRotary : public modbus::ModbusDevice, public PollingComponent {
#ifdef USE_SELECT
  SUB_SELECT(direction)
#endif
#ifdef USE_SENSOR
  SUB_SENSOR(single_cycle)
  SUB_SENSOR(angular_velocity)
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(reset_zero)
  SUB_BUTTON(reset_center)
#endif

 public:
  // Component
  void setup() override;
  void loop() override;
  void dump_config() override;

  // modbus::ModbusDevice
  void on_modbus_data(const std::vector<uint8_t> &data) override;

  // PollingComponent
  void update() override;

 public:
  void queue_command(const ModbusCommandItem &command);

  void set_resolution(uint32_t resolution) { this->resolution_ = resolution; }

 protected:
  void on_modbus_response(const ModbusCommandItem &command);

  uint16_t last_single_cycle_{0};
  int32_t last_angular_velocity_{0};
  uint32_t resolution_{1024};

  std::list<std::unique_ptr<ModbusCommandItem>> command_queue_;
  std::queue<std::unique_ptr<ModbusCommandItem>> incomming_queue_;
};

class DirectionSelect : public select::Select, public Parented<BuruiterRotary> {
 protected:
  void control(const std::string &value) override {
    if (value == "CW") {
      this->get_parent()->queue_command(ModbusCommandItem::direction(true));
    } else if (value == "CCW") {
      this->get_parent()->queue_command(ModbusCommandItem::direction(false));
    }
  }
};

class ResetZeroButton : public button::Button, public Parented<BuruiterRotary> {
 protected:
  void press_action() override { this->get_parent()->queue_command(ModbusCommandItem::reset_zero()); }
};

class ResetCenterButton : public button::Button, public Parented<BuruiterRotary> {
 protected:
  void press_action() override { this->get_parent()->queue_command(ModbusCommandItem::reset_center()); }
};

}  // namespace buruiter_rotary
}  // namespace esphome
