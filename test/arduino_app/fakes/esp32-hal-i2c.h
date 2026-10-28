/// @file esp32-hal-i2c.h
/// @brief Observe controller ownership without initializing it.
#pragma once
#include <cstdint>
bool i2cIsInit(uint8_t controller);
