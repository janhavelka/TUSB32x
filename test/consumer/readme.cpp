/// @file readme.cpp
/// @brief Execute the extracted README quick start with simulated LAI callbacks and owner time.
#include "readme_quickstart.inc"

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace {

namespace t = TUSB32x;

// This fixed fixture represents only the read-only README operation, not physical silicon.
struct SimulatedBus {
  std::uint64_t nowMs = 100;
  std::uint64_t sampledMs = 0;
  unsigned reads = 0;
  unsigned writes = 0;
  unsigned services = 0;
  unsigned clockSamples = 0;
  unsigned previousCallbackSamples = 0;
  unsigned callbacksThisTurn = 0;
  unsigned failAtRead = 0;
  unsigned expireAfterReads = 0;
  bool expireBeforeRead = false;
  bool badIdentity = false;
  bool contractViolation = false;
};

// The README accepts context-free owner hooks. This fixture-only pointer borrows one case's
// stack object for the duration of observeOnce; it is never used by production code.
SimulatedBus* activeBus = nullptr;

std::uint64_t nowMs() {
  auto& bus = *activeBus;
  ++bus.clockSamples;
  bus.sampledMs = bus.nowMs;
  return bus.nowMs;
}

void serviceOthers() {
  auto& bus = *activeBus;
  if (bus.callbacksThisTurn > 1) bus.contractViolation = true;
  bus.callbacksThisTurn = 0;
  ++bus.services;
  const bool expire = bus.expireBeforeRead ||
      (bus.expireAfterReads != 0 && bus.reads >= bus.expireAfterReads);
  bus.nowMs += expire ? 1000U : 1U;
}

// Check one callback per service turn and an owner clock sample taken after preceding work.
void callbackStarted(SimulatedBus& bus) {
  ++bus.callbacksThisTurn;
  if (bus.callbacksThisTurn > 1 || bus.services == 0 ||
      bus.clockSamples <= bus.previousCallbackSamples || bus.sampledMs != bus.nowMs) {
    bus.contractViolation = true;
  }
  bus.previousCallbackSamples = bus.clockSamples;
  ++bus.nowMs;  // Synthetic terminal completion; the next turn must sample again.
}

t::Status write(std::uint8_t, const std::uint8_t*, std::size_t,
                std::uint32_t, void* user) {
  auto& bus = *static_cast<SimulatedBus*>(user);
  callbackStarted(bus);
  ++bus.writes;
  return t::Status::Error(t::Err::I2C_ERROR, "README initialization must not write", -70000);
}

t::Status writeRead(std::uint8_t address, const std::uint8_t* tx, std::size_t txLength,
                    std::uint8_t* rx, std::size_t rxLength, std::uint32_t timeoutMs,
                    void* user) {
  auto& bus = *static_cast<SimulatedBus*>(user);
  callbackStarted(bus);
  ++bus.reads;
  if (bus.failAtRead == bus.reads) {
    return t::Status::Error(t::Err::I2C_TIMEOUT, "simulated terminal callback error", -70001);
  }
  if (address != 0x47 || tx == nullptr || txLength != 1 || rx == nullptr || timeoutMs == 0) {
    bus.contractViolation = true;
    return t::Status::Error(t::Err::I2C_ERROR, "unsupported simulated README frame", -70002);
  }
  if (tx[0] == 0 && rxLength == 8) {
    constexpr std::uint8_t IDENTITY[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0x00};
    for (std::size_t index = 0; index < sizeof(IDENTITY); ++index) rx[index] = IDENTITY[index];
    if (bus.badIdentity) rx[0] = 0xFF;
    return t::Status::Ok();
  }
  if (rxLength == 1) {
    switch (tx[0]) {
      case 0x08: rx[0] = 0; return t::Status::Ok();
      case 0x09: rx[0] = 0x04; return t::Status::Ok();
      case 0x0A: rx[0] = 0x30; return t::Status::Ok();
      case 0x45: rx[0] = 0; return t::Status::Ok();
      case 0xA0: rx[0] = 0x02; return t::Status::Ok();
      default: break;
    }
  }
  bus.contractViolation = true;
  return t::Status::Error(t::Err::I2C_ERROR, "unsupported simulated README read", -70003);
}

t::DriverConfig configuration(SimulatedBus& bus) {
  t::DriverConfig config;
  config.variant = t::Variant::TUSB320LAI;
  config.address = t::DeviceAddress::ADDR_LOW_0X47;
  config.i2cWrite = write;
  config.i2cWriteRead = writeRead;
  config.i2cUser = &bus;
  return config;
}

bool run(SimulatedBus& bus, const t::DriverConfig& config,
         std::uint64_t (*clock)() = nowMs, void (*service)() = serviceOthers) {
  activeBus = &bus;
  const bool result = observeOnce(config, clock, service);
  activeBus = nullptr;
  return result;
}

void check(bool condition, const char* name, int& failures) {
  if (!condition) {
    std::printf("FAIL README quick start: %s\n", name);
    ++failures;
  }
}

}  // namespace

/// @brief Return always-active failures for the exact README code under simulated success/faults.
int checkReadmeQuickstart() {
  int failures = 0;
  {
    SimulatedBus bus;
    check(run(bus, configuration(bus)), "read-only initialization succeeds", failures);
    check(bus.reads == 6 && bus.writes == 0, "six reads and zero writes", failures);
    check(!bus.contractViolation && bus.services >= bus.reads &&
              bus.clockSamples == bus.services + 1,
          "one callback per turn with freshly sampled owner time", failures);
  }
  {
    SimulatedBus bus;
    check(!run(bus, t::DriverConfig{}), "missing binding rejects", failures);
    auto config = configuration(bus);
    config.i2cWriteRead = nullptr;
    check(!run(bus, config), "missing transport callback rejects", failures);
    check(!run(bus, configuration(bus), nullptr), "missing clock rejects", failures);
    check(!run(bus, configuration(bus), nowMs, nullptr), "missing service hook rejects", failures);
    check(bus.reads == 0 && bus.writes == 0 && bus.services == 0 && bus.clockSamples == 0,
          "invalid binding and null hooks are bus-silent", failures);
  }
  {
    SimulatedBus bus;
    bus.failAtRead = 3;
    check(!run(bus, configuration(bus)), "terminal callback failure stays failed", failures);
    check(bus.reads == 3 && bus.writes == 0 && !bus.contractViolation,
          "failed callback is not retried", failures);
  }
  {
    SimulatedBus bus;
    bus.badIdentity = true;
    check(!run(bus, configuration(bus)), "successful transport with wrong identity fails", failures);
    check(bus.reads == 1 && bus.writes == 0 && !bus.contractViolation,
          "identity mismatch stops without writing", failures);
  }
  {
    SimulatedBus bus;
    bus.expireBeforeRead = true;
    check(!run(bus, configuration(bus)), "expired deadline before callback fails", failures);
    check(bus.reads == 0 && bus.writes == 0 && bus.services == 1,
          "deadline expiry does not perform transport", failures);
  }
  {
    SimulatedBus bus;
    bus.expireAfterReads = 6;
    check(!run(bus, configuration(bus)), "delivery cannot turn a late operation into success", failures);
    check(bus.reads == 6 && bus.writes == 0 && !bus.contractViolation,
          "successful callbacks preserve the immutable completion deadline", failures);
  }
  return failures;
}
