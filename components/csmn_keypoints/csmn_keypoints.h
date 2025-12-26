#pragma once

#include <esphome/core/component.h>
#include <esphome/core/gpio.h>
#include <esphome/components/http_request/http_request.h>
#include <esphome/components/microphone/microphone.h>
#include <memory>

#include "config.h"
#include "sdcard.h"
#include "uploader.h"
#include "api.h"
#include "utils.h"

namespace esphome {
namespace csmn_keypoints {

class CsmnKeyPoints : public Component {
 public:
  CsmnKeyPoints();

  void setup() override;
  void loop() override;
  void dump_config() override;

  void start();
  void stop();
  void toggle() {
    if (this->recording_) {
      this->stop();
    } else {
      this->start();
    }
  }

  void set_http_request(http_request::HttpRequestComponent *http_request) {
    this->http_request_ = http_request;
    this->api_->set_http_request(http_request);
  }
  void set_microphone(microphone::Microphone *microphone) { this->microphone_ = microphone; }
  void set_miso_pin(GPIOPin *miso) { this->miso_pin_ = static_cast<InternalGPIOPin *>(miso); }
  void set_mosi_pin(GPIOPin *mosi) { this->mosi_pin_ = static_cast<InternalGPIOPin *>(mosi); }
  void set_clk_pin(GPIOPin *clk) { this->clk_pin_ = static_cast<InternalGPIOPin *>(clk); }
  void set_cs_pin(GPIOPin *cs) { this->cs_pin_ = static_cast<InternalGPIOPin *>(cs); }
  void set_base_url(const std::string &base_url) {
    this->base_url_ = base_url;
    this->api_->set_base_url(base_url);
  }
  void set_uuid(const std::string &uuid) {
    this->uuid_ = uuid;
    this->api_->set_uuid(uuid);
  }

 private:
  http_request::HttpRequestComponent *http_request_{nullptr};
  microphone::Microphone *microphone_{nullptr};
  InternalGPIOPin *miso_pin_{nullptr};
  InternalGPIOPin *mosi_pin_{nullptr};
  InternalGPIOPin *clk_pin_{nullptr};
  InternalGPIOPin *cs_pin_{nullptr};
  bool recording_{false};
  std::unique_ptr<Sdcard> sdcard_{};
  std::unique_ptr<Uploader> uploader_{};
  std::unique_ptr<Api> api_{};
  std::string base_url_;
  std::string uuid_;
};

}  // namespace csmn_keypoints
}  // namespace esphome
