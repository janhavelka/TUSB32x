/// @file task.h
/// @brief Observable task/core context without an RTOS dependency in the host fixture.
#pragma once
int xPortGetCoreID();
bool xPortInIsrContext();
