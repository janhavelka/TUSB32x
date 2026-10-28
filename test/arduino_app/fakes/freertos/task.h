/// @file task.h
/// @brief Record worker creation; tests pump its mailbox explicitly without spawning it.
#pragma once
#include "FreeRTOS.h"
#include <cstddef>
int xPortGetCoreID();
bool xPortInIsrContext();
void vTaskDelay(unsigned ticks);
StaticTask_t* xTaskCreateStatic(void (*entry)(void*), const char* name,
    std::size_t bytes, void* context, unsigned priority, StackType_t* stack,
    StaticTask_t* task);
