/// @file cli_host.cpp
/// @brief Persistent application fixture executing the real CLI with an explicit scripted bus.
#include "Tusb32xCli.h"

#include <cstdio>
#include <cstring>
#include <cstdint>
#include <limits>

namespace tusb32x_cli {
/// @brief Fixture-only boundary setup; never exposed as a production command.
struct CliTestAccess {
  static void requestBase(Cli& cli, uint64_t value) { cli._lastRequest = value; }
};
}  // namespace tusb32x_cli

namespace {
namespace t = TUSB32x;

/// @brief Host-only neutral register storage with explicit callback and fault contracts.
/// @details This fixture models only the acceptance needed by the transcript, not silicon timing,
/// electrical behavior or hardware qualification. No production route controls its fault mode.
struct Bus {
  enum class Fault { NONE, TRANSPORT, READBACK };
  uint8_t registers[256]{};
  uint64_t clock = 100;
  uint32_t calls = 0;
  uint32_t writes = 0;
  uint32_t reads = 0;
  unsigned errors = 0;
  Fault fault = Fault::NONE;
  bool injected = false;
  bool written = false;

  Bus() {
    constexpr uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    std::memcpy(registers, identity, sizeof(identity));
    registers[0x08] = 0x80;
    registers[0x09] = 0x14; // Detached, 50% duty and a pending coalescing IRQ latch.
    registers[0x0A] = 0x30;
    registers[0xA0] = 2;
  }

  t::Status reject() {
    ++errors;
    return t::Status::Error(t::Err::I2C_ERROR, "unexpected host fixture callback", -90001);
  }

  t::Status outcome(bool reading) {
    if (!injected && (fault == Fault::TRANSPORT ||
        (fault == Fault::READBACK && reading && written))) {
      injected = true;
      return t::Status::Error(t::Err::I2C_TIMEOUT, "explicit host fixture fault",
                              std::numeric_limits<int32_t>::min());
    }
    return t::Status::Ok();
  }

  static t::Status write(uint8_t address, const uint8_t* data, std::size_t length,
                         uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    ++bus.calls;
    ++bus.writes;
    if ((address != 0x47 && address != 0x60 && address != 0x61 && address != 0x67) ||
        data == nullptr || length != 2 || timeout == 0 || timeout > 20 ||
        (data[0] != 0x08 && data[0] != 0x09 && data[0] != 0x0A && data[0] != 0x45))
      return bus.reject();
    const auto status = bus.outcome(false);
    if (!status.ok()) return status;
    bus.written = true;
    const auto reg = data[0];
    if (reg == 0x09) {
      // Explicit W1C acceptance: status bits persist; only observed duty is writable.
      const auto latch = static_cast<uint8_t>((data[1] & 0x10) ? 0 : bus.registers[reg] & 0x10);
      bus.registers[reg] = static_cast<uint8_t>((bus.registers[reg] & 0xE0) | latch | (data[1] & 0x06));
    } else if (reg == 0x0A) {
      // The fixture accepts reset and supplies a settled image on later reads. Core owns the wait.
      bus.registers[reg] = static_cast<uint8_t>(data[1] & ~uint8_t{0x08});
    } else bus.registers[reg] = data[1];
    return status;
  }

  static t::Status read(uint8_t address, const uint8_t* tx, std::size_t txLength,
                        uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    auto& bus = *static_cast<Bus*>(user);
    ++bus.calls;
    ++bus.reads;
    if ((address != 0x47 && address != 0x60 && address != 0x61 && address != 0x67) ||
        tx == nullptr || txLength != 1 || rx == nullptr || timeout == 0 || timeout > 20 ||
        !((tx[0] == 0 && rxLength == 8) || (rxLength == 1 &&
          (tx[0] == 0x08 || tx[0] == 0x09 || tx[0] == 0x0A || tx[0] == 0x45 || tx[0] == 0xA0))))
      return bus.reject();
    const auto status = bus.outcome(true);
    if (status.ok()) std::memcpy(rx, bus.registers + tx[0], rxLength);
    return status;
  }
};

uint64_t nowMs(void* user) { return static_cast<Bus*>(user)->clock; }
tusb32x_cli::AdapterCounters counters(void* user) {
  const auto& bus = *static_cast<Bus*>(user);
  return {true, bus.writes, bus.reads};
}
std::size_t writeOutput(const char* bytes, std::size_t length, void*) {
  return std::fwrite(bytes, 1, length, stdout);
}
bool parseRequestBase(const char* text, uint64_t& out) {
  if (!text || !*text) return false;
  uint64_t value = 0;
  for (const char* p = text; *p; ++p) {
    if (*p < '0' || *p > '9') return false;
    const auto digit = static_cast<uint64_t>(*p - '0');
    if (value > (std::numeric_limits<uint64_t>::max() - digit) / 10) return false;
    value = value * 10 + digit;
  }
  out = value;
  return true;
}
}  // namespace

int main(int argc, char** argv) {
  uint64_t requestBase = 0;
  Bus bus;
  for (int index = 1; index < argc; index += 2) {
    if (index + 1 >= argc) return 2;
    if (std::strcmp(argv[index], "--request-base") == 0) {
      if (!parseRequestBase(argv[index + 1], requestBase)) return 2;
    } else if (std::strcmp(argv[index], "--fault") == 0) {
      if (std::strcmp(argv[index + 1], "transport") == 0) bus.fault = Bus::Fault::TRANSPORT;
      else if (std::strcmp(argv[index + 1], "readback") == 0) bus.fault = Bus::Fault::READBACK;
      else return 2;
    } else return 2;
  }
  TUSB32x::TUSB32x driver;
  TUSB32x::DriverConfig config;
  config.variant = TUSB32x::Variant::TUSB320LAI;
  config.i2cWrite = Bus::write;
  config.i2cWriteRead = Bus::read;
  config.i2cUser = &bus;
  tusb32x_cli::Platform platform;
  platform.nowMs = nowMs;
  platform.write = writeOutput;
  platform.counters = counters;
  platform.user = &bus;
  platform.identity.build = "host-fixture";
  platform.identity.framework = "host";
  tusb32x_cli::Cli cli(driver, config, platform);
  if (!cli.setup().ok()) return 3;
  tusb32x_cli::CliTestAccess::requestBase(cli, requestBase);
  const auto tick = [&]() {
    ++bus.clock;
    const auto before = bus.calls;
    cli.tick();
    if (bus.calls - before > 1) ++bus.errors;
  };
  // The fixture owns blocking stdio. Each CLI call still offers only one byte and each
  // bounded flush offers at most one backend write; no production scheduler is replaced.
  for (int byte = std::getchar(); byte != EOF; byte = std::getchar()) {
    const char input = static_cast<char>(byte);
    if (!cli.exhausted()) cli.feed(&input, 1);
    tick();
    if (input == '\r' || input == '\n') {
      for (unsigned turn = 0; turn < 512; ++turn) tick();
      if (std::fflush(stdout) != 0) return 4;
    }
  }
  for (unsigned turn = 0; turn < 512; ++turn) tick();
  if (std::fflush(stdout) != 0) return 4;
  return bus.errors == 0 ? 0 : 5;
}
