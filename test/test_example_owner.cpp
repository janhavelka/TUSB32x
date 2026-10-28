/// @file test_example_owner.cpp
/// @brief Startup ownership, independent devices and bus-silent shutdown using public operations.
#include "ExampleOwner.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace {
namespace t = TUSB32x;
using Owner = tusb32x_example::ExampleOwner;
int failures = 0;
unsigned checks = 0;
void check(bool value, const char* expression, int line) {
  ++checks;
  if (!value) { ++failures; std::printf("FAIL owner example %d: %s\n", line, expression); }
}
#define CHECK(value) check((value), #value, __LINE__)

/// @brief Two distinct LAI fixtures sharing one serialized callback context; no electrical model.
struct Bus {
  unsigned calls = 0;
  unsigned writes = 0;
  unsigned failCall = 0;
  bool inCallback = false;
  uint8_t addresses[64]{};
  uint8_t registers[64]{};
  static t::Status read(uint8_t address, const uint8_t* tx, std::size_t txLength,
                        uint8_t* rx, std::size_t length, uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    CHECK(!bus.inCallback);
    bus.inCallback = true;
    CHECK(address == 0x47 || address == 0x67);
    CHECK(tx != nullptr && txLength == 1 && rx != nullptr);
    CHECK(timeout > 0 && timeout <= 20);
    CHECK(bus.calls < 64);
    if (bus.calls < 64) {
      bus.addresses[bus.calls] = address;
      bus.registers[bus.calls] = tx[0];
    }
    ++bus.calls;
    bus.inCallback = false;
    if (bus.calls == bus.failCall)
      return t::Status::Error(t::Err::I2C_BUS, "recorded bus failure", -7654321);
    const uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    if (tx[0] == 0) {
      CHECK(length == sizeof(identity));
      std::memcpy(rx, identity, sizeof(identity));
    } else {
      CHECK(length == 1);
      CHECK(tx[0] == 0xA0 || tx[0] == 8 || tx[0] == 9 || tx[0] == 10 || tx[0] == 0x45);
      rx[0] = tx[0] == 0xA0 ? 2 : tx[0] == 9 ? 0x94 : 0;
    }
    return t::Status::Ok();
  }
  static t::Status write(uint8_t address, const uint8_t* tx, std::size_t length,
                         uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    CHECK(!bus.inCallback);
    CHECK(address == 0x47 || address == 0x67);
    CHECK(length == 2 && tx[0] == 9 && tx[1] == 0x14);
    CHECK(timeout > 0 && timeout <= 20);
    ++bus.calls;
    ++bus.writes;
    return t::Status::Ok();
  }
};

t::DriverConfig config(Bus& bus, uint8_t address = 0x47) {
  t::DriverConfig value{};
  value.variant = t::Variant::TUSB320LAI;
  value.address = static_cast<t::DeviceAddress>(address);
  value.i2cWrite = Bus::write;
  value.i2cWriteRead = Bus::read;
  value.i2cUser = &bus;
  return value;
}

void completeStartup(Owner& owner, Bus& bus, uint64_t& now) {
  for (unsigned i = 0; i < 32 && !owner.startupDone(); ++i) {
    const auto before = bus.calls;
    owner.tick(++now, 255);
    CHECK(bus.calls - before <= 1);
  }
  CHECK(owner.startupDone());
}

void startupAndHandoff() {
  Bus bus;
  t::TUSB32x device;
  CHECK(device.bind(config(bus)).ok());
  Owner owner(device, 1000);
  uint64_t now = 100;
  CHECK(owner.start(now).inProgress());
  CHECK(!owner.requestService());
  owner.tick(++now, 0);
  CHECK(bus.calls == 0);
  completeStartup(owner, bus, now);
  const uint8_t expected[] = {0, 0xA0, 8, 9, 10, 0x45, 9, 8, 9};
  CHECK(bus.calls == sizeof(expected));
  CHECK(std::memcmp(bus.registers, expected, sizeof(expected)) == 0);
  CHECK(bus.writes == 0 && !owner.stopped());
  CHECK(owner.reportSequence() == 2 && owner.lastResultValid());
  CHECK(owner.lastResult().kind == t::OperationKind::REFRESH_PORT);
  CHECK(owner.lastResult().status.ok());
  CHECK(device.progress().state == t::OperationState::IDLE);
  t::OperationResult duplicate{};
  CHECK(device.takeResult(owner.lastResult().token, duplicate).is(t::Err::RESULT_NOT_AVAILABLE));
  CHECK(owner.start(++now).is(t::Err::INVALID_PARAM));
  t::OperationToken cliToken{};
  CHECK(device.startRefreshPort({now, now + 1000}, cliToken).inProgress());
  owner.tick(++now);
  CHECK(bus.calls == 9); // Startup-only owner does not poll CLI-owned work.
  CHECK(owner.stop().ok());
  CHECK(bus.calls == 9);
  CHECK(device.resultSnapshot(cliToken, duplicate).ok());
  CHECK(duplicate.status.is(t::Err::CANCELLED));
  CHECK(device.takeResult(cliToken, duplicate).ok());
  CHECK(!device.startInitialize({now, now + 1000}, cliToken).inProgress());
}

void failureAndShutdown() {
  for (unsigned failedRead = 1; failedRead <= 9; ++failedRead) {
    Bus bus;
    bus.failCall = failedRead;
    t::TUSB32x device;
    CHECK(device.bind(config(bus)).ok());
    Owner owner(device, 1000, 10);
    uint64_t now = 0;
    CHECK(owner.start(now).inProgress());
    completeStartup(owner, bus, now);
    CHECK(owner.stopped() && owner.lastResultValid());
    CHECK(owner.lastStatus().is(t::Err::I2C_BUS));
    CHECK(owner.lastStatus().detail == -7654321);
    CHECK(device.progress().state == t::OperationState::IDLE);
    CHECK(!owner.requestService());
    const auto before = bus.calls;
    owner.tick(now + 10000);
    CHECK(bus.calls == before);
    CHECK(owner.stop().ok());
    CHECK(bus.calls == before && bus.writes == 0);
  }
  Bus bus;
  t::TUSB32x device;
  CHECK(device.bind(config(bus)).ok());
  Owner owner(device, 1000);
  CHECK(owner.start(0).inProgress());
  owner.tick(1);
  CHECK(owner.stop().ok());
  CHECK(bus.calls == 1 && owner.lastResultValid());
  CHECK(owner.lastResult().status.is(t::Err::CANCELLED));
  CHECK(device.progress().state == t::OperationState::IDLE);
  CHECK(owner.stop().ok());
}

void twoDevicesAndExplicitService() {
  Bus bus;
  t::TUSB32x first;
  t::TUSB32x second;
  CHECK(first.bind(config(bus, 0x47)).ok());
  CHECK(second.bind(config(bus, 0x67)).ok());
  Owner primary(first, 1000);
  Owner secondary(second, 1000, 100);
  CHECK(primary.start(0).inProgress());
  CHECK(secondary.start(0).inProgress());
  uint64_t now = 0;
  for (; now < 32 && (!primary.startupDone() || !secondary.startupDone()); ++now) {
    auto before = bus.calls;
    primary.tick(now + 1);
    CHECK(bus.calls - before <= 1);
    before = bus.calls;
    secondary.tick(now + 1);
    CHECK(bus.calls - before <= 1);
  }
  CHECK(primary.startupDone() && secondary.startupDone());
  CHECK(bus.calls == 18 && bus.writes == 0);
  for (unsigned i = 0; i < 18; i += 2) {
    CHECK(bus.addresses[i] == 0x47 && bus.addresses[i + 1] == 0x67);
  }
  CHECK(secondary.requestService());
  CHECK(secondary.requestService()); // Coalescing does not create a hidden queue.
  const auto before = bus.calls;
  secondary.tick(++now, 0); // Admission is bus-silent, including explicit service.
  CHECK(bus.calls == before);
  CHECK(second.progress().kind == t::OperationKind::SERVICE_INTERRUPT);
  for (unsigned i = 0; i < 12 && secondary.reportSequence() == 2; ++i)
    secondary.tick(++now);
  CHECK(secondary.reportSequence() == 3);
  CHECK(bus.calls == before + 5 && bus.writes == 1);
  CHECK(secondary.lastResult().kind == t::OperationKind::SERVICE_INTERRUPT);
  const auto afterService = bus.calls;
  secondary.tick(now + 99);
  CHECK(bus.calls == afterService && second.progress().state == t::OperationState::IDLE);
  secondary.tick(now + 100);
  CHECK(second.progress().kind == t::OperationKind::REFRESH_PORT);
  CHECK(bus.calls == afterService);
  CHECK(primary.stop().ok());
  CHECK(secondary.stop().ok());
  CHECK(bus.calls == afterService);
}

void admissionFailuresAndRollover() {
  Bus bus;
  t::TUSB32x device;
  Owner unbound(device, 1000);
  CHECK(unbound.start(0).is(t::Err::NOT_BOUND));
  CHECK(unbound.startupDone() && unbound.stopped() && !unbound.lastResultValid());
  CHECK(bus.calls == 0);
  CHECK(device.bind(config(bus)).ok());
  Owner invalid(device, 0);
  CHECK(invalid.start(0).is(t::Err::INVALID_CONFIG));
  CHECK(bus.calls == 0);
  Owner rollover(device, 1000);
  uint64_t now = std::numeric_limits<uint64_t>::max() - 4;
  CHECK(rollover.start(now).inProgress());
  completeStartup(rollover, bus, now);
  CHECK(!rollover.stopped() && rollover.lastResult().status.ok());
  CHECK(rollover.stop().ok());
}

void deadlinesAndIndependentFailure() {
  Bus bus;
  t::TUSB32x device;
  CHECK(device.bind(config(bus)).ok());
  Owner expired(device, 10);
  CHECK(expired.start(100).inProgress());
  expired.tick(110, 0);
  CHECK(bus.calls == 0 && expired.startupDone() && expired.stopped());
  CHECK(expired.lastResultValid() && expired.lastResult().status.is(t::Err::TIMEOUT));
  CHECK(device.progress().state == t::OperationState::IDLE);
  CHECK(expired.stop().ok());

  t::TUSB32x first;
  t::TUSB32x second;
  CHECK(first.bind(config(bus)).ok());
  CHECK(second.bind(config(bus, 0x67)).ok());
  Owner failed(first, 1000, 100);
  Owner healthy(second, 1000, 100);
  bus.failCall = 1;
  CHECK(failed.start(0).inProgress());
  CHECK(healthy.start(0).inProgress());
  for (uint64_t now = 1; now < 32; ++now) {
    failed.tick(now);
    healthy.tick(now);
  }
  CHECK(failed.stopped() && failed.lastStatus().is(t::Err::I2C_BUS));
  CHECK(healthy.startupDone() && !healthy.stopped());
  CHECK(bus.calls == 10);
  CHECK(failed.reportSequence() == 1 && healthy.reportSequence() == 2);
  CHECK(healthy.lastResult().status.ok());
  CHECK(failed.stop().ok() && healthy.stop().ok());

  // Already-terminal work admitted by another owner is also left untouched on stop.
  CHECK(device.bind(config(bus)).ok());
  Owner observer(device, 1000);
  t::OperationToken other{};
  CHECK(device.startInitialize({100, 1000}, other).inProgress());
  CHECK(device.cancel(other).ok());
  CHECK(observer.stop().ok());
  t::OperationResult result{};
  CHECK(device.resultSnapshot(other, result).ok());
  CHECK(result.status.is(t::Err::CANCELLED));
  CHECK(device.takeResult(other, result).ok());
  CHECK(!observer.lastResultValid());
}
} // namespace

int main() {
  startupAndHandoff();
  failureAndShutdown();
  twoDevicesAndExplicitService();
  admissionFailuresAndRollover();
  deadlinesAndIndependentFailure();
  std::printf("example owner: %u checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
