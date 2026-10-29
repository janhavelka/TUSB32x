/// @file Arduino.h
/// @brief Recording application SDK surface; no firmware or electrical qualification.
#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#define ARDUINO_ISR_ATTR
#define INPUT 1
#define LOW 0
#define FALLING 2
/// @brief Host serial queues; all operations return immediately.
struct RecordingSerial {
  static constexpr std::size_t RX_CAPACITY = 256;
  std::string input;
  std::string output;
  std::string arrivalBeforeRead;
  std::size_t droppedInput = 0;
  bool connected = true;
  bool stalled = false;
  unsigned beginCalls = 0;
  void begin(unsigned long) { ++beginCalls; }
  void setTxTimeoutMs(uint32_t) {}
  void enableReboot(bool) {}
  explicit operator bool() const { return connected; }
  int available() const { return static_cast<int>(input.size()); }
  /// @brief Model the pinned SDK queue retaining its prefix and dropping the packet suffix.
  void receive(const std::string& bytes) {
    const std::size_t room = RX_CAPACITY - input.size();
    const std::size_t accepted = bytes.size() < room ? bytes.size() : room;
    input.append(bytes.data(), accepted);
    droppedInput += bytes.size() - accepted;
  }
  int read() {
    if (!arrivalBeforeRead.empty()) {
      receive(arrivalBeforeRead);
      arrivalBeforeRead.clear();
    }
    if (input.empty()) return -1;
    const unsigned char byte = static_cast<unsigned char>(input.front());
    input.erase(0, 1);
    return byte;
  }
  int availableForWrite() const { return stalled ? 0 : 256; }
  std::size_t write(const uint8_t* bytes, std::size_t length) {
    output.append(reinterpret_cast<const char*>(bytes), length);
    return length;
  }
};
extern RecordingSerial Serial;
void pinMode(int pin, int mode);
int digitalRead(int pin);
void attachInterrupt(int pin, void (*handler)(), int mode);
void detachInterrupt(int pin);
