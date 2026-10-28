/// @file task.h
/// @brief Observable execution core and ISR state without a host RTOS.
#pragma once
int xPortGetCoreID();
bool xPortInIsrContext();
