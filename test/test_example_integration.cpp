/// @file test_example_integration.cpp
/// @brief Real CLI/application-owner handoff under output backpressure, without framework stubs.
#include "ExampleOwner.h"
#include "Tusb32xCli.h"
#include "support/FakeTusb32xDevice.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
namespace t = TUSB32x;
using Fake = tusb32x_test::FakeTusb32xDevice;
unsigned checks = 0;
unsigned failures = 0;
void check(bool value, const char* expression, int line) {
  ++checks;
  if (!value) { ++failures; std::printf("FAIL example integration %d: %s\n", line, expression); }
}
#define CHECK(value) check((value), #value, __LINE__)

struct Fixture {
  Fake bus{};
  t::TUSB32x device{};
  uint64_t now = 100;
  bool stalled = true;
  unsigned outputCalls = 0;
  std::size_t outputBytes = 0;
  t::DriverConfig config{};
  tusb32x_cli::Platform platform{};

  static uint64_t clock(void* user) { return static_cast<Fixture*>(user)->now; }
  static std::size_t output(const char* data, std::size_t length, void* user) {
    auto& fixture = *static_cast<Fixture*>(user);
    CHECK(data != nullptr && length <= tusb32x_cli::ConsoleBuffer::DRAIN_BUDGET);
    ++fixture.outputCalls;
    if (fixture.stalled) return 0;
    fixture.outputBytes += length;
    return length;
  }
  Fixture() {
    config.variant = t::Variant::TUSB320LAI;
    config.address = static_cast<t::DeviceAddress>(0x47);
    config.i2cWrite = Fake::write;
    config.i2cWriteRead = Fake::writeRead;
    config.i2cUser = &bus;
    platform.nowMs = clock;
    platform.write = output;
    platform.user = this;
    CHECK(device.bind(config).ok());
  }
  void identity() {
    bus = Fake{};
    const uint8_t expected[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    std::memcpy(bus.expectRead(0x47, 0, 8).rx, expected, sizeof(expected));
    bus.expectRead(0x47, 0xA0, 1).rx[0] = 2;
    bus.expectRead(0x47, 8, 1).rx[0] = 0;
    bus.expectRead(0x47, 9, 1).rx[0] = 0x94;
    bus.expectRead(0x47, 10, 1).rx[0] = 0;
    bus.expectRead(0x47, 0x45, 1).rx[0] = 0;
  }
  void port() {
    bus = Fake{};
    bus.expectRead(0x47, 9, 1).rx[0] = 0x94;
    bus.expectRead(0x47, 8, 1).rx[0] = 0;
    bus.expectRead(0x47, 9, 1).rx[0] = 0x94;
  }
  void turn(tusb32x_example::ExampleOwner& owner, tusb32x_cli::Cli& cli) {
    ++now;
    const auto before = bus.calls;
    owner.tick(now);
    const auto afterOwner = bus.calls;
    cli.tick();
    CHECK(bus.calls - before <= 1);
    if (!owner.startupDone()) CHECK(bus.calls == afterOwner);
  }
};

void blockedConsoleDoesNotOwnStartup() {
  Fixture fixture;
  tusb32x_cli::Cli cli(fixture.device, fixture.config, fixture.platform);
  tusb32x_example::ExampleOwner owner(fixture.device, 1000);
  CHECK(cli.setup().ok());
  // Deliberately fill output before startup. Output is not allowed to control its result lifetime.
  for (unsigned i = 0; i < 32; ++i) {
    CHECK(cli.processCommand("diag"));
    cli.tick();
  }
  CHECK(fixture.outputBytes == 0 && fixture.bus.calls == 0);
  CHECK(cli.console().droppedRecords() > 0);
  fixture.identity();
  CHECK(owner.start(fixture.now).inProgress());
  for (unsigned i = 0; i < 16 && owner.reportSequence() == 0; ++i)
    fixture.turn(owner, cli);
  CHECK(owner.reportSequence() == 1 && fixture.bus.complete());
  CHECK(owner.lastResult().kind == t::OperationKind::INITIALIZE);
  fixture.port();
  for (unsigned i = 0; i < 16 && !owner.startupDone(); ++i) fixture.turn(owner, cli);
  CHECK(owner.startupDone() && !owner.stopped() && fixture.bus.complete());
  CHECK(fixture.device.progress().state == t::OperationState::IDLE);
  CHECK(owner.reportSequence() == 2 && fixture.outputBytes == 0);

  fixture.port();
  CHECK(cli.processCommand("read"));
  const auto token = fixture.device.progress().token;
  CHECK(token.valid());
  for (unsigned i = 0; i < 16 && !fixture.device.progress().resultPending; ++i)
    fixture.turn(owner, cli);
  CHECK(fixture.bus.complete() && fixture.device.progress().resultPending);
  CHECK(owner.reportSequence() == 2); // Startup owner never consumes the new CLI result.
  const auto calls = fixture.bus.calls;
  for (unsigned i = 0; i < 8; ++i) fixture.turn(owner, cli);
  CHECK(fixture.bus.calls == calls && fixture.device.progress().resultPending);
  char command[64]{};
  std::snprintf(command, sizeof(command), "result take %llu",
                static_cast<unsigned long long>(token.value));
  CHECK(cli.processCommand(command));
  fixture.turn(owner, cli);
  CHECK(fixture.device.progress().resultPending); // Destructive take needs output reservation.
  fixture.stalled = false;
  for (unsigned i = 0; i < 64; ++i) fixture.turn(owner, cli);
  CHECK(fixture.outputBytes > 0);
  CHECK(cli.processCommand(command));
  fixture.turn(owner, cli);
  CHECK(fixture.device.progress().state == t::OperationState::IDLE);
  CHECK(fixture.bus.calls == calls);

  // A shutdown during CLI work cancels locally, retaining its token for explicit CLI take.
  CHECK(cli.processCommand("read"));
  const auto shutdownToken = fixture.device.progress().token;
  CHECK(owner.stop().ok());
  cli.tick();
  t::OperationResult stopped{};
  CHECK(fixture.device.resultSnapshot(shutdownToken, stopped).ok());
  CHECK(stopped.status.is(t::Err::CANCELLED));
  CHECK(fixture.bus.calls == calls);
}

void failedStartupLeavesDiagnosticCli() {
  Fixture fixture;
  tusb32x_cli::Cli cli(fixture.device, fixture.config, fixture.platform);
  tusb32x_example::ExampleOwner owner(fixture.device, 1000);
  CHECK(cli.setup().ok());
  fixture.identity();
  fixture.bus.script[0].status = t::Status::Error(t::Err::I2C_ERROR, "startup failed", -8123456);
  CHECK(owner.start(fixture.now).inProgress());
  fixture.turn(owner, cli);
  CHECK(owner.startupDone() && owner.stopped());
  CHECK(owner.lastResult().status.detail == -8123456);
  CHECK(fixture.device.progress().state == t::OperationState::IDLE);
  CHECK(cli.processCommand("diag"));
  fixture.turn(owner, cli);
  CHECK(fixture.bus.calls == 1);
  fixture.identity();
  CHECK(cli.processCommand("init 1000")); // Only an explicit later CLI request starts another attempt.
  const auto token = fixture.device.progress().token;
  for (unsigned i = 0; i < 16 && !fixture.device.progress().resultPending; ++i)
    fixture.turn(owner, cli);
  CHECK(fixture.bus.complete());
  t::OperationResult result{};
  CHECK(fixture.device.resultSnapshot(token, result).ok() && result.status.ok());
  CHECK(owner.reportSequence() == 1); // Stopped startup owner neither retries nor takes CLI work.
}
} // namespace

int main() {
  blockedConsoleDoesNotOwnStartup();
  failedStartupLeavesDiagnosticCli();
  std::printf("example integration: %u checks, %u failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
