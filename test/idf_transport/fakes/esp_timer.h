/// @file esp_timer.h
/// @brief External microsecond clock advanced by recorded backend events.
#pragma once
#include <cstdint>
int64_t esp_timer_get_time();
