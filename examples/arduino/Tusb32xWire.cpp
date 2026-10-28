/// @file Tusb32xWire.cpp
/// @brief Pinned Wire calls with one serialized budget and conservative SDK wait accounting.
#include "Tusb32xWire.h"

#include <Wire.h>
#include <esp_arduino_version.h>
#include <esp_idf_version.h>
#include <esp_timer.h>
#include <esp_rom_sys.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>
#include <limits>

#if ESP_ARDUINO_VERSION != ESP_ARDUINO_VERSION_VAL(3, 3, 11) || \
    ESP_IDF_VERSION != ESP_IDF_VERSION_VAL(5, 5, 5)
#error "Re-audit Wire/HAL/IDF before changing the supported SDK"
#endif
#if !CONFIG_DISABLE_HAL_LOCKS || CORE_DEBUG_LEVEL != 0 || CONFIG_PM_ENABLE || \
    CONFIG_I2C_ENABLE_DEBUG_LOG || \
    (!CONFIG_IDF_TARGET_ESP32S2 && !CONFIG_IDF_TARGET_ESP32S3)
#error "Adapter requires S2/S3, disabled Arduino HAL locks, and disabled Arduino logging"
#endif

namespace tusb32x_arduino {
namespace {
using TUSB32x::Err;
using TUSB32x::Status;
constexpr uint64_t TICK_US = (1000000ULL + configTICK_RATE_HZ - 1) / configTICK_RATE_HZ;
// IDF b774170ff46 i2c_master.c: <=4 command waits + bus mutex + final event +
// NACK busy wait. Two possible 50ms clears; strict >tick comparisons add a tick.
constexpr uint64_t CLEANUP_US = 2 * (50000 + TICK_US);
constexpr uint64_t WAIT_COUNT = 7;
constexpr uint64_t BUS_FREE_US = 2; // TI requires >=1.3us STOP-to-START.

/// @brief Read the native boot clock; caller supplies normal task/tick operation.
uint64_t nowUs() { return static_cast<uint64_t>(esp_timer_get_time()); }

/// @brief Saturate observable backend counts without wrapping.
void add(uint64_t& total, uint64_t amount) {
  const auto maximum = std::numeric_limits<uint64_t>::max();
  total = amount > maximum - total ? maximum : total + amount;
}

/// @brief Map supported seven-bit target addresses to prepared-handle bits.
uint8_t addressBit(uint8_t address) {
  switch (address) {
    case 0x47: return 1;
    case 0x67: return 2;
    case 0x60: return 4;
    case 0x61: return 8;
    default: return 0;
  }
}

/// @brief No lock wait. Destruction restores timeout before allowing another bus user.
struct Session {
  Context& context; ///< Controller whose gate the caller already acquired.
  uint16_t savedTimeout; ///< Previous Wire setting restored on every terminal path.
  /// @brief Save the serialized setting without taking another lock.
  explicit Session(Context& value) : context(value), savedTimeout(value.wire->getTimeOut()) {}
  /// @brief Restore settings before returning serialization to the application.
  ~Session() {
    context.wire->setTimeOut(savedTimeout);
    context.gate.clear(std::memory_order_release);
  }
};

/// @brief Reserve the full application overhead on every calculation, conservatively.
// Splitting the remaining envelope ensures phase two never receives a new budget.
uint16_t phaseTimeout(const Context& context, uint64_t started, uint64_t budgetUs,
                      uint64_t phases) {
  const uint64_t elapsed = nowUs() - started;
  const uint64_t reserve = static_cast<uint64_t>(context.maxOverheadUs) + BUS_FREE_US;
  if (elapsed >= budgetUs || budgetUs - elapsed <= reserve) return 0;
  const uint64_t share = (budgetUs - elapsed - reserve) / phases;
  if (share <= CLEANUP_US + WAIT_COUNT * TICK_US) return 0;
  uint64_t milliseconds = ((share - CLEANUP_US) / WAIT_COUNT - TICK_US) / 1000;
  if (milliseconds > context.wireTimeoutMs) milliseconds = context.wireTimeoutMs;
  // pdMS_TO_TICKS truncates: never send an SDK wait shorter than one whole tick.
  if (milliseconds * 1000 < TICK_US) return 0;
  return static_cast<uint16_t>(milliseconds);
}

/// @brief Buffer one short write and return its terminal Wire status without retry.
Status send(Context& context, uint8_t address, const uint8_t* data, size_t length) {
  context.wire->beginTransmission(address);
  const size_t accepted = context.wire->write(data, length);
  add(context.stats.bufferedTxBytes, accepted);
  if (accepted != length) {
    // Wire.flush only clears local buffers. With HAL locks disabled it leaves no
    // lock behind and, unlike endTransmission, cannot send a partial write/probe.
    context.wire->flush();
    return Status::Error(Err::I2C_ERROR, "Wire short buffer acceptance",
                         static_cast<int32_t>(accepted));
  }
  add(context.stats.transmitCalls, 1);
  const uint8_t code = context.wire->endTransmission(true);
  if (code == 0) return Status::Ok();
  // Wire3.3.11 maps both ESP_FAIL and ESP_ERR_NOT_FOUND to 2: phase is unknown.
  return Status::Error(code == 5 ? Err::I2C_TIMEOUT : Err::I2C_ERROR,
                       "Wire transmission failed", code);
}

/// @brief Serialize and budget one validated callback; publish only complete RX.
Status transfer(uint8_t address, const uint8_t* tx, size_t txLength, uint8_t* rx,
                size_t rxLength, uint32_t timeoutMs, void* user) {
  const uint64_t started = nowUs();
  if (!user || !tx ||
      (rxLength == 0 ? txLength != 2 : (!rx || txLength != 1 || rxLength > 8)))
    return Status::Error(Err::I2C_ERROR, "Invalid Wire callback arguments");
  auto& context = *static_cast<Context*>(user);
  // Admission happens before any Wire work, including getTimeOut.
  if (context.gate.test_and_set(std::memory_order_acquire))
    return Status::Error(Err::I2C_BUSY, "Application bus gate is busy");
  const auto valid = validateContext(context);
  if (!valid.ok() || !(context.preparedAddresses & addressBit(address))) {
    context.gate.clear(std::memory_order_release);
    return valid.ok() ? Status::Error(Err::I2C_ERROR, "Address was not prepared") : valid;
  }
  const uint64_t budgetUs = static_cast<uint64_t>(timeoutMs) * 1000;
  uint16_t timeout = phaseTimeout(context, started, budgetUs, rxLength ? 2 : 1);
  if (!timeout) {
    context.gate.clear(std::memory_order_release);
    return Status::Error(Err::I2C_TIMEOUT, "Budget cannot contain SDK waits and cleanup");
  }
  Session session(context);
  context.wire->setTimeOut(timeout);
  auto status = send(context, address, tx, txLength);
  if (!status.ok()) return status; // Preserve original backend failure, even if late.
  if (rxLength) {
    esp_rom_delay_us(static_cast<uint32_t>(BUS_FREE_US));
    timeout = phaseTimeout(context, started, budgetUs, 1);
    if (!timeout) return Status::Error(Err::I2C_TIMEOUT, "No budget for receive phase");
    context.wire->setTimeOut(timeout);
    add(context.stats.receiveCalls, 1);
    const size_t received = context.wire->requestFrom(address, rxLength, true);
    if (received != rxLength) {
      // Exact SDK keeps contains_read after some failures. Retire the controller
      // for all users; do not retry or repair it here. See integration guide.
      context.retired = true;
      return Status::Error(Err::I2C_ERROR, "Wire receive length mismatch; controller retired",
                           static_cast<int32_t>(received));
    }
    uint8_t candidate[8]{};
    for (size_t index = 0; index < rxLength; ++index) {
      const int value = context.wire->read();
      if (value < 0 || value > 255) {
        context.retired = true;
        return Status::Error(Err::I2C_ERROR, "Wire read failed; controller retired", value);
      }
      candidate[index] = static_cast<uint8_t>(value);
      add(context.stats.returnedRxBytes, 1);
    }
    if (nowUs() - started > budgetUs)
      return Status::Error(Err::I2C_TIMEOUT, "Qualified application timing bound was exceeded");
    std::memcpy(rx, candidate, rxLength);
    return Status::Ok(); // No fallible check after publishing caller output.
  }
  // Supplemental violation detection, never the proof that an earlier wait was bounded.
  if (nowUs() - started > budgetUs)
    return Status::Error(Err::I2C_TIMEOUT, "Qualified application timing bound was exceeded");
  return Status::Ok();
}
} // namespace

TUSB32x::Status validateContext(const Context& context) {
  if (!context.wire || !context.wireTimeoutMs || !context.maxOverheadUs ||
      context.busHz < 100000 || context.busHz > 400000 ||
      !context.acceptSdkCleanup || !context.preparedAddresses ||
      (context.preparedAddresses & 0xF0) || context.ownerCore != xPortGetCoreID() ||
      xPortInIsrContext())
    return Status::Error(Err::I2C_ERROR, "Unqualified Wire context or execution core");
  if (context.retired) return Status::Error(Err::I2C_BUS, "Wire controller retired after receive failure");
  return Status::Ok();
}

TUSB32x::Status write(uint8_t address, const uint8_t* data, size_t length,
                     uint32_t timeoutMs, void* user) {
  return transfer(address, data, length, nullptr, 0, timeoutMs, user);
}

TUSB32x::Status writeRead(uint8_t address, const uint8_t* txData, size_t txLength,
                         uint8_t* rxData, size_t rxLength, uint32_t timeoutMs, void* user) {
  if (!rxLength) return Status::Error(Err::I2C_ERROR, "Zero receive length");
  return transfer(address, txData, txLength, rxData, rxLength, timeoutMs, user);
}

uint64_t ownerNowMs() { return nowUs() / 1000; }
} // namespace tusb32x_arduino
