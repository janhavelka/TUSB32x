/// @file cli_host.cpp
/// @brief Persistent application fixture executing the real CLI with an explicit scripted bus.
#include "Tusb32xCli.h"
#include "../support/FakeTusb32xDevice.h"

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
uint64_t nowMs(void*) { return 100; }
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
  if (argc != 1 && (argc != 3 || std::strcmp(argv[1], "--request-base") != 0 ||
      !parseRequestBase(argv[2], requestBase))) return 2;
  tusb32x_test::FakeTusb32xDevice bus;
  TUSB32x::TUSB32x driver;
  TUSB32x::DriverConfig config;
  config.variant = TUSB32x::Variant::TUSB320LAI;
  config.i2cWrite = tusb32x_test::FakeTusb32xDevice::write;
  config.i2cWriteRead = tusb32x_test::FakeTusb32xDevice::writeRead;
  config.i2cUser = &bus;
  tusb32x_cli::Platform platform;
  platform.nowMs = nowMs;
  platform.write = writeOutput;
  platform.identity.build = "host-fixture";
  platform.identity.framework = "host";
  tusb32x_cli::Cli cli(driver, config, platform);
  if (!cli.setup().ok()) return 3;
  tusb32x_cli::CliTestAccess::requestBase(cli, requestBase);
  // The fixture owns blocking stdio. Each CLI call still offers only one byte and each
  // bounded flush offers at most one backend write; no production scheduler is replaced.
  for (int byte = std::getchar(); byte != EOF; byte = std::getchar()) {
    const char input = static_cast<char>(byte);
    if (!cli.exhausted()) cli.feed(&input, 1);
    cli.tick();
    if (input == '\r' || input == '\n') {
      for (unsigned turn = 0; turn < 512; ++turn) cli.tick();
      if (std::fflush(stdout) != 0) return 4;
    }
  }
  for (unsigned turn = 0; turn < 512; ++turn) cli.tick();
  if (std::fflush(stdout) != 0) return 4;
  return bus.calls == 0 && bus.errors == 0 ? 0 : 5;
}
