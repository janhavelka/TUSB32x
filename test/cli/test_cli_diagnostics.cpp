/// @file test_cli_diagnostics.cpp
/// @brief Executed diagnostic routes, exact callback accounting and passive evidence preservation.
/// @details Literal write oracles use TI SLLSEQ8D Tables 8-11 pp17-20: 08 current,
/// 09 duty/W1C and 0A mode/reset. The fixture models only explicit register and W1C acceptance.
#include "Tusb32xCli.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace {
namespace t = TUSB32x;
unsigned checks = 0;
unsigned failures = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    ++failures;
    std::printf("FAIL CLI diagnostics line %d: %s\n", line, expression);
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

struct Step {
  uint8_t reg;
  uint8_t value;
  bool writing;
  std::size_t length;
  t::Status status{};
};

/// @brief A scripted bus whose accepted writes, including failed writes, have observable effects.
struct Fixture {
  t::TUSB32x driver;
  t::DriverConfig config;
  tusb32x_cli::Platform platform;
  tusb32x_cli::Cli cli;
  std::vector<Step> script;
  std::size_t calls = 0;
  uint8_t registers[256]{};
  uint64_t now = 100;
  std::string output;
  std::size_t writeLimit = 256;
  unsigned pinReads = 0;
  bool pinAvailable = true;
  bool pinValue = false;

  explicit Fixture(t::Variant variant = t::Variant::TUSB320LAI, bool pin = false)
      : config(configuration(this, variant)), platform(hooks(this, pin)), cli(driver, config, platform) {
    registers[9] = 0x04;
    registers[10] = 0x30;
    registers[0xA0] = variant == t::Variant::TUSB320LI || variant == t::Variant::TUSB320HI ? 1 : 2;
    CHECK(cli.setup().ok());
    CHECK(command("job auto 0").find("code=OK ") != std::string::npos);
  }
  static t::DriverConfig configuration(Fixture* fixture, t::Variant variant) {
    t::DriverConfig result;
    result.variant = variant;
    result.address = variant == t::Variant::TUSB320 || variant == t::Variant::TUSB320I
        ? t::DeviceAddress::ADDR_LOW_0X60 : t::DeviceAddress::ADDR_LOW_0X47;
    result.i2cWrite = writeBus;
    result.i2cWriteRead = readBus;
    result.i2cUser = fixture;
    return result;
  }
  static uint64_t clock(void* user) { return static_cast<Fixture*>(user)->now; }
  static std::size_t writeOutput(const char* data, std::size_t length, void* user) {
    CHECK(length <= 256);
    auto& fixture = *static_cast<Fixture*>(user);
    const auto accepted = std::min(length, fixture.writeLimit);
    fixture.output.append(data, accepted);
    return accepted;
  }
  static bool readPin(bool& value, void* user) {
    auto& fixture = *static_cast<Fixture*>(user);
    ++fixture.pinReads;
    value = fixture.pinValue;
    return fixture.pinAvailable;
  }
  static tusb32x_cli::Platform hooks(Fixture* fixture, bool pin) {
    tusb32x_cli::Platform result;
    result.nowMs = clock;
    result.write = writeOutput;
    result.readInput = pin ? readPin : nullptr;
    result.user = fixture;
    return result;
  }
  t::Status invoke(bool writing, uint8_t address, const uint8_t* tx, std::size_t txLength,
                   uint8_t* rx, std::size_t rxLength, uint32_t timeout) {
    const auto index = calls++;
    CHECK(index < script.size());
    if (index >= script.size()) return t::Status::Error(t::Err::I2C_ERROR, "unexpected callback", -90001);
    const auto expected = script[index];
    CHECK(address == static_cast<uint8_t>(config.address));
    CHECK(timeout == 20 && tx != nullptr && txLength == (writing ? 2U : 1U));
    CHECK(writing == expected.writing && tx[0] == expected.reg);
    CHECK(rxLength == (writing ? 0 : expected.length));
    if (writing) {
      CHECK(rx == nullptr && tx[1] == expected.value);
      if (tx[0] == 9) {
        const uint8_t pending = (tx[1] & 0x10U) ? 0 : registers[9] & 0x10U;
        registers[9] = static_cast<uint8_t>((registers[9] & 0xE0U) | (tx[1] & 7U) | pending);
      } else registers[tx[0]] = tx[1];
    } else if (rx != nullptr) {
      if (expected.reg == 0 && rxLength == 8) {
        const uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
        std::memcpy(rx, identity, 8);
        if (config.variant == t::Variant::TUSB322I) rx[0] = 0x32;
      } else rx[0] = registers[expected.reg];
    }
    return expected.status;
  }
  static t::Status writeBus(uint8_t address, const uint8_t* tx, std::size_t length,
                            uint32_t timeout, void* user) {
    return static_cast<Fixture*>(user)->invoke(true, address, tx, length, nullptr, 0, timeout);
  }
  static t::Status readBus(uint8_t address, const uint8_t* tx, std::size_t length,
                           uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    return static_cast<Fixture*>(user)->invoke(false, address, tx, length, rx, rxLength, timeout);
  }
  void read(uint8_t reg, std::size_t length = 1) { script.push_back({reg, 0, false, length, {}}); }
  void write(uint8_t reg, uint8_t value) { script.push_back({reg, value, true, 0, {}}); }
  void proof() {
    read(0, 8);
    if (config.variant != t::Variant::TUSB320 && config.variant != t::Variant::TUSB320I) read(0xA0);
    read(8); read(9); read(10);
    if (config.variant != t::Variant::TUSB322I) read(0x45);
  }
  void tick() {
    const auto before = calls;
    cli.tick();
    CHECK(calls - before <= 1);
  }
  std::string command(const std::string& line, unsigned callbacks = 0) {
    output.clear();
    const auto before = calls;
    CHECK(cli.processCommand(line.c_str()));
    CHECK(calls - before == callbacks); // Only raw diagnostics execute in the handler.
    tick();
    for (unsigned turn = 0; turn < 256; ++turn) tick(); // Also finishes incremental help.
    CHECK(cli.console().queued() == 0 && cli.console().droppedRecords() == 0);
    CHECK(calls - before == callbacks);
    std::size_t start = 0;
    while (start < output.size()) {
      const auto end = output.find('\n', start);
      CHECK(end != std::string::npos);
      if (end == std::string::npos) break;
      CHECK(end - start + 2 <= 512); // Include newline and C terminator.
      start = end + 1;
    }
    return output;
  }
  void finish(t::OperationToken token, bool take = true) {
    for (unsigned turn = 0; turn < 32 && driver.progress().state == t::OperationState::ACTIVE; ++turn) {
      const auto before = calls;
      driver.poll(token, now, 255);
      CHECK(calls - before <= 1);
    }
    CHECK(driver.progress().resultPending && calls == script.size());
    if (take) { t::OperationResult result; CHECK(driver.takeResult(token, result).ok()); }
  }
  void initialize() {
    CHECK(driver.bind(config).ok());
    proof();
    t::OperationToken token;
    CHECK(driver.startInitialize({now, now + 1000}, token).inProgress());
    finish(token);
    CHECK(driver.configurationSnapshot().state == t::ConfigurationState::KNOWN);
  }
  void failure() {
    read(8);
    script.back().status = t::Status::Error(t::Err::I2C_TIMEOUT, "managed failure", -123456789);
    t::OperationToken token;
    CHECK(driver.startReadProfile({now, now + 1000}, token).inProgress());
    finish(token);
  }
  /// @brief Compare every published diagnostic/configuration/port/result field, excluding framing IDs.
  std::string evidence() {
    std::string all;
    for (const auto* query : {"cfg", "sample", "diag", "irq", "xfer_stats", "job", "result", "variant", "addr"}) {
      const auto text = command(query);
      for (std::size_t start = 0; start < text.size();) {
        const auto end = text.find('\n', start);
        const auto line = text.substr(start, end - start);
        if (line.rfind("TUSB ", 0) == 0 && line.rfind("TUSB ACK ", 0) != 0 && line.rfind("TUSB END ", 0) != 0) {
          const auto request = line.find(" request=");
          const auto after = request == std::string::npos ? request : line.find(' ', request + 9);
          all += line.substr(0, line.find(" schema="));
          if (after != std::string::npos) all += line.substr(after);
          all += '\n';
        }
        if (end == std::string::npos) break;
        start = end + 1;
      }
    }
    return all;
  }
  void rejected(const std::string& line, const char* code = "INVALID_PARAM") {
    const auto before = evidence();
    const auto stagedGeneration = cli.stagedGeneration();
    const auto busImage = std::vector<uint8_t>(registers, registers + 256);
    const auto text = command(line);
    CHECK(text.find(std::string("code=") + code + " ") != std::string::npos);
    CHECK(text.find("outcome=FAILED") != std::string::npos);
    CHECK(text.find("action=UNCHANGED ") != std::string::npos);
    CHECK(before == evidence() && stagedGeneration == cli.stagedGeneration());
    CHECK(std::equal(busImage.begin(), busImage.end(), registers));
  }
};

void cachedRoutesAndHelp() {
  Fixture f;
  for (const auto* name : {"drv", "health", "state"}) {
    const auto text = f.command(name);
    CHECK(text.find("command=drv code=OK ") != std::string::npos);
    CHECK(text.find("state=UNINIT threshold=3") != std::string::npos);
  }
  for (const auto* name : {"xfer_stats", "counters"}) {
    const auto text = f.command(name);
    CHECK(text.find("command=xfer_stats code=OK ") != std::string::npos);
    CHECK(text.find("source=all_callbacks attempts=0 success=0 failures=0") != std::string::npos);
  }
  CHECK(f.command("irq").find("source=managed_operation observed=0") != std::string::npos);
  CHECK(f.command("irq").find("pending_after=unknown pending_after_valid=0") != std::string::npos);
  const auto diag = f.command("diag");
  for (const auto* type : {"DIAG", "MANAGED", "FAILURE", "IDENTITY", "IRQ", "XFER"})
    CHECK(diag.find(std::string("TUSB ") + type + " schema=1 ") != std::string::npos);
  for (const auto* key : {"bound=", "initialized=", "configuration=", "effect=", "termination=", "reset=",
      "config_write_unresolved=", "recovery_unresolved=", "mode_unresolved=", "reset_acceptance_uncertain=",
      "last_kind=", "last_code=", "last_detail=", "identity_valid=", "identity_matches=", "bytes=",
      "revision_valid=", "revision_matches=", "last_success_ms=", "last_success_time_valid=",
      "last_failure_ms=", "last_failure_time_valid=", "time_ms=", "time_valid="})
    CHECK(diag.find(key) != std::string::npos);
  CHECK(f.command("xfer_assert 0").find("assertion=PASS") != std::string::npos);
  CHECK(f.command("xfer_assert 0 0 0").find("assertion=PASS") != std::string::npos);
  CHECK(f.command("xfer_reset").find("command=xfer_reset code=OK ") != std::string::npos);
  for (const auto* topic : {"registers", "diagnostics", "reg", "rreg", "rawread", "decode", "wreg", "rawwrite",
      "irq", "intpin", "invalidate", "drv", "health", "state", "diag", "xfer_stats", "counters", "xfer_reset", "xfer_assert"}) {
    const auto text = f.command(std::string("help ") + topic);
    CHECK(text.find("command=help outcome=SUCCEEDED") != std::string::npos);
  }
  CHECK(f.calls == 0);
}

void rawReadAliasesAndFailures() {
  Fixture f;
  f.rejected("reg 8", "NOT_BOUND");
  f.rejected("wreg 8 64", "NOT_BOUND");
  f.command("bind");
  f.rejected("wreg 8 64", "NOT_INITIALIZED");
  f.rejected("wreg 0 0", "UNSUPPORTED_FEATURE");
  f.rejected("wreg 160 0", "UNSUPPORTED_FEATURE");
  for (const auto* name : {"reg", "rreg", "rawread"}) {
    f.registers[8] = 0xFF;
    f.read(8);
    const auto text = f.command(std::string(name) + " 0x08", 1);
    CHECK(text.find("command=reg code=OK ") != std::string::npos);
    CHECK(text.find("source=read variant=tusb320lai address=8 value=255") != std::string::npos);
    CHECK(text.find("command=reg outcome=SUCCEEDED code=OK detail=0 token=0 callbacks=1") != std::string::npos);
    CHECK(text.find("current=unknown") != std::string::npos);
  }
  CHECK(f.driver.transportStats().totalAttempts == 3 && f.driver.diagnostics().managedTransport.totalSuccess == 0);
  for (const auto error : {t::Err::I2C_ERROR, t::Err::I2C_TIMEOUT, t::Err::I2C_NACK_ADDR,
      t::Err::I2C_NACK_DATA, t::Err::I2C_BUSY}) {
    f.read(8);
    f.script.back().status = t::Status::Error(error, "raw backend error", std::numeric_limits<int32_t>::min());
    const auto text = f.command("reg 8", 1);
    CHECK(text.find(std::string("command=reg code=") + t::errorName(error) + " detail=-2147483648") != std::string::npos);
    CHECK(text.find("callbacks=1") != std::string::npos && text.find("TUSB REG ") == std::string::npos);
  }
  f.read(8);
  f.script.back().status = t::Status::Error(t::Err::INVALID_CONFIG, "illegal callback code", -77);
  const auto normalized = f.command("reg 8", 1);
  CHECK(normalized.find("code=I2C_ERROR detail=-77") != std::string::npos);
  CHECK(f.driver.transportStats().totalAttempts == 9 && f.driver.transportStats().totalFailures == 6);
  CHECK(f.driver.diagnostics().managedTransport.totalFailures == 0);
  CHECK(f.command("drv").find("state=UNINIT") != std::string::npos);
  const auto mismatch = f.command("xfer_assert 8 3 6");
  CHECK(mismatch.find("code=INVALID_PARAM ") != std::string::npos && mismatch.find("assertion=FAIL") != std::string::npos);
  CHECK(mismatch.find("source=all_callbacks attempts=9 success=3 failures=6") != std::string::npos);
  CHECK(f.command("xfer_assert 9 3 6").find("assertion=PASS") != std::string::npos);
  f.command("hilmark 01234567890123456789012345678901");
  const auto widest = f.command("xfer_assert 4294967295 4294967295 4294967295");
  CHECK(widest.find("nonce=01234567890123456789012345678901") != std::string::npos);
  CHECK(widest.find("expected_attempts=4294967295 expected_success=4294967295 expected_failures=4294967295") != std::string::npos);
  CHECK(widest.find("assertion=FAIL") != std::string::npos);
}

void pureDecodeAndVariantGates() {
  Fixture f;
  auto text = f.command("decode 8 255");
  CHECK(text.find("source=supplied variant=tusb320lai address=8 value=255") != std::string::npos);
  CHECK(text.find("profile_valid_fields=0 status_valid_fields=208") != std::string::npos);
  CHECK(text.find("current=unknown") != std::string::npos);
  CHECK(text.find("detected_current=MA_3000") != std::string::npos);
  text = f.command("decode 8 0x20"); // TI Table 8 CURRENT_MODE_DETECT code 10 is charge-through.
  CHECK(text.find("detected_current=CHARGE_THROUGH_500_MA") != std::string::npos);
  text = f.command("decode 10 0x34"); // preference 10 is reserved.
  CHECK(text.find("preference=unknown") != std::string::npos);
  CHECK(text.find("profile_valid_fields=81 status_valid_fields=0") != std::string::npos);
  CHECK(text.find("reset_requested=0 reset_valid=1") != std::string::npos);
  text = f.command("decode 9 0x20"); // Raw direction is CC2 even with detached attachment code.
  CHECK(text.find("attachment=DETACHED orientation=CC2") != std::string::npos);
  CHECK(text.find("TUSB DECODE_PROFILE ") != std::string::npos && text.find("valid_fields=") != std::string::npos);
  for (const auto variant : {t::Variant::TUSB320, t::Variant::TUSB320I, t::Variant::TUSB320LI,
      t::Variant::TUSB320HI, t::Variant::TUSB320LAI, t::Variant::TUSB320HAI, t::Variant::TUSB322I}) {
    Fixture part(variant);
    part.command("bind");
    for (unsigned reg = 0; reg <= 10; ++reg) {
      const auto decoded = part.command("decode " + std::to_string(reg) + " 255");
      CHECK(decoded.find("command=decode code=OK ") != std::string::npos);
      CHECK(decoded.find("value=255") != std::string::npos);
    }
    part.rejected("decode 11 0");
    part.rejected("reg 11");
    part.rejected("wreg 11 0");
    const bool base = variant == t::Variant::TUSB320 || variant == t::Variant::TUSB320I;
    if (base) {
      part.rejected("decode 160 2", "UNSUPPORTED_FEATURE");
      part.rejected("reg 160", "UNSUPPORTED_FEATURE");
    }
    if (variant == t::Variant::TUSB322I) {
      part.rejected("decode 69 0", "UNSUPPORTED_FEATURE");
      part.rejected("reg 69", "UNSUPPORTED_FEATURE");
      part.rejected("wreg 8 64", "UNSUPPORTED_FEATURE");
      part.rejected("wreg 9 20", "UNSUPPORTED_FEATURE");
    }
    CHECK(part.calls == 0);
  }
}

void everyRawCallbackStatus() {
  // Enum values are the public contiguous Err contract. Invalid uint8 values must
  // also normalize, and must still count as the one actual raw callback attempt.
  for (unsigned value = 0; value <= static_cast<unsigned>(t::Err::I2C_BUSY) + 1U; ++value) {
    const auto error = static_cast<t::Err>(value == static_cast<unsigned>(t::Err::I2C_BUSY) + 1U ? 255U : value);
    const auto normalized = error == t::Err::OK || (error >= t::Err::I2C_ERROR && error <= t::Err::I2C_BUSY)
        ? error : t::Err::I2C_ERROR;
    for (const bool writing : {false, true}) {
      Fixture f;
      if (writing) { f.initialize(); f.write(8, 0x40); }
      else { f.command("bind"); f.read(8); }
      const int32_t detail = error == t::Err::OK ? 0 : std::numeric_limits<int32_t>::min();
      f.script.back().status = t::Status::Error(error, "callback status table", detail);
      const auto before = f.driver.transportStats();
      const auto managed = f.driver.diagnostics().managedTransport;
      const auto text = f.command(writing ? "wreg 8 64" : "reg 8", 1);
      const std::string typed = std::string("code=") + t::errorName(normalized) + " detail=" + std::to_string(detail);
      CHECK(text.find(typed) != std::string::npos);
      CHECK(text.find("action=ATTEMPTED ") != std::string::npos && text.find("callbacks=1") != std::string::npos);
      const auto after = f.driver.transportStats();
      CHECK(after.totalAttempts == before.totalAttempts + 1);
      CHECK(after.totalSuccess == before.totalSuccess + (normalized == t::Err::OK ? 1U : 0U));
      CHECK(after.totalFailures == before.totalFailures + (normalized == t::Err::OK ? 0U : 1U));
      CHECK(after.lastStatus.code == normalized && after.lastStatus.detail == detail);
      CHECK(f.driver.diagnostics().managedTransport.totalSuccess == managed.totalSuccess);
      CHECK(f.driver.diagnostics().managedTransport.totalFailures == managed.totalFailures);
      CHECK(f.driver.diagnostics().managedTransport.consecutiveFailures == managed.consecutiveFailures);
      if (writing) {
        CHECK(f.registers[8] == 0x40 && f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
        CHECK(f.driver.diagnostics().hardwareEffect == (normalized == t::Err::OK
            ? t::HardwareEffect::PARTIAL : t::HardwareEffect::INDETERMINATE));
      }
    }
  }
}

void rawWriteEffectsAndProcedureDenial() {
  for (const auto* name : {"wreg", "rawwrite"}) {
    Fixture f;
    f.initialize();
    const auto managed = f.driver.diagnostics().managedTransport.totalSuccess;
    f.registers[9] = 0x14;
    f.command("duty 30"); // An ACK must preserve verified 50%, not staged 30%.
    f.write(9, 0x14);
    const auto ack = f.command(std::string(name) + " 9 20", 1);
    CHECK(ack.find("command=wreg code=OK ") != std::string::npos);
    CHECK(ack.find("source=write variant=tusb320lai address=9 value=20") != std::string::npos);
    CHECK(f.registers[9] == 0x04);
    CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::KNOWN);
    CHECK(f.driver.configurationSnapshot().verifiedProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_50);
    CHECK(f.cli.stagedProfile().drpDutyCycle == t::DrpDutyCycle::PERCENT_30);
    CHECK(f.driver.diagnostics().managedTransport.totalSuccess == managed);
  }
  for (const auto* line : {"wreg 10 0x20", "wreg 10 0x38", "wreg 10 0x31", "wreg 69 4", "wreg 9 16"}) {
    Fixture f;
    f.initialize();
    f.rejected(line, "UNSUPPORTED_FEATURE");
  }
  for (const auto* line : {"wreg 8 1", "wreg 8 192", "wreg 9 128", "wreg 10 0x34", "wreg 69 1", "wreg 69 8"}) {
    Fixture f;
    f.initialize();
    f.rejected(line);
  }
  for (const bool failed : {false, true}) {
    Fixture f;
    f.initialize();
    const auto managed = f.driver.diagnostics().managedTransport;
    f.write(8, 0x40);
    if (failed) f.script.back().status = t::Status::Error(t::Err::I2C_TIMEOUT, "write may be accepted", -2147483647);
    const auto text = f.command("wreg 8 64", 1);
    CHECK(text.find(failed ? "code=I2C_TIMEOUT detail=-2147483647" : "code=OK detail=0") != std::string::npos);
    CHECK(text.find("callbacks=1") != std::string::npos);
    CHECK(f.registers[8] == 0x40 && f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    CHECK(f.driver.diagnostics().configurationWriteUnresolved);
    CHECK(f.driver.diagnostics().hardwareEffect == (failed ? t::HardwareEffect::INDETERMINATE : t::HardwareEffect::PARTIAL));
    CHECK(f.driver.diagnostics().managedTransport.totalSuccess == managed.totalSuccess);
    CHECK(f.driver.diagnostics().managedTransport.totalFailures == managed.totalFailures);
    f.read(8);
    f.command("rawread 8", 1);
    CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    f.rejected("wreg 8 64", "CONFIGURATION_UNKNOWN");
    const auto before = f.driver.diagnostics();
    f.command("xfer_reset");
    CHECK(f.driver.transportStats().totalAttempts == 0 && f.driver.transportStats().totalFailures == 0);
    CHECK(f.driver.diagnostics().hardwareEffect == before.hardwareEffect && f.driver.diagnostics().configurationWriteUnresolved);
    CHECK(f.driver.diagnostics().managedTransport.totalSuccess == managed.totalSuccess);
    CHECK(f.driver.diagnostics().managedTransport.totalFailures == managed.totalFailures);
    CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  }
}

void parserAndOwnerExclusion() {
  Fixture f;
  f.initialize();
  for (const auto* line : {"reg", "reg -1", "reg +8", "reg 256", "reg 0x100", "reg 8 extra",
      "reg 0x", "reg 8junk", "decode 8", "decode +8 0", "decode 8 -1", "decode 8 256", "decode 8 0 extra",
      "wreg 8", "wreg 8 +1", "wreg 8 0x100", "wreg 8 0 extra", "irq invalid", "intpin extra",
      "invalidate", "invalidate invalid", "invalidate power-loss extra", "drv extra", "diag extra",
      "xfer_stats extra", "xfer_reset extra", "xfer_assert", "xfer_assert +6", "xfer_assert -1",
      "xfer_assert 0x6", "xfer_assert 4294967296", "xfer_assert 6 6", "xfer_assert 6 6 -1", "xfer_assert 6 6 0 extra"})
    f.rejected(line);
  f.read(8);
  t::OperationToken token;
  CHECK(f.driver.startReadProfile({f.now, f.now + 1000}, token).inProgress());
  for (const auto* line : {"reg 8", "wreg 8 64", "xfer_reset", "forget-evidence"}) f.rejected(line, "BUSY");
  for (const auto* query : {"drv", "diag", "xfer_stats", "irq", "decode 8 255"})
    CHECK(f.command(query).find("code=OK ") != std::string::npos);
  CHECK(f.driver.cancel(token).ok());
  for (const auto* line : {"reg 8", "wreg 8 64", "xfer_reset", "forget-evidence"}) f.rejected(line, "RESULT_PENDING");
  CHECK(f.driver.progress().resultPending);
  t::OperationResult result;
  CHECK(f.driver.takeResult(token, result).ok() && result.status.is(t::Err::CANCELLED));
  f.rejected("forget-evidence", "INVALID_CONFIG");
}

void healthAndManagedProbe() {
  Fixture f;
  f.initialize();
  for (unsigned streak = 1; streak <= 3; ++streak) {
    f.failure();
    const auto text = f.command("drv");
    CHECK(text.find(streak == 3 ? "state=OFFLINE" : "state=DEGRADED") != std::string::npos);
    CHECK(text.find("managed_streak=" + std::to_string(streak)) != std::string::npos);
  }
  const auto before = f.driver.diagnostics().managedTransport;
  f.read(8);
  f.command("reg 8", 1);
  CHECK(f.command("health").find("state=OFFLINE") != std::string::npos);
  CHECK(f.driver.diagnostics().managedTransport.consecutiveFailures == 3);
  const auto oldCalls = f.calls;
  f.command("xfer_reset");
  CHECK(f.calls == oldCalls && f.driver.transportStats().totalAttempts == 0);
  CHECK(f.driver.diagnostics().managedTransport.totalSuccess == before.totalSuccess);
  CHECK(f.driver.diagnostics().managedTransport.totalFailures == before.totalFailures);
  CHECK(f.command("state").find("state=OFFLINE") != std::string::npos);
  // The passive label cannot reject an explicit probe or perform an implicit retry.
  f.proof();
  CHECK(f.command("probe").find("command=init code=IN_PROGRESS") != std::string::npos);
  const auto token = f.driver.progress().token;
  f.finish(token, false);
  CHECK(f.driver.diagnostics().managedTransport.totalSuccess == before.totalSuccess + 6);
  CHECK(f.driver.diagnostics().managedTransport.consecutiveFailures == 0);
  CHECK(f.command("drv").find("state=READY") != std::string::npos);
  CHECK(f.driver.progress().resultPending);
  const auto diag = f.command("diag");
  CHECK(diag.find("source=managed_callbacks initialized=1") != std::string::npos);
  CHECK(diag.find("code=I2C_TIMEOUT detail=-123456789 time_ms=0 time_valid=0") != std::string::npos);
}

void automaticTrafficIsSeparate() {
  Fixture f;
  f.command("bind");
  f.read(9); f.read(8); f.read(9);
  f.command("read");
  const auto token = f.driver.progress().token;
  CHECK(f.cli.processCommand("job auto 1"));
  CHECK(f.calls == 0);
  f.tick();
  CHECK(f.calls == 1);
  CHECK(f.cli.processCommand("diag"));
  CHECK(f.calls == 1); // The cached handler does no callbacks; the owner turn still advances work.
  f.tick();
  CHECK(f.calls == 2);
  CHECK(f.cli.processCommand("rawread 8"));
  CHECK(f.calls == 2); // Rejected raw diagnostics cannot bypass the active operation.
  f.tick();
  CHECK(f.calls == 3);
  for (unsigned turn = 0; turn < 64; ++turn) f.tick();
  CHECK(f.driver.progress().resultPending && f.driver.progress().token.value == token.value);
  CHECK(f.output.find("command=diag outcome=SUCCEEDED code=OK detail=0 token=0 callbacks=0") != std::string::npos);
  CHECK(f.output.find("command=reg outcome=FAILED code=BUSY detail=0 token=0 callbacks=0") != std::string::npos);
  CHECK(f.driver.transportStats().totalAttempts == 3 && f.driver.diagnostics().managedTransport.totalSuccess == 3);
}

void cachedInterruptHistory() {
  Fixture f;
  f.initialize();
  f.registers[9] = 0x14;
  f.read(9); f.write(9, 0x14);
  t::OperationToken token;
  CHECK(f.driver.startAcknowledgeInterrupt({100, 1100}, token).inProgress());
  f.finish(token);
  CHECK(f.registers[9] == 0x04);
  const auto attempts = f.driver.transportStats().totalAttempts;
  const auto text = f.command("irq");
  CHECK(text.find("source=managed_operation observed=1 pending_before=1 ack_attempted=1 ack_confirmed=1") != std::string::npos);
  CHECK(text.find("pending_after=unknown pending_after_valid=0") != std::string::npos);
  CHECK(f.driver.transportStats().totalAttempts == attempts);
  f.command("end");
  CHECK(f.command("irq").find("observed=1 pending_before=1 ack_attempted=1 ack_confirmed=1") != std::string::npos);
  CHECK(f.driver.transportStats().totalAttempts == attempts);
}

void configurationMismatchDetails() {
  Fixture f;
  auto text = f.command("cfg");
  CHECK(text.find("observed_ms=0 observed_time_valid=0") != std::string::npos);
  CHECK(text.find("mismatch_valid=0 mismatch_register=unknown mismatch_expected=unknown mismatch_observed=unknown mismatch_mask=unknown") != std::string::npos);
  f.initialize();
  auto desired = f.driver.configurationSnapshot().verifiedProfile;
  desired.sourceCurrent = t::SourceCurrent::MA_1500;
  t::OperationToken apply;
  CHECK(f.driver.startApplyProfile(desired, {100, 1100}, apply).inProgress());
  CHECK(f.driver.cancel(apply).ok()); // Retain an explicit desired profile without changing hardware.
  t::OperationResult result;
  CHECK(f.driver.takeResult(apply, result).ok() && result.hardwareEffect == t::HardwareEffect::NONE);
  f.registers[8] = 0x80; // Independent external advertisement: 3 A, versus the desired 1.5 A.
  f.now = 200;
  f.proof();
  t::OperationToken verify;
  CHECK(f.driver.startVerifyProfile({200, 1200}, verify).inProgress());
  f.finish(verify, false);
  CHECK(f.driver.resultSnapshot(verify, result).ok() && result.status.is(t::Err::CONFIGURATION_MISMATCH));
  const auto snapshot = f.driver.configurationSnapshot();
  CHECK(snapshot.observedUptimeMs == 200 && snapshot.observedTimeValid);
  CHECK(snapshot.mismatchValid && snapshot.mismatchRegister == 8);
  CHECK(snapshot.mismatchExpected == 64 && snapshot.mismatchObserved == 128 && snapshot.mismatchMask == 192);
  CHECK(snapshot.desiredProfile.sourceCurrent == t::SourceCurrent::MA_1500);
  CHECK(snapshot.observedProfile.sourceCurrent == t::SourceCurrent::MA_3000);
  for (const auto* query : {"cfg", "settings", "profile", "profile show"}) {
    text = f.command(query);
    CHECK(text.find("TUSB CONFIG_DETAIL schema=1 ") != std::string::npos);
    CHECK(text.find("observed_ms=200 observed_time_valid=1 termination=ENABLED reset=NONE") != std::string::npos);
    CHECK(text.find("mismatch_valid=1 mismatch_register=8 mismatch_expected=64 mismatch_observed=128 mismatch_mask=192") != std::string::npos);
    CHECK(f.driver.progress().resultPending);
  }
  CHECK(f.driver.resultSnapshot(verify, result).ok() && result.status.is(t::Err::CONFIGURATION_MISMATCH));
}

void diagnosticAppearanceParity() {
  for (const bool verbose : {false, true}) {
    Fixture f;
    f.initialize();
    f.command("color off");
    f.command(verbose ? "verbose 1" : "verbose 0");
    for (const auto* query : {"drv", "diag", "irq", "xfer_stats", "decode 8 0xC2", "cfg"}) {
      const auto text = f.command(query);
      CHECK(text.find('\033') == std::string::npos);
      CHECK(text.find("code=OK detail=0") != std::string::npos);
      CHECK((text.find("[OK] OK detail=0") != std::string::npos) == verbose);
      CHECK(text.find("outcome=SUCCEEDED") != std::string::npos);
      if (std::strcmp(query, "decode 8 0xC2") == 0) {
        CHECK(text.find("current=unknown") != std::string::npos);
        CHECK(text.find("accessory=UNKNOWN") != std::string::npos);
        CHECK(text.find("profile_valid_fields=0 status_valid_fields=144") != std::string::npos);
      }
    }
    f.read(8);
    auto text = f.command("reg 8", 1);
    CHECK(text.find('\033') == std::string::npos && text.find("code=OK detail=0") != std::string::npos);
    CHECK(text.find("source=read variant=tusb320lai address=8 value=0") != std::string::npos);
    CHECK((text.find("[OK] OK detail=0") != std::string::npos) == verbose);
    f.read(8);
    f.script.back().status = t::Status::Error(t::Err::I2C_BUS, "visible raw error", -2147483647);
    text = f.command("rawread 8", 1);
    CHECK(text.find('\033') == std::string::npos);
    CHECK(text.find("[E] I2C_BUS detail=-2147483647 visible raw error") != std::string::npos);
    CHECK(text.find("code=I2C_BUS detail=-2147483647") != std::string::npos);
    CHECK(text.find("callbacks=1") != std::string::npos && text.find("TUSB REG ") == std::string::npos);
    const auto before = f.evidence();
    text = f.command("reg -1");
    CHECK(text.find('\033') == std::string::npos && text.find("[E] INVALID_PARAM detail=0") != std::string::npos);
    CHECK(text.find("outcome=FAILED code=INVALID_PARAM") != std::string::npos && before == f.evidence());
    for (const bool failed : {false, true}) {
      Fixture write;
      write.initialize();
      write.command("color off");
      write.command(verbose ? "verbose 1" : "verbose 0");
      write.write(8, 0x40);
      if (failed) write.script.back().status = t::Status::Error(t::Err::I2C_TIMEOUT, "visible write error", -99);
      text = write.command("wreg 8 64", 1);
      CHECK(text.find('\033') == std::string::npos && text.find("callbacks=1") != std::string::npos);
      CHECK(text.find(failed ? "code=I2C_TIMEOUT detail=-99" : "code=OK detail=0") != std::string::npos);
      if (failed) CHECK(text.find("[E] I2C_TIMEOUT detail=-99 visible write error") != std::string::npos);
      else CHECK((text.find("[OK] OK detail=0") != std::string::npos) == verbose);
      CHECK(write.registers[8] == 0x40 && write.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    }
  }
}

void failedRawWriteUnderBackpressure() {
  Fixture f;
  f.initialize();
  f.writeLimit = 0;
  f.output.clear();
  for (unsigned turn = 0; turn < 100; ++turn) {
    CHECK(f.cli.processCommand("version"));
    f.tick();
  }
  CHECK(f.output.empty() && f.cli.console().droppedRecords() != 0);
  const auto lossesBeforeWrite = f.cli.console().droppedRecords();
  const auto before = f.driver.transportStats();
  const auto managed = f.driver.diagnostics().managedTransport;
  f.write(8, 0x40);
  f.script.back().status = t::Status::Error(t::Err::I2C_TIMEOUT, "accepted write with lost response",
      std::numeric_limits<int32_t>::min());
  const auto callbacks = f.calls;
  CHECK(f.cli.processCommand("wreg 8 64"));
  CHECK(f.calls == callbacks + 1);
  f.tick();
  CHECK(f.calls == callbacks + 1 && f.registers[8] == 0x40);
  CHECK(f.cli.console().droppedRecords() > lossesBeforeWrite);
  CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  CHECK(f.driver.diagnostics().hardwareEffect == t::HardwareEffect::INDETERMINATE);
  CHECK(f.driver.diagnostics().configurationWriteUnresolved && !f.driver.progress().resultPending);
  const auto after = f.driver.transportStats();
  CHECK(after.totalAttempts == before.totalAttempts + 1 && after.totalFailures == before.totalFailures + 1);
  CHECK(after.totalSuccess == before.totalSuccess);
  CHECK(after.lastStatus.is(t::Err::I2C_TIMEOUT) && after.lastStatus.detail == std::numeric_limits<int32_t>::min());
  const auto filtered = f.driver.diagnostics().managedTransport;
  CHECK(filtered.initialized == managed.initialized && filtered.totalSuccess == managed.totalSuccess);
  CHECK(filtered.totalFailures == managed.totalFailures && filtered.consecutiveFailures == managed.consecutiveFailures);
  CHECK(filtered.lastStatus.code == managed.lastStatus.code && filtered.lastStatus.detail == managed.lastStatus.detail);
  CHECK(filtered.lastSuccessUptimeMs == managed.lastSuccessUptimeMs && filtered.lastSuccessTimeValid == managed.lastSuccessTimeValid);
  CHECK(filtered.lastFailureUptimeMs == managed.lastFailureUptimeMs && filtered.lastFailureTimeValid == managed.lastFailureTimeValid);
  const auto lostRecords = f.cli.console().droppedRecords();
  const auto lostBytes = f.cli.console().droppedBytes();
  for (unsigned turn = 0; turn < 100; ++turn) {
    f.tick();
    CHECK(f.calls == callbacks + 1);
    CHECK(f.cli.console().droppedRecords() == lostRecords && f.cli.console().droppedBytes() == lostBytes);
  }
  CHECK(f.output.empty());
  f.writeLimit = 256;
  for (unsigned turn = 0; turn < 128 && f.cli.console().queued() != 0; ++turn) f.tick();
  CHECK(f.cli.console().queued() == 0 && !f.cli.console().lossPending());
  CHECK(f.output.find("TUSB LOSS schema=1 dropped_records=") != std::string::npos);
  CHECK(f.calls == callbacks + 1 && !f.driver.progress().resultPending);
  f.output.clear();
  CHECK(f.cli.processCommand("diag"));
  f.tick();
  for (unsigned turn = 0; turn < 128 && f.cli.console().queued() != 0; ++turn) f.tick();
  CHECK(f.cli.console().queued() == 0 && f.calls == callbacks + 1);
  CHECK(f.output.find("configuration=UNKNOWN effect=INDETERMINATE") != std::string::npos);
  CHECK(f.output.find("config_write_unresolved=1") != std::string::npos);
  CHECK(f.output.find("code=I2C_TIMEOUT detail=-2147483648 time_ms=0 time_valid=0") != std::string::npos);
  CHECK(f.output.find("last_code=I2C_TIMEOUT last_detail=-2147483648") != std::string::npos);
  CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  CHECK(f.driver.diagnostics().hardwareEffect == t::HardwareEffect::INDETERMINATE);
  CHECK(f.driver.diagnostics().managedTransport.totalSuccess == managed.totalSuccess);
  CHECK(f.driver.diagnostics().managedTransport.totalFailures == managed.totalFailures);
  CHECK(f.driver.diagnostics().configurationWriteUnresolved && !f.driver.progress().resultPending);
}

void pinAndInvalidationEvidence() {
  Fixture unavailable;
  auto text = unavailable.command("intpin");
  CHECK(text.find("code=UNSUPPORTED_FEATURE ") != std::string::npos);
  CHECK(text.find("source=application available=0 value=unknown") != std::string::npos);
  Fixture pin(t::Variant::TUSB320LAI, true);
  for (const bool value : {false, true}) {
    pin.pinValue = value;
    text = pin.command("intpin");
    CHECK(text.find("code=OK ") != std::string::npos);
    CHECK(text.find(value ? "available=1 value=1" : "available=1 value=0") != std::string::npos);
  }
  pin.pinAvailable = false;
  text = pin.command("intpin");
  CHECK(text.find("code=INVALID_CONFIG ") != std::string::npos);
  CHECK(text.find("available=0 value=unknown") != std::string::npos);
  CHECK(pin.pinReads == 3 && pin.calls == 0);
  Fixture historical;
  historical.initialize();
  historical.registers[9] = 0xB4;
  historical.registers[8] = 0x11;
  historical.read(9); historical.read(8); historical.read(9);
  t::OperationToken refresh;
  CHECK(historical.driver.startRefreshPort({100, 1100}, refresh).inProgress());
  historical.finish(refresh);
  const auto port = historical.driver.portSnapshot();
  CHECK(!port.invalidated && port.rawValidMask != 0 && port.observedTimeValid);
  historical.command("invalidate external-reset");
  const auto invalidated = historical.driver.portSnapshot();
  CHECK(invalidated.invalidated && invalidated.rawStatus08 == port.rawStatus08 && invalidated.rawStatus09 == port.rawStatus09);
  CHECK(invalidated.rawValidMask == port.rawValidMask && invalidated.validFields == port.validFields);
  CHECK(invalidated.sequence == port.sequence && invalidated.observedUptimeMs == port.observedUptimeMs);
  CHECK(invalidated.observedTimeValid == port.observedTimeValid);
  for (const auto* reason : {"external-reset", "power-loss", "target-change"}) {
    Fixture f;
    f.initialize();
    f.write(8, 0x40);
    f.script.back().status = t::Status::Error(t::Err::I2C_TIMEOUT, "possible write", -41);
    f.command("wreg 8 64", 1);
    const auto effect = f.driver.diagnostics().hardwareEffect;
    text = f.command(std::string("invalidate ") + reason);
    CHECK(text.find(std::string("reason=") + reason) != std::string::npos);
    CHECK(text.find("command=invalidate code=OK ") != std::string::npos);
    CHECK(!f.driver.diagnostics().initialized && f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    CHECK(f.driver.diagnostics().hardwareEffect == effect && f.driver.diagnostics().configurationWriteUnresolved);
    CHECK(f.command("end").find("callbacks=0") != std::string::npos);
    f.rejected("addr 0x67", "CONFIGURATION_UNKNOWN");
    CHECK(f.driver.diagnostics().hardwareEffect == effect);
    CHECK(f.command("forget-evidence").find("code=OK ") != std::string::npos);
    CHECK(f.driver.diagnostics().hardwareEffect == t::HardwareEffect::NONE);
    CHECK(f.driver.transportStats().totalAttempts == 7);
    CHECK(f.command("addr 0x67").find("code=OK ") != std::string::npos);
  }
}
} // namespace

/// @brief Execute diagnostic behavior in the registered CLI suite; return the failure count.
int runCliDiagnosticsTests() {
  cachedRoutesAndHelp();
  rawReadAliasesAndFailures();
  pureDecodeAndVariantGates();
  everyRawCallbackStatus();
  rawWriteEffectsAndProcedureDenial();
  parserAndOwnerExclusion();
  healthAndManagedProbe();
  automaticTrafficIsSeparate();
  cachedInterruptHistory();
  configurationMismatchDetails();
  diagnosticAppearanceParity();
  failedRawWriteUnderBackpressure();
  pinAndInvalidationEvidence();
  std::printf("CLI diagnostics: %u checks, %u failures\n", checks, failures);
  return static_cast<int>(failures);
}
