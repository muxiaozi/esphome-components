#pragma once

#include "api.h"
#include "sdcard.h"

namespace esphome {
namespace csmn_keypoints {

class Uploader {
 public:
  bool setup();

  void set_api(Api *api) {
    this->api_ = api;
  }

  void set_sdcard(Sdcard *sdcard) {
    this->sdcard_ = sdcard;
  }

 public:
  Api *api_{nullptr};
  Sdcard *sdcard_{nullptr};
};

}  // namespace csmn_keypoints
}  // namespace esphome