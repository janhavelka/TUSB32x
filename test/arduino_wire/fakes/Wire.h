/// @file Wire.h
/// @brief Recording host substitute; no claim about SDK linkage or physical I2C.
#pragma once
#include <cstddef>
#include <cstdint>

/// @brief Fixed scripted Wire surface used by the real adapter's host tests.
class TwoWire {
 public:
  uint16_t getTimeOut();
  void setTimeOut(uint16_t value);
  void beginTransmission(uint8_t address);
  size_t write(const uint8_t* data, size_t length);
  uint8_t endTransmission(bool stop);
  size_t requestFrom(uint8_t address, size_t length, bool stop);
  int read();
  void flush();
  bool begin();
  bool end();
};
