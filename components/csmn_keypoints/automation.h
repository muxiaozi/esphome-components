#pragma once

#include "esphome/core/automation.h"
#include "csmn_keypoints.h"

namespace esphome {
namespace csmn_keypoints {

template<typename... Ts> class StartAction : public Action<Ts...>, public Parented<CsmnKeyPoints> {
  void play(const Ts &...x) override { this->parent_->start(); }
};

template<typename... Ts> class StopAction : public Action<Ts...>, public Parented<CsmnKeyPoints> {
  void play(const Ts &...x) override { this->parent_->stop(); }
};

template<typename... Ts> class ToggleAction : public Action<Ts...>, public Parented<CsmnKeyPoints> {
  void play(const Ts &...x) override { this->parent_->toggle(); }
};

}  // namespace csmn_keypoints
}  // namespace esphome
