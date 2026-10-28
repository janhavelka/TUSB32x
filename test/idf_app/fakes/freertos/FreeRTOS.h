/// @file FreeRTOS.h
/// @brief Finite recording task definitions, with no host scheduler.
#pragma once
#include <cstddef>
#include <cstdint>
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ 1000
#endif
using TickType_t = uint32_t;
using BaseType_t = int;
using UBaseType_t = unsigned;
constexpr BaseType_t pdTRUE = 1;
#define pdMS_TO_TICKS(ms) ((static_cast<TickType_t>(ms) * configTICK_RATE_HZ) / 1000)
