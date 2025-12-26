#include "uploader.h"
#include "config.h"
#include "utils.h"
#include <freertos/FreeRTOS.h>
#include <esphome/core/util.h>
#include <esphome/core/log.h>
#ifdef USE_ETHERNET
#include <esphome/components/ethernet/ethernet_component.h>
#endif  // USE_ETHERNET
#ifdef USE_WIFI
#include <esphome/components/wifi/wifi_component.h>
#endif  // USE_WIFI

namespace esphome {
namespace csmn_keypoints {

static const char *TAG = "csmn_keypoints::uploader";
static const uint32_t UPLOAD_EVENT_QUEUE_LENGTH = 32;

enum UploadEventType {
  UPLOAD_EVENT_CREATE_RECORD,
  UPLOAD_EVENT_UPLOAD_FILE,
};

struct UploadEvent {
  UploadEventType type;
  uint32_t record_id;
};

enum State {
  STATE_WAIT_FOR_NETWORK,
  STATE_REGISTER_DEVICE,
  STATE_UPLOAD_RECORD,
};

class FailCounter {
 public:
  FailCounter(uint32_t max_failures = 3) : max_failures_(max_failures) {}

  void set_record_id(uint32_t record_id) {
    if (this->record_id_ != record_id) {
      this->record_id_ = record_id;
      this->count_ = 0;
    }
  }

  bool increment() {
    this->count_++;
    return this->count_ >= this->max_failures_;
  }

 private:
  uint32_t record_id_{0};
  uint32_t count_{0};
  uint32_t max_failures_{3};
};

bool network_connected() {
#ifdef USE_WIFI
  if (wifi::global_wifi_component && wifi::global_wifi_component->is_connected()) {
    return true;
  }
#endif  // USE_WIFI
#ifdef USE_ETHERNET
  if (ethernet::global_eth_component && ethernet::global_eth_component->is_connected()) {
    return true;
  }
#endif  // USE_ETHERNET
  return false;
}

void do_upload_record(Uploader *uploader) {
  FailCounter fail_counter;
  while (network_connected()) {
    std::optional<uint32_t> record_id = global_csmn_keypoints_config->next_upload_index();
    if (!record_id.has_value()) {
      vTaskDelay(pdMS_TO_TICKS(10 * 1000));
      continue;
    }

    fail_counter.set_record_id(record_id.value());

    ApiError api_err;

    // 1. create record
    ESP_LOGD(TAG, "Create record ID %u", record_id.value());
    api_err = uploader->api_->create_record(record_id.value());
    if (api_err == API_ERROR_NETWORK) {
      break;
    } else if (api_err == API_ERROR_NOT_FOUND) {
      ESP_LOGW(TAG, "Record ID %u not found, marking as skipped", record_id.value());
      global_csmn_keypoints_config->set_record_flag(record_id.value(), FLAG_SKIP_UPLOAD);
      continue;
    } else if (api_err != API_ERROR_OK) {
      ESP_LOGW(TAG, "Failed to create record ID %u", record_id.value());
      if (fail_counter.increment()) {
        ESP_LOGW(TAG, "Exceeded maximum retries for record ID %u, skipping", record_id.value());
        global_csmn_keypoints_config->set_record_flag(record_id.value(), FLAG_SKIP_UPLOAD);
      }
      continue;
    } else {
      ESP_LOGI(TAG, "Successfully created record ID %u", record_id.value());
    }

    // 2. upload file
    std::string filepath = uploader->sdcard_->get_filepath(record_id.value());
    ESP_LOGD(TAG, "Uploading record ID %u from file %s", record_id.value(), filepath.c_str());
    api_err = uploader->api_->upload_record_audio(record_id.value(), filepath);
    if (api_err == API_ERROR_NETWORK) {
      break;
    } else if (api_err == API_ERROR_NOT_FOUND) {
      ESP_LOGW(TAG, "File for record ID %u not found, marking as skipped", record_id.value());
      global_csmn_keypoints_config->set_record_flag(record_id.value(), FLAG_SKIP_UPLOAD);
      continue;
    } else if (api_err != API_ERROR_OK) {
      ESP_LOGW(TAG, "Failed to upload record ID %u", record_id.value());
      if (fail_counter.increment()) {
        ESP_LOGW(TAG, "Exceeded maximum retries for record ID %u, skipping", record_id.value());
        global_csmn_keypoints_config->set_record_flag(record_id.value(), FLAG_SKIP_UPLOAD);
      }
      continue;
    } else {
      ESP_LOGI(TAG, "Successfully uploaded record ID %u", record_id.value());
      global_csmn_keypoints_config->set_record_flag(record_id.value(), FLAG_UPLOADED);
    }
  }
}

State do_state(State state, Uploader *uploader) {
  switch (state) {
    case STATE_WAIT_FOR_NETWORK: {
      ESP_LOGD(TAG, "STATE_WAIT_FOR_NETWORK");
      while (true) {
        if (network_connected()) {
          break;
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
      }
      return STATE_REGISTER_DEVICE;
    }
    case STATE_REGISTER_DEVICE: {
      ESP_LOGD(TAG, "STATE_REGISTER_DEVICE");
      ApiError api_err = uploader->api_->register_device();
      if (api_err == API_ERROR_OK) {
        ESP_LOGI(TAG, "Device registration successful");
        return STATE_UPLOAD_RECORD;
      } else {
        ESP_LOGW(TAG, "Device registration failed");
        return STATE_WAIT_FOR_NETWORK;
      }
    }
    case STATE_UPLOAD_RECORD: {
      ESP_LOGD(TAG, "STATE_UPLOAD_RECORD");
      do_upload_record(uploader);
      return STATE_WAIT_FOR_NETWORK;
    }
    default: {
      ESP_LOGD(TAG, "Unknown state: %d", state);
      return STATE_WAIT_FOR_NETWORK;
    }
  }
  return state;
}

void upload_task(void *pvParameters) {
  auto *uploader = static_cast<Uploader *>(pvParameters);
  State state = STATE_WAIT_FOR_NETWORK;
  while (true) {
    state = do_state(state, uploader);
  }
  vTaskDelete(nullptr);
}

bool Uploader::setup() {
  BaseType_t result = xTaskCreate(upload_task, "upload_task", 8192, this, tskIDLE_PRIORITY + 1, nullptr);
  return result == pdPASS;
}

}  // namespace csmn_keypoints
}  // namespace esphome