/// @file esp_timer.h
/// @brief External physical microsecond clock advanced independently of the adapter.
#pragma once
#include <cstdint>
int64_t esp_timer_get_time();
