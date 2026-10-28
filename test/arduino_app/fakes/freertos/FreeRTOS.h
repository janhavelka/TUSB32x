/// @file FreeRTOS.h
/// @brief Single-threaded recording task declarations, not an RTOS emulator.
#pragma once
#include <cstdint>
#define configTICK_RATE_HZ 1000
struct StaticTask_t {};
using StackType_t = uint32_t;
using portMUX_TYPE = int;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(mux) static_cast<void>(mux)
#define portEXIT_CRITICAL(mux) static_cast<void>(mux)
#define portENTER_CRITICAL_ISR(mux) static_cast<void>(mux)
#define portEXIT_CRITICAL_ISR(mux) static_cast<void>(mux)
