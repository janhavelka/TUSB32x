/// @file esp_arduino_version.h
/// @brief Pinned API signatures for a recording host test; no SDK link claim.
#pragma once
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(3, 3, 11)
#define ESP_ARDUINO_VERSION_STR "3.3.11"
