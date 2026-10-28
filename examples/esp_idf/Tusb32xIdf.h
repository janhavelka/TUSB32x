/// @file Tusb32xIdf.h
/// @brief Example-only synchronous native ESP-IDF master adapter; no Arduino dependency.
#pragma once

#include <atomic>
#include <driver/i2c_master.h>
#include <TUSB32x/Config.h>

namespace tusb32x_idf {

/// @brief Observable SDK calls and successful payload bytes, never physical transaction counts.
struct BackendStats {
  uint64_t transmitCalls = 0; ///< Saturating i2c_master_transmit invocation count.
  uint64_t receiveCalls = 0; ///< Saturating i2c_master_receive invocation count.
  uint64_t successfulTxBytes = 0; ///< Requested bytes of successful SDK transmissions only.
  uint64_t successfulRxBytes = 0; ///< Requested bytes of successful SDK receives only.
};

/// @brief One application-owned serialization boundary for every user of a physical bus.
/// @details Create the SDK bus using config on ownerCore before binding. These copied
/// creation settings are application evidence: opaque SDK handles cannot be inspected.
/// All users must share gate and honor retired, including users outside this adapter.
/// Pin the owner task to ownerCore; a runtime core check cannot prevent later migration.
/// No event callbacks may be registered. Keep this object alive until every binding
/// ends and the application deletes its devices and bus. No reentry or ISR use.
struct BusContext {
  i2c_master_bus_handle_t handle = nullptr; ///< Borrowed bus; never deleted by callbacks.
  i2c_master_bus_config_t config{}; ///< Exact immutable creation settings; default clock and zero queue depth required.
  std::atomic_flag gate = ATOMIC_FLAG_INIT; ///< Nonblocking bus gate, released before callback return.
  int ownerCore = -1; ///< Core that created the bus/interrupt; all transfers must run here.
  uint32_t maxOverheadUs = 0; ///< Qualified whole-callback CPU/preemption overhead in microseconds; zero rejects.
  bool acceptSdkCleanup = false; ///< Explicit acceptance of SDK-internal reset and bus clear on error.
  bool eventCallbacksRegistered = false; ///< Must remain false; asynchronous completion is unsupported.
  bool retired = false; ///< Receive failure retires every bus user until application teardown.
  BackendStats stats{}; ///< Read under application serialization; independent of core callback attempts.
};

/// @brief Borrowed target association; application creates the device with these exact settings.
/// @details End all affected drivers before changing/removing handles or settings.
/// The adapter cannot verify that an opaque device belongs to this bus/configuration.
struct Context {
  BusContext* bus = nullptr; ///< Borrowed common bus context; must outlive this target and its callbacks.
  i2c_master_dev_handle_t device = nullptr; ///< Borrowed synchronous SDK device; never created/deleted here.
  i2c_device_config_t config{}; ///< Exact creation settings, seven-bit address and SCL in 100000..400000Hz.
  uint16_t sdkTimeoutMs = 10; ///< Positive cap per SDK wait, not the aggregate callback timeout.
};

/// @brief Validate application-provided setup locally without SDK calls or locking.
/// @param[in] context Exact handle/configuration association supplied by the application.
/// @return OK, I2C_ERROR for invalid/unsupported setup, I2C_BUS for a retired bus.
/// @note Cannot certify handle metadata, wiring, absence of outside users or qualified CPU overhead.
/// Call before bind on the serialized owner; concurrent callbacks/resource changes are not allowed.
TUSB32x::Status validateContext(const Context& context);

/// @brief Send pointer and data once with a finite aggregate callback budget.
/// @param[in] address Seven-bit address matching the borrowed device configuration.
/// @param[in] data Two borrowed bytes, valid until return; no active SDK work accesses them later.
/// @param[in] length Exactly two bytes.
/// @param[in] timeoutMs Whole callback milliseconds including lock, waits, cleanup and overhead.
/// @param[in,out] user Borrowed Context; shared gate contention returns immediately.
/// @return OK or I2C_* with full esp_err_t detail; failures may follow a hardware effect.
TUSB32x::Status write(uint8_t address, const uint8_t* data, size_t length,
                     uint32_t timeoutMs, void* user);

/// @brief Pointer STOP, 2us bus-free guard, receive STOP under one gate and deadline.
/// @param[in] address Seven-bit address matching the borrowed device configuration.
/// @param[in] txData Borrowed register pointer; valid until return.
/// @param[in] txLength Exactly one byte.
/// @param[out] rxData Borrowed output, unchanged on error; no active SDK work accesses it after return.
/// @param[in] rxLength Exact receive size in 1..8 bytes.
/// @param[in] timeoutMs Aggregate milliseconds; receive receives only remaining budget.
/// @param[in,out] user Borrowed Context. Failed SDK receive retires the shared bus.
/// @return OK or I2C_*; no retry, inferred NACK phase, partial count or automatic recovery.
TUSB32x::Status writeRead(uint8_t address, const uint8_t* txData, size_t txLength,
                         uint8_t* rxData, size_t rxLength, uint32_t timeoutMs, void* user);

/// @brief Floor native esp_timer microseconds to monotonic 64-bit milliseconds.
/// @return Time since boot with 1ms resolution; no wrapping 32-bit intermediate.
/// @note Owner must sample afresh after callbacks; preserve the core's separate 1ms wait margin.
uint64_t ownerNowMs();

} // namespace tusb32x_idf
