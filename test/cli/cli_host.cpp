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
  /// @brief Drain output without advancing simulated owner time or polling managed work.
  static void drain(Cli& cli) {
    cli.helpStep();
    cli._console.drain(cli._platform.write, cli._platform.user);
  }
};
}  // namespace tusb32x_cli

namespace {
namespace t = TUSB32x;

/// @brief Host-only neutral register storage with explicit callback and fault contracts.
/// @details This fixture models only the acceptance needed by the transcript, not silicon timing,
/// electrical behavior or hardware qualification. No production route controls its fault mode.
struct Bus {
  enum class Fault {
    NONE, TRANSPORT, NACK, DISCONNECT, READBACK, WRITE, WRITE_APPLIED,
    ENABLE_WRITE, ENABLE_WRITE_APPLIED,
    LATE_WRITE, TAKE_FULL, RESET_TAKE_FULL
  };
  uint8_t registers[256]{};
  uint64_t clock = 100;
  uint32_t calls = 0;
  uint32_t writes = 0;
  uint32_t reads = 0;
  unsigned errors = 0;
  Fault fault = Fault::NONE;
  bool injected = false;
  bool written = false;
  int input = -1; ///< -1 absent, -2 failed, otherwise the explicit application input level.
  bool fixture = false; ///< Explicit launch-only physical model; never hardware evidence.
  const char* fixtureState = nullptr; ///< Borrowed path to bounded external observations.
  uint64_t fixtureVersion = 0; ///< Last applied observation; ACK must not reassert an old latch.
  bool fixtureLoaded = false; ///< Distinguishes initial version zero from an unchanged file.
  t::Variant variant = t::Variant::TUSB320LAI; ///< Explicit launch-only modeled silicon.
  const char* tracePath = nullptr; ///< Optional test-owned callback evidence, never CLI output.
  bool armOutputStall = false; ///< Launch-only file action targeting the next real help response.
  unsigned requestedOutputStalls = 2; ///< Explicit successful/loss backpressure model duration.
  unsigned outputStalls = 0; ///< Remaining bounded zero-write backend responses.

  Bus() {
    constexpr uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    std::memcpy(registers, identity, sizeof(identity));
    registers[0x08] = 0x80;
    registers[0x09] = 0x14; // Detached, 50% duty and a pending coalescing IRQ latch.
    registers[0x0A] = 0x30;
    registers[0xA0] = 2;
  }

  /// @brief Apply one independently scripted physical observation before a command line.
  /// @details Decimal version/08/09 fields provide RO CC/attachment and IRQ inputs.
  /// Writable controls remain owned by driver callbacks except an explicit modeled power-cycle
  /// action. An optional action token arms one fault or bounded next-help output stall. Reading
  /// the same version preserves cleared W1C/fault evidence; malformed/regressing input fails closed.
  bool observeFixture() {
    if (!fixture) return true;
    FILE* source = std::fopen(fixtureState, "rb");
    if (!source) return false;
    char text[128]{};
    const auto size = std::fread(text, 1, sizeof(text) - 1, source);
    const bool complete = !std::ferror(source) && std::feof(source);
    const bool closed = std::fclose(source) == 0;
    unsigned long long version = 0;
    unsigned raw08 = 0;
    unsigned raw09 = 0;
    char action[32]{};
    char extra = 0;
    const int fields = std::sscanf(text, "%llu %u %u %31s %c", &version, &raw08, &raw09,
                                   action, &extra);
    if (!complete || !closed || size == 0 ||
        (fields != 3 && fields != 4) ||
        raw08 > 255 || raw09 > 255 || version < fixtureVersion) return false;
    if (fixtureLoaded && version == fixtureVersion) return true;
    fixtureVersion = static_cast<uint64_t>(version);
    fixtureLoaded = true;
    if (fields == 4) {
      if (std::strcmp(action, "nack") == 0 || std::strcmp(action, "disconnect") == 0) {
        fault = std::strcmp(action, "nack") == 0 ? Fault::NACK : Fault::DISCONNECT;
        injected = false;
      } else if (std::strcmp(action, "power-cycle") == 0) {
        if (variant != t::Variant::TUSB320LAI) return false;
        // Independent LAI power-on values: SLLSEQ8D Tables8-11 pp17-20. This
        // external power-cycle model is distinct from the self-clearing software reset.
        registers[0x08] = 0;
        registers[0x09] = 0x20;
        registers[0x0A] = 0;
        registers[0x45] = 0;
        fault = Fault::NONE;
      } else if (std::strcmp(action, "backpressure") == 0 ||
                 std::strcmp(action, "backpressure-loss") == 0) {
        armOutputStall = true;
        requestedOutputStalls = std::strcmp(action, "backpressure-loss") == 0 ? 64U : 2U;
      } else return false;
    }
    registers[0x08] = static_cast<uint8_t>((registers[0x08] & 0xC1) | (raw08 & 0x3E));
    registers[0x09] = static_cast<uint8_t>((registers[0x09] & 0x07) | (raw09 & 0xF0));
    return true;
  }

  t::Status reject() {
    ++errors;
    return t::Status::Error(t::Err::I2C_ERROR, "unexpected host fixture callback", -90001);
  }

  t::Status outcome(bool reading) {
    if (!injected && (fault == Fault::TRANSPORT || fault == Fault::NACK ||
        fault == Fault::DISCONNECT ||
        ((fault == Fault::WRITE || fault == Fault::WRITE_APPLIED) && !reading) ||
        ((fault == Fault::ENABLE_WRITE || fault == Fault::ENABLE_WRITE_APPLIED) &&
         !reading && written) ||
        (fault == Fault::READBACK && reading && written))) {
      injected = true;
      const auto code = fault == Fault::NACK ? t::Err::I2C_NACK_ADDR :
          fault == Fault::DISCONNECT ? t::Err::I2C_BUS : t::Err::I2C_TIMEOUT;
      return t::Status::Error(code, "explicit host fixture fault",
                              std::numeric_limits<int32_t>::min());
    }
    return t::Status::Ok();
  }

  /// @brief Save terminal callback bytes/outcome or a modeled zero-write output observation.
  /// @details Trace I/O belongs only to this host fixture. Files are closed after each bounded
  /// record so the Python oracle can inspect exact write boundaries before cancellation.
  void trace(uint64_t entered, char direction, uint8_t address, uint8_t reg,
             uint8_t value, std::size_t length, const t::Status& status) {
    if (!tracePath) return;
    FILE* target = std::fopen(tracePath, "ab");
    if (!target) { ++errors; return; }
    if (std::fprintf(target, "%llu %llu %c %u %u %u %zu %u %ld\n",
        static_cast<unsigned long long>(entered), static_cast<unsigned long long>(clock),
        direction, static_cast<unsigned>(address), static_cast<unsigned>(reg),
        static_cast<unsigned>(value), length, status.ok() ? 1U : 0U,
        static_cast<long>(status.detail)) < 0) ++errors;
    if (std::fclose(target) != 0) ++errors;
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
    const auto entered = bus.clock;
    if (!status.ok() && bus.fault != Fault::WRITE_APPLIED &&
        bus.fault != Fault::ENABLE_WRITE_APPLIED) {
      bus.trace(entered, 'W', address, data[0], data[1], length, status);
      return status;
    }
    bus.written = true;
    const auto reg = data[0];
    if (reg == 0x09) {
      // Explicit W1C acceptance: status persists; only persistent controls are writable.
      const auto latch = static_cast<uint8_t>((data[1] & 0x10) ? 0 : bus.registers[reg] & 0x10);
      bus.registers[reg] = static_cast<uint8_t>((bus.registers[reg] & 0xE0) | latch | (data[1] & 0x07));
    } else if (reg == 0x0A) {
      // SLLSEN9F Table7-7 p19 / SLLSEQ8D Table10 p19 specify a self-clearing command,
      // not a full software-reset default image. Retaining other controls is a modeled outcome.
      // SLLSEO5D 6.3.2 p9 separately promises reset-to-DRP: model only that mode change.
      bus.registers[reg] = static_cast<uint8_t>(data[1] & ~uint8_t{0x08});
      if ((data[1] & 0x08) && bus.variant == t::Variant::TUSB322I)
        bus.registers[reg] &= static_cast<uint8_t>(~uint8_t{0x30});
    } else if (reg == 0x08 && bus.fixture) {
      bus.registers[reg] = static_cast<uint8_t>((bus.registers[reg] & 0x3F) | (data[1] & 0xC0));
    } else bus.registers[reg] = data[1];
    if (!bus.injected && bus.fault == Fault::LATE_WRITE) {
      bus.injected = true;
      bus.clock += 1000; // Deliberately late backend success; owner must reject expired work.
    }
    bus.trace(entered, 'W', address, reg, data[1], length, status);
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
          (tx[0] <= 0x0A || tx[0] == 0x45 || tx[0] == 0xA0))))
      return bus.reject();
    const auto status = bus.outcome(true);
    if (status.ok()) std::memcpy(rx, bus.registers + tx[0], rxLength);
    bus.trace(bus.clock, 'R', address, tx[0], status.ok() ? rx[0] : 0, rxLength, status);
    return status;
  }
};

uint64_t nowMs(void* user) { return static_cast<Bus*>(user)->clock; }
/// @brief Return only the explicitly selected test input; no GPIO or transport work.
bool readInput(bool& value, void* user) {
  const auto& bus = *static_cast<Bus*>(user);
  if (bus.fixture) {
    value = (bus.registers[0x09] & 0x10) == 0;
    return true;
  }
  const auto input = bus.input;
  if (input < 0) return false;
  value = input != 0;
  return true;
}
tusb32x_cli::AdapterCounters counters(void* user) {
  const auto& bus = *static_cast<Bus*>(user);
  return {true, bus.writes, bus.reads};
}
std::size_t writeOutput(const char* bytes, std::size_t length, void* user) {
  auto& bus = *static_cast<Bus*>(user);
  if (bus.outputStalls > 0) {
    --bus.outputStalls;
    bus.trace(bus.clock, 'Z', 0, 0, 0, length, t::Status::Ok());
    return 0; // Native zero-write/backpressure model; no USB/serial timing claim.
  }
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
  bool observation = false;
  Bus bus;
  for (int index = 1; index < argc; index += 2) {
    if (index + 1 >= argc) return 2;
    if (std::strcmp(argv[index], "--request-base") == 0) {
      if (!parseRequestBase(argv[index + 1], requestBase)) return 2;
    } else if (std::strcmp(argv[index], "--fault") == 0) {
      if (std::strcmp(argv[index + 1], "transport") == 0) bus.fault = Bus::Fault::TRANSPORT;
      else if (std::strcmp(argv[index + 1], "readback") == 0) bus.fault = Bus::Fault::READBACK;
      else if (std::strcmp(argv[index + 1], "write") == 0) bus.fault = Bus::Fault::WRITE;
      else if (std::strcmp(argv[index + 1], "write-applied") == 0)
        bus.fault = Bus::Fault::WRITE_APPLIED;
      else if (std::strcmp(argv[index + 1], "late-write") == 0)
        bus.fault = Bus::Fault::LATE_WRITE;
      else if (std::strcmp(argv[index + 1], "enable-write") == 0)
        bus.fault = Bus::Fault::ENABLE_WRITE;
      else if (std::strcmp(argv[index + 1], "enable-write-applied") == 0)
        bus.fault = Bus::Fault::ENABLE_WRITE_APPLIED;
      else if (std::strcmp(argv[index + 1], "take-full") == 0) bus.fault = Bus::Fault::TAKE_FULL;
      else if (std::strcmp(argv[index + 1], "reset-take-full") == 0)
        bus.fault = Bus::Fault::RESET_TAKE_FULL;
      else return 2;
    } else if (std::strcmp(argv[index], "--variant") == 0) {
      if (std::strcmp(argv[index + 1], "tusb320") == 0) bus.variant = t::Variant::TUSB320;
      else if (std::strcmp(argv[index + 1], "tusb320i") == 0) bus.variant = t::Variant::TUSB320I;
      else if (std::strcmp(argv[index + 1], "tusb320lai") == 0)
        bus.variant = t::Variant::TUSB320LAI;
      else if (std::strcmp(argv[index + 1], "tusb322i") == 0)
        bus.variant = t::Variant::TUSB322I;
      else return 2;
    } else if (std::strcmp(argv[index], "--trace") == 0) {
      if (argv[index + 1][0] == '\0') return 2;
      bus.tracePath = argv[index + 1];
    } else if (std::strcmp(argv[index], "--observation") == 0) {
      if (std::strcmp(argv[index + 1], "1") != 0) return 2;
      observation = true;
    } else if (std::strcmp(argv[index], "--fixture") == 0) {
      if (std::strcmp(argv[index + 1], "1") != 0) return 2;
      bus.fixture = true;
      observation = true;
    } else if (std::strcmp(argv[index], "--fixture-state") == 0) {
      if (argv[index + 1][0] == '\0') return 2;
      bus.fixtureState = argv[index + 1];
    } else if (std::strcmp(argv[index], "--input") == 0) {
      if (std::strcmp(argv[index + 1], "0") == 0) bus.input = 0;
      else if (std::strcmp(argv[index + 1], "1") == 0) bus.input = 1;
      else if (std::strcmp(argv[index + 1], "unavailable") == 0) bus.input = -1;
      else if (std::strcmp(argv[index + 1], "failure") == 0) bus.input = -2;
      else return 2;
    } else return 2;
  }
  if (bus.fixture != (bus.fixtureState != nullptr)) return 2;
  if (bus.tracePath) {
    FILE* target = std::fopen(bus.tracePath, "wb");
    if (!target) return 8;
    if (std::fclose(target) != 0) return 8;
  }
  if (bus.variant == t::Variant::TUSB322I) bus.registers[0] = 0x32;
  TUSB32x::TUSB32x driver;
  TUSB32x::DriverConfig config;
  config.variant = bus.variant;
  if (bus.variant == t::Variant::TUSB320 || bus.variant == t::Variant::TUSB320I)
    config.address = t::DeviceAddress::ADDR_LOW_0X60;
  config.i2cWrite = Bus::write;
  config.i2cWriteRead = Bus::read;
  config.i2cUser = &bus;
  tusb32x_cli::Platform platform;
  platform.nowMs = nowMs;
  platform.write = writeOutput;
  platform.counters = counters;
  if (bus.input != -1 || bus.fixture) platform.readInput = readInput;
  platform.user = &bus;
  platform.identity.build = "host-fixture";
  platform.identity.framework = "host";
  if (bus.fixture) {
    platform.identity.git = "0123456789abcdef0123456789abcdef01234567";
    platform.identity.tree = "clean";
    platform.identity.frameworkVersion = "test";
    platform.identity.target = "native";
  }
  tusb32x_cli::Cli cli(driver, config, platform);
  if (!cli.setup().ok()) return 3;
  tusb32x_cli::CliTestAccess::requestBase(cli, requestBase);
  const auto tick = [&]() {
    ++bus.clock;
    const auto before = bus.calls;
    cli.tick();
    if (bus.calls - before > 1) ++bus.errors;
  };
  if (observation) {
    // Keep command work separate from output draining. A cached query can therefore
    // observe an older active operation before another bounded owner turn advances it.
    char line[193]{};
    while (std::fgets(line, sizeof(line), stdin) != nullptr) {
      if (!bus.observeFixture()) return 8;
      const auto length = std::strlen(line);
      if (length == 0 || line[length - 1] != '\n') return 6;
      if (bus.armOutputStall && std::strcmp(line, "help\n") == 0) {
        bus.armOutputStall = false;
        bus.outputStalls = bus.requestedOutputStalls;
      }
      const bool congested = bus.fault == Bus::Fault::TAKE_FULL ||
          (bus.fault == Bus::Fault::RESET_TAKE_FULL &&
           driver.progress().kind == t::OperationKind::RESET);
      const bool denyTake = congested && !bus.injected &&
          std::strncmp(line, "result take ", 12) == 0;
      if (denyTake) {
        char filler[511];
        std::memset(filler, 'x', sizeof(filler));
        std::memcpy(filler, "APP ", 4);
        filler[sizeof(filler) - 1] = '\n';
        for (unsigned index = 0; index < 12; ++index)
          if (!cli.enqueueApplicationRecord(filler, sizeof(filler))) ++bus.errors;
        if (cli.console().available() >= 5U * 511U || cli.console().available() <= 1500)
          ++bus.errors;
        bus.injected = true;
      }
      std::size_t offset = 0;
      while (offset < length) {
        const auto used = cli.feed(line + offset, length - offset);
        if (used == 0) return 7;
        offset += used;
        tick();
      }
      if (denyTake && (!driver.progress().resultPending ||
          cli.console().droppedRecords() != 0 || cli.console().droppedBytes() != 0))
        ++bus.errors;
      for (unsigned turn = 0; turn < 512; ++turn) tusb32x_cli::CliTestAccess::drain(cli);
      if (std::fflush(stdout) != 0) return 4;
    }
    if ((bus.fault == Bus::Fault::TAKE_FULL || bus.fault == Bus::Fault::RESET_TAKE_FULL) &&
        !bus.injected) ++bus.errors;
    return bus.errors == 0 ? 0 : 5;
  }
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
