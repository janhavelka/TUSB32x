/// @file ExampleOwner.h
/// @brief Fixed application startup and secondary-device scheduling, independent of console output.
#pragma once

#include <TUSB32x/TUSB32x.h>
#include <cstdint>

namespace tusb32x_example {

/// @brief Serialized application owner for read-only startup and optional secondary-device service.
/// @details Borrows the driver; it owns only tokens it admits. Startup initializes then refreshes,
/// taking each exact result. A zero refresh period hands the driver to the CLI after startup.
/// Nonzero periods enable secondary refreshes. Any failure stops automatic scheduling; no retry,
/// reset or recovery is hidden here. Output never participates in result ownership or progress.
class ExampleOwner {
 public:
  /// @brief Borrow a driver and copy finite scheduling intervals in milliseconds.
  /// @param[in,out] device Serialized driver, bound before start and alive through this owner.
  /// @param[in] operationTimeoutMs Positive immutable timeout for each admitted operation.
  /// @param[in] refreshPeriodMs Zero for startup only; otherwise delay after each completed job.
  ExampleOwner(TUSB32x::TUSB32x& device, uint32_t operationTimeoutMs,
               uint32_t refreshPeriodMs = 0);
  ExampleOwner(const ExampleOwner&) = delete;
  ExampleOwner& operator=(const ExampleOwner&) = delete;

  /// @brief Admit one read-only initialization; no I2C and no second startup attempt.
  /// @param[in] nowMs Fresh owner time in the driver's uint64 millisecond domain.
  /// @return IN_PROGRESS or the actual admission failure; invalid zero timeout is INVALID_CONFIG.
  TUSB32x::Status start(uint64_t nowMs);
  /// @brief Advance only this owner's token, then consume its exact terminal result.
  /// @param[in] nowMs Fresh timestamp sampled after the previous callback, never reused as completion.
  /// @param[in] maxTransfers Zero forbids callbacks; all nonzero values are capped at one.
  /// @note At most one poll and one later admission per call; no loops, sleep, output or heap use.
  void tick(uint64_t nowMs, uint8_t maxTransfers = 1);
  /// @brief Coalesce an explicit application IRQ-service request; never call from an ISR.
  /// @return True if accepted after successful secondary startup; false when inactive or startup-only.
  bool requestService();
  /// @brief Stop admissions, cancel/take our own job and end the binding without I2C.
  /// @return Delivery/end failure if any. External CLI results remain in the core, never consumed.
  /// @note The application releases its bus only after every bound driver has ended.
  TUSB32x::Status stop();
  /// @brief Inspect whether startup completed or failed; no callback.
  /// @return True when the primary may hand control to the CLI.
  bool startupDone() const { return _startupDone; }
  /// @brief Inspect stopped scheduling, including terminal/admission failure.
  /// @return True after failure or explicit stop; never a hardware shutdown claim.
  bool stopped() const { return _stopped; }
  /// @brief Inspect whether a consumed result is available for historical reporting.
  /// @return True after this owner successfully takes its own exact result.
  bool lastResultValid() const { return _lastResultValid; }
  /// @brief Borrow the latest consumed historical outcome; check lastResultValid first.
  /// @return Reference valid for this owner's lifetime, replaced by the next consumed result.
  const TUSB32x::OperationResult& lastResult() const { return _lastResult; }
  /// @brief Copy latest admission/delivery/outcome status, preserving backend detail.
  /// @return Latest status; a later admission can be IN_PROGRESS beside an earlier retained result.
  TUSB32x::Status lastStatus() const { return _lastStatus; }
  /// @brief Inspect saturating completed-outcome/failure report count; zero means no report yet.
  /// @return Report count, saturated at UINT64_MAX; not an electrical event count.
  uint64_t reportSequence() const { return _reportSequence; }

 private:
  enum class Phase : uint8_t { INITIALIZE, STARTUP_REFRESH, READY };
  TUSB32x::TUSB32x& _device;
  uint32_t _timeoutMs;
  uint32_t _periodMs;
  TUSB32x::OperationToken _token{};
  TUSB32x::OperationResult _lastResult{};
  TUSB32x::Status _lastStatus{};
  uint64_t _reportSequence = 0;
  uint64_t _completedAtMs = 0;
  Phase _phase = Phase::INITIALIZE;
  bool _started = false;
  bool _startupDone = false;
  bool _stopped = false;
  bool _lastResultValid = false;
  bool _serviceRequested = false;

  /// @brief Record a local admission/delivery failure without fabricating a device result.
  void fail(TUSB32x::Status status);
  /// @brief Consume only the stored exact token and retain its historical outcome for reporting.
  TUSB32x::Status collect();
  /// @brief Advance the saturating report counter without wrapping.
  void report();
};

} // namespace tusb32x_example
