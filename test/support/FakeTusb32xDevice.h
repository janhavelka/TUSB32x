/// @file FakeTusb32xDevice.h
/// @brief Fixed scripted terminal callbacks; deliberately not a silicon emulator.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <TUSB32x/TUSB32x.h>

namespace tusb32x_test {

/// @brief Exact callback expectations and explicit fault injection without allocation.
/// @details Scripts use TI-derived bytes supplied by tests, never production register helpers.
/// Register mutations represent possible acceptance only; no reset, W1C or timing model is implied.
struct FakeTusb32xDevice {
  enum class Kind : uint8_t { WRITE, READ };

  /// @brief One complete callback contract, including optional partial receive/error effects.
  struct Step {
    Kind kind = Kind::READ;
    uint8_t address = 0x47;
    uint8_t tx[2] = {};
    std::size_t txLength = 1;
    uint8_t rx[11] = {};
    std::size_t rxLength = 1;
    std::size_t fillLength = 1;
    uint32_t timeoutMs = 20;
    TUSB32x::Status status = TUSB32x::Status::Ok();
    bool applyWrite = false;
    const TUSB32x::TransportStats* observedStats = nullptr;
    uint32_t expectedAttempts = 0;
    uint32_t expectedSuccess = 0;
    uint32_t expectedFailures = 0;
    const uint8_t* callerOutput = nullptr;
  };

  Step script[8] = {}; ///< Fixed script storage; an overfull script is a fixture error.
  std::size_t stepCount = 0;
  std::size_t calls = 0;
  unsigned int errors = 0;
  uint8_t registers[256] = {}; ///< Only explicitly applied writes change this neutral storage.

  /// @brief Append an expected pointer/read callback and return its editable fault controls.
  Step& expectRead(uint8_t address, uint8_t reg, std::size_t length, uint32_t timeoutMs = 20) {
    auto& step = append();
    step.kind = Kind::READ;
    step.address = address;
    step.tx[0] = reg;
    step.txLength = 1;
    step.rxLength = length;
    step.fillLength = length;
    step.timeoutMs = timeoutMs;
    return step;
  }

  /// @brief Append an expected exact two-byte write, with no mutation unless requested.
  Step& expectWrite(uint8_t address, uint8_t reg, uint8_t value, uint32_t timeoutMs = 20) {
    auto& step = append();
    step.kind = Kind::WRITE;
    step.address = address;
    step.tx[0] = reg;
    step.tx[1] = value;
    step.txLength = 2;
    step.rxLength = 0;
    step.fillLength = 0;
    step.timeoutMs = timeoutMs;
    return step;
  }

  /// @brief True only when all expectations ran exactly once with no fixture disagreement.
  bool complete() const { return errors == 0 && calls == stepCount; }

  /// @brief Terminal write entry point; user must point to this live fixture.
  static TUSB32x::Status write(uint8_t address, const uint8_t* tx, std::size_t txLength,
                               uint32_t timeoutMs, void* user) {
    auto& fake = *static_cast<FakeTusb32xDevice*>(user);
    return fake.invoke(Kind::WRITE, address, tx, txLength, nullptr, 0, timeoutMs);
  }

  /// @brief Terminal pointer/read entry point with exact receive length and scratch isolation.
  static TUSB32x::Status writeRead(uint8_t address, const uint8_t* tx, std::size_t txLength,
                                   uint8_t* rx, std::size_t rxLength, uint32_t timeoutMs,
                                   void* user) {
    auto& fake = *static_cast<FakeTusb32xDevice*>(user);
    return fake.invoke(Kind::READ, address, tx, txLength, rx, rxLength, timeoutMs);
  }

 private:
  Step& append() {
    if (stepCount >= 8) {
      ++errors;
      return script[7];
    }
    auto& step = script[stepCount++];
    step = {};
    return step;
  }

  TUSB32x::Status invoke(Kind kind, uint8_t address, const uint8_t* tx,
                         std::size_t txLength, uint8_t* rx, std::size_t rxLength,
                         uint32_t timeoutMs) {
    const auto index = calls++;
    if (index >= stepCount) return reject();
    const auto& step = script[index];
    if (kind != step.kind || address != step.address || tx == nullptr ||
        txLength != step.txLength || rxLength != step.rxLength || timeoutMs != step.timeoutMs ||
        txLength > 2 || rxLength > 11 || step.fillLength > rxLength ||
        (kind == Kind::READ && (rx == nullptr || rx == step.callerOutput)) ||
        (kind == Kind::WRITE && rx != nullptr)) return reject();
    if (std::memcmp(tx, step.tx, txLength) != 0) return reject();
    if (step.observedStats != nullptr &&
        (step.observedStats->totalAttempts != step.expectedAttempts ||
         step.observedStats->totalSuccess != step.expectedSuccess ||
         step.observedStats->totalFailures != step.expectedFailures)) return reject();
    if (kind == Kind::READ && step.fillLength != 0) {
      std::memcpy(rx, step.rx, step.fillLength);
    }
    if (kind == Kind::WRITE && step.applyWrite) registers[tx[0]] = tx[1];
    return step.status;
  }

  TUSB32x::Status reject() {
    ++errors;
    return TUSB32x::Status::Error(TUSB32x::Err::I2C_ERROR, "unexpected scripted callback", -90001);
  }
};

}  // namespace tusb32x_test
