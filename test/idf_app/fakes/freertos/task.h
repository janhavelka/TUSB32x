/// @file task.h
/// @brief Observe owner core, ISR context and finite yields without a host scheduler.
#pragma once
#include "FreeRTOS.h"
int xPortGetCoreID();
bool xPortInIsrContext();
void vTaskDelay(TickType_t ticks);
