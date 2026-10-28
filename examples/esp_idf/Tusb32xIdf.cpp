/// @file Tusb32xIdf.cpp
/// @brief Native IDF 5.5.5/6.1 synchronous STOP-separated transfers with one callback budget.
#include "Tusb32xIdf.h"

#include <esp_idf_version.h>
#include <esp_rom_sys.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>
#include <limits>

#if ESP_IDF_VERSION != ESP_IDF_VERSION_VAL(5, 5, 5) && \
    ESP_IDF_VERSION != ESP_IDF_VERSION_VAL(6, 1, 0)
#error "Re-audit native I2C waits and buffer lifetime before changing the supported IDF"
#endif
#if CONFIG_PM_ENABLE || CONFIG_I2C_ENABLE_DEBUG_LOG || CONFIG_LOG_MAXIMUM_LEVEL != 0 || \
    (!CONFIG_IDF_TARGET_ESP32S2 && !CONFIG_IDF_TARGET_ESP32S3)
#error "Native adapter requires S2/S3 with PM and SDK logging disabled"
#endif
static_assert(configTICK_RATE_HZ >= 100 && configTICK_RATE_HZ <= 1000,
              "Re-audit timeout quantization outside 100..1000Hz");

namespace tusb32x_idf {
namespace {
using TUSB32x::Err;
using TUSB32x::Status;
constexpr uint64_t TICK_US = (1000000ULL + configTICK_RATE_HZ - 1) / configTICK_RATE_HZ;
// IDF b774170ff46 (5.5.5) and fff9895c82d7 (6.1), i2c_master.c: bus lock,
// <=4 command waits, final event, NACK-busy wait; <=2 bus clears of 50ms plus
// a tick. Short frames fit FIFO32 on S2/S3. CPU work and RCC critical-section
// contention belong to qualified maxOverheadUs. See docs/idf-transport.md.
constexpr uint64_t WAIT_COUNT = 7;
constexpr uint64_t CLEANUP_US = 2 * (50000 + TICK_US);
constexpr uint64_t BUS_FREE_US = 2; // TI STOP-to-START minimum is 1.3us.

/// @brief Read the native boot clock in microseconds on a normal owner task.
uint64_t nowUs() { return static_cast<uint64_t>(esp_timer_get_time()); }

/// @brief Saturate observable SDK counters without claiming physical traffic counts.
void add(uint64_t& value, uint64_t amount) {
  const uint64_t maximum = std::numeric_limits<uint64_t>::max();
  value = amount > maximum - value ? maximum : value + amount;
}

/// @brief Release only the application gate; never delete, reset or recover SDK resources.
struct Session {
  BusContext& bus; ///< Shared controller whose gate was acquired before construction.
  /// @brief Release serialization before leaving this terminal callback.
  ~Session() { bus.gate.clear(std::memory_order_release); }
};

/// @brief Divide the remaining aggregate envelope among all remaining SDK phases.
uint16_t phaseTimeout(const Context& context, uint64_t started, uint64_t budgetUs,
                      uint64_t phases) {
  const uint64_t elapsed = nowUs() - started;
  const uint64_t reserve = static_cast<uint64_t>(context.bus->maxOverheadUs) + BUS_FREE_US;
  if (elapsed >= budgetUs || budgetUs - elapsed <= reserve) return 0;
  const uint64_t share = (budgetUs - elapsed - reserve) / phases;
  if (share <= CLEANUP_US + WAIT_COUNT * TICK_US) return 0;
  uint64_t milliseconds = ((share - CLEANUP_US) / WAIT_COUNT - TICK_US) / 1000;
  if (milliseconds > context.sdkTimeoutMs) milliseconds = context.sdkTimeoutMs;
  // IDF pdMS_TO_TICKS truncates; never pass zero/infinite or less than one tick.
  if (milliseconds * 1000 < TICK_US) return 0;
  return static_cast<uint16_t>(milliseconds);
}

/// @brief Preserve exact native detail; generic IDF failures do not reveal the NACK phase.
Status sdkStatus(esp_err_t error) {
  if (error == ESP_OK) return Status::Ok();
  return Status::Error(error == ESP_ERR_TIMEOUT ? Err::I2C_TIMEOUT : Err::I2C_ERROR,
                       "Native I2C transfer failed", static_cast<int32_t>(error));
}

/// @brief Serialize one terminal write or pointer/read and publish only a complete receive.
Status transfer(uint8_t address, const uint8_t* tx, size_t txLength, uint8_t* rx,
                size_t rxLength, uint32_t timeoutMs, void* user) {
  const uint64_t started = nowUs();
  if (!user || !tx || (rxLength == 0 ? txLength != 2 :
      (!rx || txLength != 1 || rxLength > 8)))
    return Status::Error(Err::I2C_ERROR, "Invalid native I2C callback arguments");
  auto& context = *static_cast<Context*>(user);
  if (!context.bus)
    return Status::Error(Err::I2C_ERROR, "Missing native I2C bus context");
  auto& bus = *context.bus;
  // Context/configuration fields are immutable while bound. Check the execution
  // core before taking the gate; the same-core ISR lifetime rule is mandatory.
  if (bus.ownerCore != xPortGetCoreID() || xPortInIsrContext())
    return Status::Error(Err::I2C_ERROR, "Unsupported native I2C execution context");
  if (bus.gate.test_and_set(std::memory_order_acquire))
    return Status::Error(Err::I2C_BUSY, "Application bus gate is busy");
  Session session{bus};
  const auto valid = validateContext(context);
  if (!valid.ok()) return valid;
  if (address != context.config.device_address)
    return Status::Error(Err::I2C_ERROR, "Address does not match native device handle");
  const uint64_t budgetUs = static_cast<uint64_t>(timeoutMs) * 1000;
  uint16_t timeout = phaseTimeout(context, started, budgetUs, rxLength ? 2 : 1);
  if (!timeout)
    return Status::Error(Err::I2C_TIMEOUT, "Budget cannot contain SDK waits and cleanup");

  add(bus.stats.transmitCalls, 1);
  auto status = sdkStatus(i2c_master_transmit(context.device, tx, txLength, timeout));
  if (!status.ok()) return status; // Actual backend failure stays primary, even if late.
  add(bus.stats.successfulTxBytes, txLength);
  if (rxLength) {
    esp_rom_delay_us(static_cast<uint32_t>(BUS_FREE_US));
    timeout = phaseTimeout(context, started, budgetUs, 1);
    if (!timeout) return Status::Error(Err::I2C_TIMEOUT, "No budget for native receive phase");
    uint8_t candidate[8]{};
    add(bus.stats.receiveCalls, 1);
    status = sdkStatus(i2c_master_receive(context.device, candidate, rxLength, timeout));
    if (!status.ok()) {
      // Both audited SDKs retain contains_read on some error paths. Interrupts are masked
      // before return, and the owner runs on the interrupt's core. No later SDK
      // transfer may awaken that retained read descriptor; retire ALL bus users.
      bus.retired = true;
      return status;
    }
    add(bus.stats.successfulRxBytes, rxLength);
    if (nowUs() - started > budgetUs)
      return Status::Error(Err::I2C_TIMEOUT, "Qualified application timing bound was exceeded");
    std::memcpy(rx, candidate, rxLength);
    return Status::Ok();
  }
  // Violation detection supplements the audited wait bound; it cannot create it.
  if (nowUs() - started > budgetUs)
    return Status::Error(Err::I2C_TIMEOUT, "Qualified application timing bound was exceeded");
  return Status::Ok();
}
} // namespace

TUSB32x::Status validateContext(const Context& context) {
  if (!context.bus || !context.device)
    return Status::Error(Err::I2C_ERROR, "Missing native I2C handles");
  const auto& bus = *context.bus;
  const auto address = context.config.device_address;
  if (!bus.handle || bus.config.clk_source != I2C_CLK_SRC_DEFAULT ||
      bus.config.trans_queue_depth != 0 || bus.eventCallbacksRegistered ||
      !bus.maxOverheadUs || !bus.acceptSdkCleanup || !context.sdkTimeoutMs ||
      bus.ownerCore != xPortGetCoreID() || xPortInIsrContext() ||
      context.config.dev_addr_length != I2C_ADDR_BIT_LEN_7 ||
      context.config.flags.disable_ack_check || context.config.scl_speed_hz < 100000 ||
      context.config.scl_speed_hz > 400000 ||
      (address != 0x47 && address != 0x67 && address != 0x60 && address != 0x61))
    return Status::Error(Err::I2C_ERROR, "Unqualified native I2C context");
  if (bus.retired) return Status::Error(Err::I2C_BUS, "Native controller retired after receive failure");
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
} // namespace tusb32x_idf
