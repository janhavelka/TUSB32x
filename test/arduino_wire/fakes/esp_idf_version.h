/// @file esp_idf_version.h
/// @brief Selected IDF API version for host contract compilation.
#pragma once
#define ESP_IDF_VERSION_VAL(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))
#define ESP_IDF_VERSION ESP_IDF_VERSION_VAL(5, 5, 5)
