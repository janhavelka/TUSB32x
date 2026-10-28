/// @file main.cpp
/// @brief Standalone package runtime contract using only public headers and simulated transport.
/// @details Checks remain active in Release. No device, framework, private header or test hook is used.
#include <TUSB32x/CommandTable.h>
#include <TUSB32x/Config.h>
#include <TUSB32x/Status.h>
#include <TUSB32x/TUSB32x.h>
#include <TUSB32x/Version.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace {

namespace t = TUSB32x;

/// @brief Fixed read-only LAI fixture with synthetic owner time; no electrical behavior is modeled.
struct SimulatedBus {
  uint32_t reads = 0; ///< Terminal read callback attempts.
  uint32_t writes = 0; ///< Terminal write callback attempts; initialization requires zero.
  uint64_t nowMs = 100; ///< Synthetic monotonic milliseconds, sampled after each callback.
};

/// @brief Report a consumer invariant in every build mode.
/// @param[in] condition Required condition.
/// @param[in] message Static failure explanation.
/// @return The condition; false propagates to the process exit status.
bool require(bool condition, const char* message) {
  if (!condition) std::fprintf(stderr, "FAIL: %s\n", message);
  return condition;
}

/// @brief Reject every data write; initialization is required to be read-only.
/// @param[in] address Unused selected seven-bit address.
/// @param[in] data Borrowed bytes, never retained or accessed by this rejecting callback.
/// @param[in] length Unused byte count.
/// @param[in] timeoutMs Unused aggregate millisecond budget.
/// @param[in] user Live SimulatedBus owned by main through end().
/// @return Terminal I2C_ERROR with a full signed backend detail.
t::Status simulatedWrite(uint8_t address, const uint8_t* data, std::size_t length,
                         uint32_t timeoutMs, void* user) {
  (void)address;
  (void)data;
  (void)length;
  (void)timeoutMs;
  auto& bus = *static_cast<SimulatedBus*>(user);
  ++bus.writes;
  ++bus.nowMs;
  return t::Status::Error(t::Err::I2C_ERROR, "unexpected simulated write", -80001);
}

/// @brief Supply exactly the six documented initialization reads from a local LAI image.
/// @param[in] address Required seven-bit address 0x47.
/// @param[in] tx Borrowed one-byte pointer; never retained.
/// @param[in] txLength Required pointer length one.
/// @param[out] rx Borrowed output, filled completely only for a valid frame.
/// @param[in] rxLength Eight for identity, one for every scalar read.
/// @param[in] timeoutMs Required positive aggregate budget, at most the configured 20 ms.
/// @param[in] user Live SimulatedBus borrowed until end(); callback never reenters the driver.
/// @return Terminal OK or I2C_ERROR for any unexpected frame; no retries or partial success.
t::Status simulatedRead(uint8_t address, const uint8_t* tx, std::size_t txLength,
                        uint8_t* rx, std::size_t rxLength, uint32_t timeoutMs, void* user) {
  auto& bus = *static_cast<SimulatedBus*>(user);
  constexpr uint8_t REGISTERS[] = {0x00, 0xA0, 0x08, 0x09, 0x0A, 0x45};
  constexpr uint8_t VALUES[] = {0x00, 0x02, 0x00, 0x04, 0x30, 0x00};
  const uint32_t index = bus.reads++;
  ++bus.nowMs;
  if (index >= sizeof(REGISTERS) || address != 0x47 || tx == nullptr || txLength != 1 ||
      tx[0] != REGISTERS[index] || rx == nullptr || rxLength != (index == 0 ? 8U : 1U) ||
      timeoutMs == 0 || timeoutMs > 20) {
    return t::Status::Error(t::Err::I2C_ERROR, "unexpected simulated read", -80002);
  }
  if (index == 0) {
    constexpr uint8_t IDENTITY[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
    for (std::size_t i = 0; i < sizeof(IDENTITY); ++i) rx[i] = IDENTITY[i];
  } else {
    rx[0] = VALUES[index];
  }
  return t::Status::Ok();
}

/// @brief Bind, initialize cooperatively and inspect take-once copied evidence.
/// @param[in,out] driver Serialized owner facade, bound until explicit end().
/// @param[in,out] bus Simulated callback context that outlives the borrowed binding.
/// @return True only when all public runtime invariants hold.
bool consume(t::TUSB32x& driver, SimulatedBus& bus) {
  t::DriverConfig config{};
  config.i2cWrite = simulatedWrite;
  config.i2cWriteRead = simulatedRead;
  config.i2cUser = &bus;
  config.variant = t::Variant::TUSB320LAI;
  config.address = t::DeviceAddress::ADDR_LOW_0X47;
  config.i2cTimeoutMs = 20;
  if (!require(driver.bind(config).ok() && bus.reads == 0 && bus.writes == 0,
               "binding is bus-silent")) return false;

  t::OperationToken token{};
  t::Status outcome = driver.startInitialize({bus.nowMs, bus.nowMs + 100}, token);
  if (!require(outcome.inProgress() && token.valid() && bus.reads == 0 && bus.writes == 0,
               "admission returns a nonzero token without callbacks")) return false;
  const t::ConfigurationSnapshot before = driver.configurationSnapshot();
  outcome = driver.poll(token, bus.nowMs, 0);
  if (!require(outcome.inProgress() && bus.reads == 0 && bus.writes == 0 &&
               driver.progress().lastPollTransfers == 0,
               "zero poll budget invokes no callbacks")) return false;

  // Each iteration represents a separate owner turn, with a fresh post-callback time sample.
  for (uint32_t turn = 0; turn < 20 && outcome.inProgress(); ++turn) {
    const uint32_t attempts = bus.reads + bus.writes;
    ++bus.nowMs;
    outcome = driver.poll(token, bus.nowMs, 1);
    if (!require(bus.reads + bus.writes - attempts <= 1,
                 "one owner turn invokes at most one terminal callback")) return false;
  }
  if (!require(outcome.ok() && bus.reads == 6 && bus.writes == 0,
               "finite read-only initialization completes six expected callbacks")) return false;

  t::OperationResult result{};
  const uint32_t attempts = bus.reads + bus.writes;
  t::OperationToken wrong{token.value + 1};
  if (!require(driver.takeResult(wrong, result).is(t::Err::TOKEN_MISMATCH) &&
               result.token.value == 0, "wrong token preserves the caller result")) return false;
  const t::Status delivery = driver.takeResult(token, result);
  if (!require(delivery.ok() && result.token.value == token.value &&
               result.kind == t::OperationKind::INITIALIZE &&
               result.state == t::OperationState::SUCCEEDED && result.status.ok() &&
               result.status.detail == 0 && result.hardwareEffect == t::HardwareEffect::NONE &&
               result.transfers == 6 && result.completedTimeValid &&
               result.identity.identityMatches && result.identity.revisionMatches &&
               result.configuration.state == t::ConfigurationState::KNOWN &&
               result.configuration.hasObservedProfile && result.configuration.hasVerifiedProfile &&
               result.configuration.rawConfig0A == 0x30 &&
               bus.reads + bus.writes == attempts,
               "exact-token take delivers successful read-only evidence without callbacks")) {
    return false;
  }
  const t::ConfigurationSnapshot copied = driver.configurationSnapshot();
  if (!require(driver.takeResult(token, result).is(t::Err::RESULT_NOT_AVAILABLE) &&
               result.token.value == token.value, "terminal result is consumed once")) return false;
  const t::Status ended = driver.end();
  return require(ended.ok() && !driver.diagnostics().bound && bus.reads + bus.writes == attempts &&
                 before.state == t::ConfigurationState::UNCONFIGURED &&
                 copied.state == t::ConfigurationState::KNOWN && copied.rawConfig0A == 0x30 &&
                 result.configuration.state == t::ConfigurationState::KNOWN,
                 "end is bus-silent and cannot mutate earlier value snapshots");
}

}  // namespace

/// @brief Run the simulated public package contract with Release-active checks.
/// @return Zero on success; one on any invariant failure.
int main() {
  SimulatedBus bus{};
  t::TUSB32x driver{};
  const bool passed = consume(driver, bus);
  const bool ended = require(driver.end().ok(), "explicit cleanup end succeeds");
  if (!passed || !ended) return 1;
  std::printf("Package consumer PASS: TUSB32x %s; SIMULATED read-only initialization\n", t::VERSION);
  return 0;
}
