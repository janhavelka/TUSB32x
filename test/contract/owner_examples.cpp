/**
 * @file owner_examples.cpp
 * @brief Compile-only owner usage of planned observation, apply and cancellation contracts.
 *
 * Compile with -fsyntax-only; never link or execute this fixture. PlannedApi.h
 * adds future declarations in a separate type inheriting the delivered facade.
 * These examples check C++ usage, not operation timing, effects or hardware;
 * runtime owner coverage lives in test_operations.cpp.
 */
#include "PlannedApi.h"

#include <cstddef>
#include <cstdint>

namespace OwnerExamples {

using Driver = TUSB32x::planned::Driver;
using TUSB32x::Err;
using TUSB32x::OperationResult;
using TUSB32x::OperationTiming;
using TUSB32x::OperationToken;
using TUSB32x::Status;

// A context with stable application-owned identity, not an emulated device.
struct FakeBus {};

// Neither callback performs hardware work or pretends to succeed. Arguments
// retain the real callback shape, but no buffers are retained or populated.
Status unavailableWrite(uint8_t, const uint8_t*, size_t, uint32_t, void*) {
  return Status::Error(Err::I2C_ERROR, "compile-only fixture has no transport");
}

Status unavailableWriteRead(uint8_t, const uint8_t*, size_t, uint8_t*, size_t,
                            uint32_t, void*) {
  return Status::Error(Err::I2C_ERROR, "compile-only fixture has no transport");
}

// The external fixture controls rawTickMs between scheduled owner turns. Both
// devices borrow this bus; the owner serializes every user of that bus. No bus,
// scheduler, clock or FieldCore object is created or owned by the driver.
struct FakeOwner {
  FakeBus bus{};
  uint32_t rawTickMs = 0;
  uint32_t previousTickMs = 0;
  uint64_t extendedMs = 0;
  bool clockStarted = false;
};

// Sample freshly on each call, including after another device's callback.
// Successive samples must be less than 2^31 ms apart with no clock reset.
// Unsigned subtraction handles a 32-bit wrap; casting the raw tick alone does
// not. Backward/ambiguous samples preserve the extension and output.
Status sampleClock(FakeOwner& owner, uint64_t& nowMs) {
  const uint32_t rawTickMs = owner.rawTickMs;
  if (!owner.clockStarted) {
    owner.previousTickMs = rawTickMs;
    owner.extendedMs = rawTickMs;
    owner.clockStarted = true;
  } else {
    const uint32_t elapsedMs = rawTickMs - owner.previousTickMs;
    constexpr uint32_t HALF_RANGE = uint32_t{1} << 31;
    if (elapsedMs >= HALF_RANGE) {
      return Status::Error(Err::INVALID_PARAM, "backward or ambiguous owner clock");
    }
    owner.extendedMs += elapsedMs;
    owner.previousTickMs = rawTickMs;
  }
  nowMs = owner.extendedMs;
  return Status::Ok();
}

// Keep the issuing instance with its token. Equal token numbers from different
// devices are unrelated; never dispatch using the token number alone.
struct OwnedOperation {
  Driver& device;
  OperationToken token{};
};

// One call is one scheduled owner turn: sample time, poll at most once, and
// exact-take a terminal result. IN_PROGRESS leaves out unchanged. Local poll
// rejection also leaves out unchanged. OK means delivery succeeded; callers
// inspect out.status separately, including transport failure or cancellation.
// Taking never retries or repairs writes and never starts replacement work.
Status advanceOnce(FakeOwner& owner, OwnedOperation& operation, OperationResult& out) {
  uint64_t nowMs = 0;
  const Status sampled = sampleClock(owner, nowMs);
  if (!sampled.ok()) {
    return sampled;
  }

  const Status advanced = operation.device.poll(operation.token, nowMs, 1);
  if (advanced.inProgress()) {
    return advanced;
  }
  const TUSB32x::OperationProgress progress = operation.device.progress();
  if (!progress.resultPending || progress.token.value != operation.token.value) {
    return advanced;
  }
  const Status delivered = operation.device.takeResult(operation.token, out);
  if (!delivered.ok()) {
    return delivered;
  }
  operation.token = {};
  return delivered;
}

// Passive observation: the caller explicitly admits a bounded refresh. It does
// not initialize, apply defaults, acknowledge an interrupt or authorize power.
// A valid binding suffices even when configuration trust is UNKNOWN. The owner
// later calls advanceOnce on separate scheduler turns until it delivers a result.
Status startPassiveObservation(OwnedOperation& operation, const OperationTiming& timing) {
  return operation.device.startRefreshPort(timing, operation.token);
}

// Read a REFRESH_PORT result only after successful delivery. This checks evidence
// presence, not whether its age meets application policy. Other fields still
// need their own validity/coherence checks. DETACHED is ordinary evidence.
bool hasAttachmentEvidence(const OperationResult& result) {
  const uint16_t attachmentField = static_cast<uint16_t>(TUSB32x::PortField::ATTACHMENT);
  return result.kind == TUSB32x::OperationKind::REFRESH_PORT && result.status.ok() &&
         !result.port.invalidated && result.port.observedTimeValid &&
         (result.port.validFields & attachmentField) != 0;
}

// Explicit apply: caller has already established current identity, known actual
// controls and board prerequisites. Admission copies this candidate into desired
// state; only later poll calls can write. A rejected start preserves the token.
// No apply is chained from initialization, observation or result delivery.
Status startExplicitApply(OwnedOperation& operation, const TUSB32x::DeviceProfile& candidate,
                          const OperationTiming& timing) {
  return operation.device.startApplyProfile(candidate, timing, operation.token);
}

// Cancel an admitted apply after any independently scheduled owner turn. A coarse
// WRITING phase or fixed poll count cannot establish whether a write occurred.
// With the unavailable fake backend an apply cannot be demonstrated successfully;
// this function only type-checks handling of the future engine's actual evidence.
// Cancel performs no I2C and cannot undo a write. A matching already terminal
// operation keeps its original outcome. Successful delivery returns OK even when
// out.status is CANCELLED or I2C_ERROR; out.hardwareEffect and out.configuration
// retain PARTIAL/INDETERMINATE and UNKNOWN evidence where applicable. No last poll
// timestamp is invented as cancellation completion time. The application decides
// whether to request a later read-only reconcile; there is no automatic replay.
Status cancelPossibleWrite(OwnedOperation& operation, OperationResult& out) {
  const Status cancelled = operation.device.cancel(operation.token);
  if (!cancelled.ok()) {
    return cancelled;
  }
  const Status delivered = operation.device.takeResult(operation.token, out);
  if (!delivered.ok()) {
    return delivered;
  }
  operation.token = {};
  return delivered;
}

// Two distinct strapped targets share one externally owned bus and callbacks.
// The owner and devices must outlive their bindings, and the owner calls end on
// each before destroying the bus. A second-bind failure leaves the first bound;
// this helper returns the error without hiding that partial local setup.
Status bindTwoDevices(FakeOwner& owner, Driver& first, Driver& second) {
  TUSB32x::DriverConfig firstConfig{};
  firstConfig.i2cWrite = unavailableWrite;
  firstConfig.i2cWriteRead = unavailableWriteRead;
  firstConfig.i2cUser = &owner.bus;
  firstConfig.variant = TUSB32x::Variant::TUSB320LAI;
  firstConfig.address = TUSB32x::DeviceAddress::ADDR_LOW_0X47;
  const Status firstBound = first.bind(firstConfig);
  if (!firstBound.ok()) {
    return firstBound;
  }

  TUSB32x::DriverConfig secondConfig = firstConfig;
  secondConfig.address = TUSB32x::DeviceAddress::ADDR_HIGH_0X67;
  return second.bind(secondConfig);
}

// Each operation was admitted separately on its own device. No loop or hidden
// retry: the application invokes another turn when its scheduler chooses. Each
// advanceOnce samples the clock itself, so the second device never receives a
// timestamp sampled before a possible first-device callback. A stalled fake
// clock is an equal-tick sample, not permission to finish a hardware wait.
// Examine each returned status; OK delivers its corresponding result, whose own
// status describes the operation. A failure on one device does not retry either.
void advanceTwoDevicesOnce(FakeOwner& owner, OwnedOperation& first, OwnedOperation& second,
                           OperationResult& firstResult, OperationResult& secondResult,
                           Status& firstStatus, Status& secondStatus) {
  firstStatus = advanceOnce(owner, first, firstResult);
  secondStatus = advanceOnce(owner, second, secondResult);
}

}  // namespace OwnerExamples
