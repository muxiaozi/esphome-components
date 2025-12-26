#pragma once

#include <string>
#include <esphome/components/http_request/http_request_idf.h>
#include "sdcard.h"

namespace esphome {
namespace csmn_keypoints {

enum ApiError {
  API_ERROR_OK = 0,
  API_ERROR_NETWORK,
  API_ERROR_HTTP_CODE,
  API_ERROR_RESPONSE_CODE,
  API_ERROR_NOT_FOUND,
};

class Api {
 public:
  ApiError register_device();
  ApiError create_record(uint32_t record_id);
  ApiError upload_record_audio(uint32_t record_id, const std::string &filepath);

  void set_base_url(const std::string &base_url) {
    this->base_url_ = base_url;
  }

  void set_http_request(http_request::HttpRequestComponent *http_request) {
    this->http_request_ = static_cast<http_request::HttpRequestIDF *>(http_request);
  }

  void set_uuid(const std::string &uuid) {
    this->uuid_ = uuid;
  }

  void set_sdcard(class Sdcard *sdcard) {
    this->sdcard_ = sdcard;
  }

 private:
  http_request::HttpRequestIDF *http_request_{nullptr};
  std::string base_url_;
  std::string uuid_;
  Sdcard *sdcard_{nullptr};
};

}  // namespace csmn_keypoints
}  // namespace esphome