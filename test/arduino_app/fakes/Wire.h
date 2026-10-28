/// @file Wire.h
/// @brief Minimal recording controller for the real application and real adapter.
#pragma once
#include <cstddef>
#include <cstdint>
class TwoWire {
 public:
  bool begin(int sda, int scl, uint32_t frequency);
  bool end();
  uint16_t getTimeOut();
  void setTimeOut(uint16_t value);
  void beginTransmission(uint8_t address);
  std::size_t write(uint8_t value);
  std::size_t write(const uint8_t* bytes, std::size_t length);
  uint8_t endTransmission(bool stop);
  std::size_t requestFrom(uint8_t address, std::size_t length, bool stop);
  int read();
  void flush();
};
extern TwoWire Wire;
