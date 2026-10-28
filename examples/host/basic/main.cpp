/// @file main.cpp
/// @brief Runnable ordinary C++ owner with explicitly SIMULATED transport and time.
/// @details No physical device or electrical timing is represented. One serialized owner borrows
/// fixed context and callback buffers. Production owners sample fresh time after every callback.
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>
#include <TUSB32x/Version.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "profile.h"

namespace {

namespace t = TUSB32x;

/// @brief Fixed local image and synthetic milliseconds, alive until driver.end() returns.
/// @details The gate detects overlap in this single-threaded example; it is not an OS mutex.
/// A real shared bus needs bounded serialization across all users and both read phases.
struct SimulatedBus {
  uint8_t registers[0xA1]{}; ///< Local storage, not a general silicon emulator.
  uint64_t nowMs = 100; ///< Synthetic owner-clock domain; never wall-clock evidence.
  uint32_t calls = 0; ///< Terminal callback attempts, not physical bus transactions.
  uint32_t writes = 0; ///< Successful simulated data writes.
  bool gateHeld = false; ///< Held only inside callbacks, never between poll turns.

  /// @brief Supply a detached LAI image with normal DRP preference.
  SimulatedBus() {
    const uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
    for (std::size_t i = 0; i < sizeof(identity); ++i) registers[i] = identity[i];
    registers[0x09] = 0x04;
    registers[0x0A] = 0x30;
    registers[0xA0] = 0x02;
  }
};

/// @brief Print typed status and full backend detail.
/// @param label Static display label.
/// @param status Copied API or operation outcome with static-lifetime text.
void printStatus(const char* label, const t::Status& status) {
  std::printf("%s: %s detail=%ld msg=%s\n", label, t::errorName(status.code),
              static_cast<long>(status.detail), status.msg);
}

/// @brief Check a consumer invariant in every build, including Release.
/// @param condition Required invariant.
/// @param message Static explanation shown on failure.
/// @return The condition; callers propagate failure to a nonzero process exit.
bool require(bool condition, const char* message) {
  if (!condition) std::printf("FAIL: %s\n", message);
  return condition;
}

/// @brief Terminal SIMULATED callback for this example's persistent preference write.
/// @details Two local bytes and one synthetic millisecond including gate acquisition. Unsupported
/// frames fail; no retry, reset or recovery is simulated. No buffer is retained and no reentry occurs.
/// @param address Seven-bit selected target, required to be 0x47.
/// @param data Borrowed pointer/value bytes, valid only until return.
/// @param length Required to be two.
/// @param timeoutMs Positive aggregate synthetic millisecond budget.
/// @param user Live SimulatedBus borrowed until driver.end().
/// @return Terminal OK or I2C_* with static text and full example backend detail.
t::Status simulatedWrite(uint8_t address, const uint8_t* data, std::size_t length,
                         uint32_t timeoutMs, void* user) {
  if (user == nullptr) return t::Status::Error(t::Err::I2C_ERROR, "missing context", -70001);
  auto& bus = *static_cast<SimulatedBus*>(user);
  ++bus.calls;
  if (bus.gateHeld) return t::Status::Error(t::Err::I2C_BUSY, "simulated bus occupied", -70002);
  if (timeoutMs == 0) return t::Status::Error(t::Err::I2C_TIMEOUT, "zero budget", -70003);
  if (address != 0x47 || data == nullptr || length != 2 || data[0] != 0x0A ||
      (data[1] != 0x30 && data[1] != 0x32)) {
    return t::Status::Error(t::Err::I2C_ERROR, "unsupported simulated write", -70004);
  }
  bus.gateHeld = true;
  bus.registers[data[0]] = data[1];
  ++bus.writes;
  ++bus.nowMs;
  bus.gateHeld = false;
  return t::Status::Ok();
}

/// @brief Terminal SIMULATED pointer-write STOP/read, serialized for the entire logical access.
/// @details Eight local bytes maximum and one synthetic millisecond; no SDK, short receive or
/// asynchronous completion exists. A physical backend needs an independent aggregate bound audit.
/// @param address Seven-bit 0x47 target.
/// @param tx Borrowed one-byte register pointer, not retained after return.
/// @param txLength Required to be one.
/// @param rx Borrowed destination filled completely only on success, never retained.
/// @param rxLength Eight for identity, one for a supported scalar register.
/// @param timeoutMs Positive aggregate synthetic budget for gate and both phases.
/// @param user Live SimulatedBus; callback never reenters its driver.
/// @return Terminal status; unsupported frames fail without partial success.
t::Status simulatedWriteRead(uint8_t address, const uint8_t* tx, std::size_t txLength,
                             uint8_t* rx, std::size_t rxLength, uint32_t timeoutMs, void* user) {
  if (user == nullptr) return t::Status::Error(t::Err::I2C_ERROR, "missing context", -70001);
  auto& bus = *static_cast<SimulatedBus*>(user);
  ++bus.calls;
  if (bus.gateHeld) return t::Status::Error(t::Err::I2C_BUSY, "simulated bus occupied", -70002);
  if (timeoutMs == 0) return t::Status::Error(t::Err::I2C_TIMEOUT, "zero budget", -70003);
  if (address != 0x47 || tx == nullptr || txLength != 1 || rx == nullptr ||
      !((tx[0] == 0 && rxLength == 8) ||
        (rxLength == 1 && (tx[0] <= 0x0A || tx[0] == 0x45 || tx[0] == 0xA0)))) {
    return t::Status::Error(t::Err::I2C_ERROR, "unsupported simulated read", -70005);
  }
  bus.gateHeld = true;
  for (std::size_t i = 0; i < rxLength; ++i) rx[i] = bus.registers[tx[0] + i];
  ++bus.nowMs;
  bus.gateHeld = false;
  return t::Status::Ok();
}

/// @brief Label effect evidence independently of delivery and operation outcome.
/// @param effect Public effect classification.
/// @return Static label for every public effect value.
const char* effectName(t::HardwareEffect effect) {
  switch (effect) {
    case t::HardwareEffect::NONE: return "NONE";
    case t::HardwareEffect::CONFIRMED: return "CONFIRMED";
    case t::HardwareEffect::PARTIAL: return "PARTIAL";
    case t::HardwareEffect::INDETERMINATE: return "INDETERMINATE";
  }
  return "INVALID";
}

/// @brief Advance a synthetic owner turn and create an immutable 100 ms deadline.
/// @param bus Application-owned context; its clock domain persists across operations.
/// @return Valid finite operation timing, never renewed by poll.
t::OperationTiming timing(SimulatedBus& bus) {
  ++bus.nowMs;
  return {bus.nowMs, bus.nowMs + 100};
}

/// @brief Run at most 64 example turns, one callback per poll, then take the exact owned token.
/// @details Production owners return to unrelated work between turns. Here synthetic time advances
/// without sleeping; this finite demonstration establishes no hardware wait or bus timing.
/// @param driver Serialized owner instance with an accepted operation.
/// @param bus Live borrowed context; the gate must be released after each turn.
/// @param token Exact instance-local token returned at admission.
/// @param admission Start status; IN_PROGRESS means accepted work.
/// @param result Receives the terminal result, independently inspected by the caller.
/// @return True on valid admission, bounded progress and successful delivery, even if payload failed.
bool finishOperation(t::TUSB32x& driver, SimulatedBus& bus, t::OperationToken token,
                     const t::Status& admission, t::OperationResult& result) {
  printStatus("Admission", admission);
  if (!require(admission.inProgress() && token.valid(), "operation must be admitted")) return false;
  t::Status outcome = admission;
  for (uint32_t turn = 0; turn < 64 && outcome.inProgress(); ++turn) {
    const uint32_t before = bus.calls;
    ++bus.nowMs;  // Fresh synthetic sample after the preceding terminal callback.
    outcome = driver.poll(token, bus.nowMs, 1);
    if (!require(bus.calls - before <= 1 && !bus.gateHeld,
                 "poll callback ceiling and released gate")) return false;
  }
  if (!require(!outcome.inProgress(), "finite example turn limit")) return false;
  printStatus("Poll outcome", outcome);
  const uint32_t before = bus.calls;
  const t::Status delivery = driver.takeResult(token, result);
  printStatus("Result delivery", delivery);
  if (!require(delivery.ok() && result.token.value == token.value && bus.calls == before,
               "exact-token delivery is bus-silent")) return false;
  printStatus("Operation payload", result.status);
  std::printf("Effect=%s; callbacks=%lu\n", effectName(result.hardwareEffect),
              static_cast<unsigned long>(result.transfers));
  return require(result.status.code == outcome.code, "delivery preserves operation outcome");
}

/// @brief Exercise explicit application policy through public APIs only.
/// @param driver Bound here for the duration of the demonstration.
/// @param bus Context outlives this function and the caller's explicit end().
/// @return True only when initialization, apply, observation and recovery evidence agree.
bool demonstrate(t::TUSB32x& driver, SimulatedBus& bus) {
  t::DriverConfig config{};
  config.i2cWrite = simulatedWrite;
  config.i2cWriteRead = simulatedWriteRead;
  config.i2cUser = &bus;
  config.variant = t::Variant::TUSB320LAI;
  config.address = t::DeviceAddress::ADDR_LOW_0X47;
  const auto binding = driver.bind(config);
  printStatus("Bind", binding);
  if (!require(binding.ok() && bus.calls == 0, "bind has no callbacks")) return false;

  t::OperationToken token{};
  t::OperationResult result{};
  auto admission = driver.startInitialize(timing(bus), token);
  if (!require(bus.calls == 0, "initialize admission is bus-silent") ||
      !finishOperation(driver, bus, token, admission, result) ||
      !require(result.status.ok() && result.hardwareEffect == t::HardwareEffect::NONE &&
                   bus.writes == 0 && driver.configurationSnapshot().state ==
                       t::ConfigurationState::KNOWN,
               "read-only initialization proves actual configuration")) return false;

  // A separate application decision changes Try preference, not board power or USB policy.
  admission = driver.startApplyProfile(makeExampleProfile(), timing(bus), token);
  if (!finishOperation(driver, bus, token, admission, result) ||
      !require(result.status.ok() && result.hardwareEffect == t::HardwareEffect::CONFIRMED &&
                   bus.writes == 1, "explicit apply must verify its write")) return false;

  admission = driver.startRefreshPort(timing(bus), token);
  if (!finishOperation(driver, bus, token, admission, result) ||
      !require(result.status.ok() && result.hardwareEffect == t::HardwareEffect::NONE,
               "port refresh is read-only")) return false;
  const t::PortSnapshot cached = driver.portSnapshot();
  if (!require(!cached.invalidated && cached.observedTimeValid && cached.sequence == 1 &&
                   cached.attachment == t::Attachment::DETACHED,
               "complete detached observation remains distinct from failure")) return false;
  std::printf("Cached SIMULATED port: detached; sequence=%llu; acquired=%llu ms\n",
              static_cast<unsigned long long>(cached.sequence),
              static_cast<unsigned long long>(cached.observedUptimeMs));

  // Model a coordinated external reset event; no core reset or automatic replay occurs.
  bus.registers[0x0A] = 0x30;
  const uint32_t before = bus.calls;
  const auto invalidation = driver.invalidateDeviceState(
      t::Status::Error(t::Err::CONFIGURATION_UNKNOWN, "application observed external reset"));
  printStatus("External reset invalidation", invalidation);
  if (!require(invalidation.ok() && bus.calls == before, "invalidation is bus-silent")) return false;
  uint8_t identityByte = 0;
  const auto rawRead = driver.readRegisterSync(0, identityByte);
  printStatus("Unrelated raw identity read", rawRead);
  if (!require(rawRead.ok() && identityByte == 0x30 && bus.calls == before + 1 &&
                   driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN &&
                   driver.portSnapshot().invalidated &&
                   driver.portSnapshot().sequence == cached.sequence && !cached.invalidated,
               "raw success cannot repair trust/freshness or change an older caller copy")) {
    return false;
  }
  std::printf("After raw success: configuration=UNKNOWN; cached port invalidated\n");

  // Independently choose read-only reconciliation and keep the reset settings, without replay.
  admission = driver.startReconcile(timing(bus), token);
  if (!finishOperation(driver, bus, token, admission, result) ||
      !require(result.status.is(t::Err::CONFIGURATION_MISMATCH) &&
                   result.hardwareEffect == t::HardwareEffect::NONE && bus.writes == 1 &&
                   driver.configurationSnapshot().state == t::ConfigurationState::KNOWN &&
                   driver.portSnapshot().invalidated,
               "reconcile proves actual differing settings without replay or port refresh")) {
    return false;
  }
  admission = driver.startRefreshPort(timing(bus), token);
  if (!finishOperation(driver, bus, token, admission, result) ||
      !require(result.status.ok() && !driver.portSnapshot().invalidated &&
                   driver.portSnapshot().sequence == 2,
               "only a complete new refresh renews port observation")) return false;
  return true;
}

}  // namespace

/// @brief Run the finite SIMULATED owner and explicitly unbind before context destruction.
/// @details All driver access is serialized. A production ISR signals its owner only, without API
/// calls. For original TUSB320/I after VDD return, the application establishes stable power/I2C and
/// decides whether to startRecoverDeadBattery after identity. The LAI example needs no base-only
/// recovery; a consumer never copies the silicon register sequence.
/// @return Zero when every always-active check passed, one on any failure including shutdown.
int main() {
  std::printf("SIMULATED TUSB320LAI owner; synthetic milliseconds; package %s\n", t::VERSION);
  SimulatedBus bus;  // Construct first so it outlives the driver on every exit path.
  t::TUSB32x driver;
  const bool demonstrated = demonstrate(driver, bus);
  const uint32_t before = bus.calls;
  const auto ended = driver.end();  // No bus reset or rollback; context remains alive here.
  printStatus("End", ended);
  const bool shutdown = require(ended.ok() && bus.calls == before && !bus.gateHeld &&
                                   !driver.diagnostics().bound, "bus-silent safe teardown");
  if (demonstrated && shutdown) std::printf("SIMULATED owner checks passed\n");
  return demonstrated && shutdown ? 0 : 1;
}
