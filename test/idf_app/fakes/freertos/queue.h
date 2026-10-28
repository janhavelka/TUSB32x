/// @file queue.h
/// @brief Finite UART-event recording queue; no host allocation or scheduler.
#pragma once
#include "FreeRTOS.h"
struct RecordingQueue;
using QueueHandle_t = RecordingQueue*;
BaseType_t xQueueReceive(QueueHandle_t queue, void* item, TickType_t ticks);
UBaseType_t uxQueueMessagesWaiting(QueueHandle_t queue);
