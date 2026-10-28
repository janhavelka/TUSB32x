/// @file Tusb32xWire.h
/// @brief Example-only terminal Wire adapter for the audited ESP32 S2/S3 SDK.
#pragma once

#include <atomic>
#include <TUSB32x/Config.h>

class TwoWire;

namespace tusb32x_arduino {

/// @brief Observable Wire calls and bytes; never inferred physical bus transactions.
struct WireStats {
  uint64_t transmitCalls = 0; ///< endTransmission(true) invocations, saturating.
  uint64_t receiveCalls = 0; ///< requestFrom(...,true) invocations, saturating.
  uint64_t bufferedTxBytes = 0; ///< Bytes accepted by Wire.write, not bytes proved on SDA.
  uint64_t returnedRxBytes = 0; ///< Bytes returned by Wire.read, not partial hardware counts.
};

/// @brief One application-owned context per physical controller, shared by every bus user.
/// @details Borrowed until all bound drivers end. Not copyable because the gate is unique.
/// Before enabling callbacks, the application initializes Wire on ownerCore, warms each
/// selected address's HAL handle, disables SDK logging, and qualifies maxOverheadUs.
/// Every bus user must acquire gate, check retired, and leave no pending transaction.
/// Use only synchronous master mode on a task pinned to ownerCore, with the audited
/// Arduino 3.3.11/IDF 5.5.5 S2/S3 build and globally disabled HAL locks. Each callback
/// completes terminally, borrows buffers only through return and never re-enters the driver.
/// Serialize each driver separately; the bus gate does not serialize its cached APIs.
/// No ISR calls. See docs/arduino-transport.md for SDK waits/cleanup and qualification.
struct Context {
  TwoWire* wire = nullptr; ///< Borrowed initialized master; adapter never begins/ends/deletes it.
  std::atomic_flag gate = ATOMIC_FLAG_INIT; ///< Nonblocking shared serialization; never held across polls.
  int ownerCore = -1; ///< Core that called Wire.begin; callbacks reject another core or ISR.
  uint8_t preparedAddresses = 0; ///< Prewarmed handles: bits 0/1/2/3 mean 47/67/60/61 hex.
  uint16_t wireTimeoutMs = 10; ///< Positive upper cap for each SDK wait; not the callback budget.
  uint32_t busHz = 0; ///< Exact application-configured SCL rate, supported range 100000..400000Hz.
  uint32_t maxOverheadUs = 0; ///< Qualified whole-callback CPU/preemption overhead; zero rejects setup.
  bool acceptSdkCleanup = false; ///< Explicit acceptance of SDK reset/bus-clear on failure.
  bool retired = false; ///< Failed receive/teardown retires ALL users until successful application recreation.
  WireStats stats{}; ///< Passive counts, read only under the application serialization boundary.
};

/// @brief Check setup locally; does not access Wire, allocate or acquire its internal locks.
/// @param[in] context Application prerequisites and controller association.
/// @return OK, I2C_ERROR for invalid/unsupported setup, or I2C_BUS for a retired controller.
/// @note Call on the owner task before bind. Cannot verify wiring, prewarming or a measured CPU bound.
TUSB32x::Status validateContext(const Context& context);

/// @brief Write exactly pointer and data under one terminal aggregate timeout.
/// @param[in] address Prepared seven-bit 47/67/60/61 address.
/// @param[in] data Two borrowed bytes; valid until return.
/// @param[in] length Must equal two.
/// @param[in] timeoutMs Whole callback milliseconds, including gate/SDK cleanup/CPU overhead.
/// @param[in,out] user Borrowed Context; no reentry. Gate contention fails immediately.
/// @return OK or I2C_*; Wire code preserved as detail, without inferred NACK phase.
/// Zero/insufficient budget returns I2C_TIMEOUT; invalid arguments/setup return I2C_ERROR.
/// @note No retry, recovery or bus ownership. A failed transmission may already have applied.
TUSB32x::Status write(uint8_t address, const uint8_t* data, size_t length,
                     uint32_t timeoutMs, void* user);

/// @brief Explicit pointer STOP, two-microsecond bus-free guard, then read STOP.
/// @param[in] address Prepared seven-bit target.
/// @param[in] txData One borrowed register pointer byte.
/// @param[in] txLength Must equal one.
/// @param[out] rxData Borrowed output, unchanged unless all bytes arrive successfully.
/// @param[in] rxLength Exact length in 1..8; no caller buffer retained by SDK.
/// @param[in] timeoutMs Aggregate milliseconds including gate, both phases, SDK cleanup and
/// qualified CPU overhead; second phase uses only the remaining time.
/// @param[in,out] user Borrowed Context shared by the entire physical controller.
/// @return OK or I2C_*; receive failure reports observed length, not an unavailable ESP error.
/// Zero/insufficient budget returns I2C_TIMEOUT; invalid arguments/setup return I2C_ERROR.
/// @note Failed receive retires the controller. Temporary Wire timeout is always restored.
TUSB32x::Status writeRead(uint8_t address, const uint8_t* txData, size_t txLength,
                         uint8_t* rxData, size_t rxLength, uint32_t timeoutMs, void* user);

/// @brief Native esp_timer microseconds floored to monotonic uint64 milliseconds.
/// @return Time since boot; no 32-bit millis rollover. Resolution is one millisecond.
/// @note Sample afresh after each terminal callback; no wait-completion timestamp is inferred.
uint64_t ownerNowMs();

} // namespace tusb32x_arduino
