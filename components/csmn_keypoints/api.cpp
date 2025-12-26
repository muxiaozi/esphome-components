#include "api.h"
#include "config.h"
#include "utils.h"
#include <esphome/components/json/json_util.h>

#define TAG "csmn_keypoints::api"

#define HTTP_READ_BUFFER_SIZE 256

namespace esphome {
namespace csmn_keypoints {

ApiError Api::register_device() {
  std::string body = json::build_json([this](JsonObject root) {
    root["id"] = this->uuid_;
    root["name"] = get_name();
    root["wifi_mac"] = get_wifi_mac();
    root["flash_size"] = get_flash_size_bytes();
    root["ram_size"] = get_total_heap_size_bytes();
    root["chip_id"] = get_chip_id();
    root["reset_reason"] = get_reset_reason();
    root["idf_version"] = get_idf_version();
    root["firmware_version"] = get_firmware_version();
    root["manufacturer"] = get_manufacturer();
    root["model"] = get_model();
    root["description"] = get_description();
  });

  std::list<http_request::Header> headers = {{"Content-Type", "application/json"},
                                             {"Accept", "application/json"},
                                             {"Content-Length", std::to_string(body.length())}};

  std::string url = this->base_url_ + "/devices";
  auto container = this->http_request_->post(url, body, headers);
  if (!container) {
    ESP_LOGE(TAG, "Device registration failed: No response");
    return API_ERROR_NETWORK;
  }

  if (container->status_code != 200) {
    ESP_LOGE(TAG, "Device registration failed: HTTP %d", container->status_code);
    return API_ERROR_HTTP_CODE;
  }

  std::string response_body;
  int bytes_read = 0;
  while (bytes_read < container->content_length) {
    char buf[HTTP_READ_BUFFER_SIZE];
    int read = container->read((uint8_t *) buf, sizeof(buf));
    if (read <= 0) {
      break;
    }
    response_body.append(buf, read);
    bytes_read += read;
  }
  container->end();

  bool result = json::parse_json(response_body, [](JsonObject root) {
    int code = root["code"].as<int>();
    std::string msg = root["msg"].as<std::string>();

    if (code != 0) {
      ESP_LOGE(TAG, "Device creation failed: (%d) %s", code, msg.c_str());
      return false;
    }

    ESP_LOGI(TAG, "Device creation succeeded: %s", msg.c_str());
    return true;
  });

  if (!result) {
    return API_ERROR_RESPONSE_CODE;
  }

  return API_ERROR_OK;
}

ApiError Api::create_record(uint32_t record_id) {
  RecordConfig record_config{};
  if (!global_csmn_keypoints_config->get_record_config(record_id, &record_config)) {
    ESP_LOGE(TAG, "Failed to get record config for record ID %u", record_id);
    return API_ERROR_NOT_FOUND;
  }

  std::string body = json::build_json([this, &record_config, record_id](JsonObject root) {
    root["index"] = record_id;
    root["begin_time"] = record_config.start_time;
    root["duration_seconds"] = record_config.duration_seconds;
    root["size_bytes"] = record_config.size_bytes;
    root["crc16"] = record_config.crc16;
  });

  std::list<http_request::Header> headers = {{"Content-Type", "application/json"},
                                             {"Accept", "application/json"},
                                             {"Content-Length", std::to_string(body.length())}};

  std::string url = this->base_url_ + "/devices/" + this->uuid_ + "/records";
  auto container = this->http_request_->post(url, body, headers);

  if (!container) {
    ESP_LOGE(TAG, "Record creation failed: No response");
    return API_ERROR_NETWORK;
  }

  if (container->status_code != 200) {
    ESP_LOGE(TAG, "Record creation failed: HTTP %d", container->status_code);
    return API_ERROR_HTTP_CODE;
  }

  std::string response_body;
  int bytes_read = 0;
  while (bytes_read < container->content_length) {
    char buf[HTTP_READ_BUFFER_SIZE];
    int read = container->read((uint8_t *) buf, sizeof(buf));
    if (read <= 0) {
      break;
    }
    response_body.append(buf, read);
    bytes_read += read;
  }
  container->end();

  bool result = json::parse_json(response_body, [](JsonObject root) {
    int code = root["code"].as<int>();
    std::string msg = root["msg"].as<std::string>();

    if (code != 0) {
      ESP_LOGE(TAG, "Record creation failed: (%d) %s", code, msg.c_str());
      return false;
    }

    ESP_LOGI(TAG, "Record creation succeeded: %s", msg.c_str());
    return true;
  });

  if (!result) {
    return API_ERROR_RESPONSE_CODE;
  }

  return API_ERROR_OK;
}

ApiError Api::upload_record_audio(uint32_t record_id, const std::string &filepath) {
  if (!file_exists(filepath)) {
    ESP_LOGE(TAG, "File %s does not exist", filepath.c_str());
    return API_ERROR_NOT_FOUND;
  }

  std::list<http_request::Header> headers = {
      {"Accept", "application/json"},
  };
  std::string url = this->base_url_ + "/devices/" + this->uuid_ + "/records/" + std::to_string(record_id) + "/audio";

  auto container = this->http_request_->perform_upload(url, filepath, headers);

  if (!container) {
    ESP_LOGE(TAG, "Record upload failed: No response");
    return API_ERROR_NETWORK;
  }

  if (container->status_code != 200) {
    ESP_LOGE(TAG, "Record upload failed: HTTP %d", container->status_code);
    return API_ERROR_HTTP_CODE;
  }

  std::string response_body;
  int bytes_read = 0;
  while (bytes_read < container->content_length) {
    char buf[HTTP_READ_BUFFER_SIZE];
    int read = container->read((uint8_t *) buf, sizeof(buf));
    if (read <= 0) {
      break;
    }
    response_body.append(buf, read);
    bytes_read += read;
  }
  container->end();

  bool result = json::parse_json(response_body, [](JsonObject root) {
    int code = root["code"].as<int>();
    std::string msg = root["msg"].as<std::string>();

    if (code != 0) {
      ESP_LOGE(TAG, "Record upload failed: (%d) %s", code, msg.c_str());
      return false;
    }

    ESP_LOGI(TAG, "Record upload succeeded: %s", msg.c_str());
    return true;
  });

  if (!result) {
    return API_ERROR_RESPONSE_CODE;
  }

  return API_ERROR_OK;
}

}  // namespace csmn_keypoints
}  // namespace esphome