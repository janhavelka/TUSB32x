/// @file test_cli.cpp
/// @brief Executed local CLI routing, atomic staging, evidence gates and bounded framing tests.
#include "Tusb32xCli.h"
#include "../support/FakeTusb32xDevice.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace tusb32x_cli {
/// @brief Boundary-state access restricted to executable tests, without production commands.
struct CliTestAccess {
  static void requestBase(Cli& cli, uint64_t value) { cli._lastRequest = value; }
  static void stageGeneration(Cli& cli, uint32_t value) { cli._stagedGeneration = value; }
  static void maximumRecord(Cli& cli) {
    Cli::Request request;
    request.id = std::numeric_limits<uint64_t>::max();
    std::memcpy(request.nonce, "01234567890123456789012345678901", 33);
    request.command = "profile.show";
    cli.profileOutput(request);
    cli.record(request, "ACK", "command=profile.validate code=CONFIGURATION_UNKNOWN detail=-2147483648 action=UNCHANGED token=18446744073709551615 staged_generation=4294967295 desired_generation=4294967295 applied_generation=4294967295 configuration=UNCONFIGURED");
  }
  static void maximumPort(Cli& cli, const TUSB32x::PortSnapshot& port, uint64_t nowMs) {
    Cli::Request request;
    request.id = std::numeric_limits<uint64_t>::max();
    std::memcpy(request.nonce, "01234567890123456789012345678901", 33);
    request.command = "sample";
    cli.portOutput(request, port, nowMs);
  }
  static void maximumResult(Cli& cli, const TUSB32x::OperationResult& result) {
    Cli::Request request;
    request.id = std::numeric_limits<uint64_t>::max();
    std::memcpy(request.nonce, "01234567890123456789012345678901", 33);
    request.command = "recover-dead-battery";
    cli._operationRequest = request;
    cli._operationToken = result.token;
    cli.resultOutput(request, result);
    cli.close(request, result.status, result.token, result.transfers, result.state);
  }
  static TUSB32x::Status assertCounters(const TUSB32x::TransportStats& stats,
      uint32_t attempts, uint32_t success, uint32_t failures, bool all) {
    return Cli::assertCounters(stats, attempts, success, failures, all);
  }
  static void maximumDiagnostics(Cli& cli, const TUSB32x::DriverDiagnostics& diagnostics,
      const TUSB32x::ConfigurationSnapshot& configuration, const TUSB32x::TransportStats& stats,
      const TUSB32x::HealthSnapshot& health) {
    Cli::Request request;
    request.id = std::numeric_limits<uint64_t>::max();
    std::memcpy(request.nonce, "01234567890123456789012345678901", 33);
    request.command = "diag";
    cli.configurationOutput(request, configuration);
    cli.diagnosticsOutput(request, diagnostics, stats);
    cli.healthOutput(request, health, diagnostics);
    Cli::Response response;
    response.asserting = response.assertAll = true;
    response.expectedAttempts = response.expectedSuccess = response.expectedFailures =
        std::numeric_limits<uint32_t>::max();
    response.assertion = "SATURATED";
    cli.transportOutput(request, response, stats);
    response.reg = 8; response.value = 255;
    response.decoded.variant = TUSB32x::Variant::TUSB320LAI;
    response.decoded.reg = 8; response.decoded.value = 255;
    response.decoded.statusValidFields = 511;
    response.decoded.detectedCurrent = TUSB32x::DetectedCurrent::CHARGE_THROUGH_500_MA;
    response.decoded.accessory = TUSB32x::Accessory::DEBUG_LOCAL_DFP;
    response.decoded.orientation = TUSB32x::Orientation::NOT_APPLICABLE;
    response.decoded.attachment = TUSB32x::Attachment::LOCAL_SOURCE;
    cli.registerOutput(request, response);
  }
};
}  // namespace tusb32x_cli

namespace {
namespace t = TUSB32x;
using Cli = tusb32x_cli::Cli;
using Fake = tusb32x_test::FakeTusb32xDevice;
unsigned failures = 0;
unsigned checks = 0;
void check(bool value, const char* expression, int line) {
  ++checks;
  if (!value) { ++failures; std::printf("FAIL CLI line %d: %s\n", line, expression); }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

bool sameProfile(const t::DeviceProfile& a, const t::DeviceProfile& b) {
  return a.portMode == b.portMode && a.sourceCurrent == b.sourceCurrent &&
      a.sourcePreference == b.sourcePreference && a.drpDutyCycle == b.drpDutyCycle &&
      a.ccDebounce == b.ccDebounce && a.disableUfpAccessory == b.disableUfpAccessory &&
      a.disableTerm == b.disableTerm && a.disableRdRp == b.disableRdRp;
}
bool sameConfiguration(const t::ConfigurationSnapshot& a, const t::ConfigurationSnapshot& b) {
  return a.hasDesiredProfile == b.hasDesiredProfile && a.hasObservedProfile == b.hasObservedProfile &&
      a.hasVerifiedProfile == b.hasVerifiedProfile && a.desiredGeneration == b.desiredGeneration &&
      a.verifiedGeneration == b.verifiedGeneration && a.state == b.state &&
      sameProfile(a.desiredProfile, b.desiredProfile) &&
      sameProfile(a.observedProfile, b.observedProfile) &&
      sameProfile(a.verifiedProfile, b.verifiedProfile) &&
      a.rawConfig08 == b.rawConfig08 && a.rawConfig09 == b.rawConfig09 &&
      a.rawConfig0A == b.rawConfig0A && a.rawConfig45 == b.rawConfig45 &&
      a.rawConfigValidMask == b.rawConfigValidMask && a.observedValidFields == b.observedValidFields &&
      a.observedUptimeMs == b.observedUptimeMs && a.observedTimeValid == b.observedTimeValid &&
      a.terminationState == b.terminationState && a.resetState == b.resetState &&
      a.mismatchValid == b.mismatchValid && a.mismatchRegister == b.mismatchRegister &&
      a.mismatchExpected == b.mismatchExpected && a.mismatchObserved == b.mismatchObserved &&
      a.mismatchMask == b.mismatchMask;
}

struct Fixture {
  Fake bus;
  t::TUSB32x driver;
  t::DriverConfig config;
  tusb32x_cli::Platform platform;
  std::string output;
  std::size_t writeLimit = 256;
  std::size_t writes = 0;
  std::size_t maxOffered = 0;
  bool denyTarget = false;
  const char* targetErrorMessage = "fixture borrowed address mismatch";
  int32_t targetErrorDetail = 0;
  Cli cli;

  Fixture() : config(makeConfig(&bus)), platform(makePlatform(this)), cli(driver, config, platform) {
    CHECK(cli.setup().ok());
    CHECK(!driver.diagnostics().bound && bus.calls == 0 && output.empty());
  }
  static t::DriverConfig makeConfig(Fake* bus) {
    t::DriverConfig c;
    c.variant = t::Variant::TUSB320LAI;
    c.i2cWrite = Fake::write;
    c.i2cWriteRead = Fake::writeRead;
    c.i2cUser = bus;
    return c;
  }
  static uint64_t now(void*) { return 100; }
  static std::size_t write(const char* bytes, std::size_t length, void* user) {
    auto& fixture = *static_cast<Fixture*>(user);
    ++fixture.writes;
    fixture.maxOffered = std::max(fixture.maxOffered, length);
    const auto accepted = std::min(length, fixture.writeLimit);
    fixture.output.append(bytes, accepted);
    return accepted;
  }
  static tusb32x_cli::AdapterCounters counters(void*) { return {true, 0, 0}; }
  static tusb32x_cli::AdapterCounters maximumCounters(void*) {
    return {true, std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()};
  }
  static t::Status validate(const t::DriverConfig&, void* user) {
    const auto& fixture = *static_cast<Fixture*>(user);
    return fixture.denyTarget
        ? t::Status::Error(t::Err::INVALID_CONFIG, fixture.targetErrorMessage, fixture.targetErrorDetail)
        : t::Status::Ok();
  }
  static tusb32x_cli::Platform makePlatform(Fixture* fixture) {
    tusb32x_cli::Platform p;
    p.nowMs = now; p.write = write; p.counters = counters;
    p.validateTarget = validate; p.user = fixture;
    p.identity.build = "host-fixture"; p.identity.framework = "host";
    return p;
  }
  void flush() {
    for (unsigned turn = 0; turn < 512; ++turn) {
      const auto before = writes;
      cli.tick();
      CHECK(writes - before <= 1 && maxOffered <= 256);
    }
    if (writeLimit != 0) CHECK(cli.console().queued() == 0);
  }
  std::string command(const char* command) {
    output.clear();
    const auto callbacks = bus.calls;
    CHECK(cli.processCommand(command));
    flush();
    CHECK(bus.calls == callbacks);
    return output;
  }
  void rejected(const char* command, const char* code = nullptr, const char* canonical = "invalid") {
    const auto staged = cli.stagedProfile();
    const auto generation = cli.stagedGeneration();
    const auto target = cli.targetConfig();
    const auto configuration = driver.configurationSnapshot();
    const auto progress = driver.progress();
    const auto diagnostic = driver.diagnostics();
    const auto statistics = driver.transportStats();
    const auto port = driver.portSnapshot();
    t::OperationResult oldResult;
    const bool retained = driver.resultSnapshot(progress.token, oldResult).ok();
    const auto text = this->command(command);
    CHECK(sameProfile(staged, cli.stagedProfile()) && generation == cli.stagedGeneration());
    CHECK(target.variant == cli.targetConfig().variant && target.address == cli.targetConfig().address);
    CHECK(sameConfiguration(configuration, driver.configurationSnapshot()));
    CHECK(progress.token.value == driver.progress().token.value &&
        progress.state == driver.progress().state && progress.resultPending == driver.progress().resultPending);
    CHECK(diagnostic.bound == driver.diagnostics().bound &&
        diagnostic.hardwareEffect == driver.diagnostics().hardwareEffect &&
        diagnostic.configurationWriteUnresolved == driver.diagnostics().configurationWriteUnresolved &&
        diagnostic.modeTransitionUnresolved == driver.diagnostics().modeTransitionUnresolved &&
        diagnostic.resetAcceptanceUncertain == driver.diagnostics().resetAcceptanceUncertain);
    CHECK(statistics.totalAttempts == driver.transportStats().totalAttempts &&
        statistics.totalFailures == driver.transportStats().totalFailures &&
        statistics.totalSuccess == driver.transportStats().totalSuccess);
    CHECK(port.sequence == driver.portSnapshot().sequence && port.invalidated == driver.portSnapshot().invalidated &&
        port.validFields == driver.portSnapshot().validFields && port.rawValidMask == driver.portSnapshot().rawValidMask);
    if (retained) {
      t::OperationResult result;
      CHECK(driver.resultSnapshot(progress.token, result).ok());
      CHECK(result.token.value == oldResult.token.value && result.status.code == oldResult.status.code &&
          result.status.detail == oldResult.status.detail && result.hardwareEffect == oldResult.hardwareEffect &&
          sameConfiguration(result.configuration, oldResult.configuration));
    }
    CHECK(text.find(std::string("command=") + canonical + " ") != std::string::npos);
    if (code) CHECK(text.find(std::string("code=") + code + " ") != std::string::npos);
    CHECK(text.find("action=UNCHANGED ") != std::string::npos);
    CHECK(text.find("outcome=FAILED") != std::string::npos);
  }
  t::OperationToken initialize(bool take = true) {
    const auto selected = cli.targetConfig();
    const auto address = static_cast<uint8_t>(selected.address);
    const bool base = selected.variant == t::Variant::TUSB320 || selected.variant == t::Variant::TUSB320I;
    CHECK(driver.bind(selected).ok());
    const uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    auto& id = bus.expectRead(address, 0, 8);
    std::memcpy(id.rx, identity, 8);
    if (!base) bus.expectRead(address, 0xA0, 1).rx[0] = 2;
    bus.expectRead(address, 0x08, 1).rx[0] = 0;
    bus.expectRead(address, 0x09, 1).rx[0] = 0;
    bus.expectRead(address, 0x0A, 1).rx[0] = 0x30;
    bus.expectRead(address, 0x45, 1).rx[0] = 0;
    t::OperationToken token;
    CHECK(driver.startInitialize({100, 1000}, token).inProgress());
    for (unsigned turn = 0; turn < 12 && driver.progress().state == t::OperationState::ACTIVE; ++turn)
      driver.poll(token, 100);
    CHECK(driver.progress().state == t::OperationState::SUCCEEDED && bus.complete());
    if (take) { t::OperationResult result; CHECK(driver.takeResult(token, result).ok()); }
    return token;
  }
};

void routesAndAliases() {
  Fixture f;
  const char* queries[] = {"version", "ver", "color", "verbose", "bus", "console", "capabilities",
      "variant", "model", "addr", "sample", "snapshot", "freshness", "cfg", "settings", "profile",
      "profile show", "profile validate", "mode", "current", "duty", "debounce", "preference",
      "ufpaccessory", "termination", "rd-rp", "profile reset", "end", "unbind", "forget-evidence"};
  const char* canonical[] = {"version", "version", "color", "verbose", "bus", "console", "capabilities",
      "variant", "variant", "addr", "sample", "sample", "freshness", "profile.show", "profile.show",
      "profile.show", "profile.show", "profile.validate", "mode", "current", "duty", "debounce",
      "preference", "ufpaccessory", "termination", "rd-rp", "profile.reset", "end", "end", "forget-evidence"};
  static_assert(sizeof(queries) == sizeof(canonical));
  for (std::size_t index = 0; index < sizeof(queries) / sizeof(*queries); ++index) {
    const auto text = f.command(queries[index]);
    CHECK(text.find(std::string("command=") + canonical[index] + " ") != std::string::npos);
    CHECK(text.find("code=OK ") != std::string::npos && text.find("callbacks=0") != std::string::npos);
  }
  const auto bound = f.command("bind");
  CHECK(bound.find("code=OK ") != std::string::npos && f.driver.diagnostics().bound);
  f.command("unbind");
  f.rejected("profile discard", "CONFIGURATION_UNKNOWN", "profile.discard");
  std::size_t count = 0;
  const auto* rows = tusb32x_cli::commandRows(count);
  CHECK(count == 55);
  for (std::size_t index = 0; index < count; ++index) {
    const bool managed = std::strcmp(rows[index].execution, "CORE_JOB") == 0;
    const bool raw = std::strcmp(rows[index].execution, "RAW_I2C") == 0;
    const bool rawRead = std::strcmp(rows[index].command, "reg") == 0;
    CHECK(rows[index].maxCallbacks == (managed || raw ? 1 : 0) &&
        rows[index].txBytes == (managed ? 2 : raw ? rawRead ? 1 : 2 : 0) &&
        rows[index].rxBytes == (managed ? 8 : rawRead ? 1 : 0));
    const auto help = f.command((std::string("help ") + rows[index].command).c_str());
    CHECK(help.find("outcome=SUCCEEDED") != std::string::npos);
  }
  for (const auto* topic : {"all", "common", "connection", "port", "profile", "lifecycle",
      "interrupt", "job", "registers", "diagnostics", "?", "ver", "model", "settings", "snapshot", "unbind",
      "initialize", "probe", "status", "selfcheck", "selftest", "stop", "job.cancel", "job.result"}) {
    const auto help = f.command((std::string("? ") + topic).c_str());
    CHECK(help.find("outcome=SUCCEEDED") != std::string::npos);
  }
  f.command("color off");
  auto text = f.command("help");
  CHECK(text.find('\x1b') == std::string::npos);
  CHECK(text.find("init ") != std::string::npos && text.find("irq ack") != std::string::npos &&
      text.find("wreg ") != std::string::npos && text.find("job step") != std::string::npos);
  f.command("color on");
  text = f.command("help");
  CHECK(text.find("\x1b[36m") != std::string::npos && text.find("\x1b[32m") != std::string::npos);
  CHECK(f.bus.calls == 0);
}

void setupAndAppearanceContracts() {
  Fixture owner;
  Cli unprepared(owner.driver, owner.config, owner.platform);
  CHECK(!unprepared.processCommand("bind") && unprepared.feed("bind\n", 5) == 0);
  unprepared.tick();
  CHECK(owner.output.empty() && owner.bus.calls == 0);
  auto missingClock = owner.platform;
  missingClock.nowMs = nullptr;
  Cli noClock(owner.driver, owner.config, missingClock);
  CHECK(noClock.setup().is(t::Err::INVALID_CONFIG));
  auto missingOutput = owner.platform;
  missingOutput.write = nullptr;
  Cli noOutput(owner.driver, owner.config, missingOutput);
  CHECK(noOutput.setup().is(t::Err::INVALID_CONFIG));
  auto invalidIdentity = owner.platform;
  invalidIdentity.identity.git = "contains space";
  Cli badIdentity(owner.driver, owner.config, invalidIdentity);
  CHECK(badIdentity.setup().is(t::Err::INVALID_CONFIG));
  auto invalidConfig = owner.config;
  invalidConfig.variant = t::Variant::UNSPECIFIED;
  Cli noPart(owner.driver, invalidConfig, owner.platform);
  CHECK(noPart.setup().is(t::Err::INVALID_CONFIG));
  CHECK(!owner.driver.diagnostics().bound && owner.bus.calls == 0);

  owner.command("color 0");
  const auto help = owner.command("help all");
  std::size_t count = 0;
  const auto* rows = tusb32x_cli::commandRows(count);
  for (std::size_t index = 0; index < count; ++index) {
    const auto start = help.find(std::string(rows[index].syntax));
    CHECK(start != std::string::npos);
    if (start == std::string::npos) continue;
    const auto delimiter = help.find(" - ", start);
    CHECK(delimiter != std::string::npos && delimiter - start >= 32);
    CHECK(help.find("[stage]", start) != std::string::npos ||
        help.find("[cached]", start) != std::string::npos);
  }
}

void stagingAndAtomicTargets() {
  Fixture f;
  const auto original = f.driver.configurationSnapshot();
  const char* setters[] = {"mode port", "mode dfp", "mode ufp", "mode drp", "current 1a5", "current 3a0",
      "current default", "duty 30", "duty 40", "duty 50", "duty 60", "debounce 118", "debounce 134",
      "debounce 152", "debounce 168", "preference try-sink", "preference try-source", "preference normal",
      "ufpaccessory off", "ufpaccessory on", "termination off", "termination on", "rd-rp off", "rd-rp on",
      "color 0", "color 1", "verbose 1", "verbose 0"};
  for (const auto* setter : setters) {
    const auto result = f.command(setter);
    CHECK(result.find("outcome=SUCCEEDED") != std::string::npos);
    CHECK(sameConfiguration(original, f.driver.configurationSnapshot()));
  }
  f.command("ufpaccessory off"); CHECK(f.cli.stagedProfile().disableUfpAccessory);
  f.command("termination off"); CHECK(f.cli.stagedProfile().disableTerm);
  f.command("rd-rp off"); CHECK(f.cli.stagedProfile().disableRdRp);
  f.command("profile reset"); CHECK(sameProfile(f.cli.stagedProfile(), t::DeviceProfile{}));
  f.command("debounce 118");
  f.rejected("variant tusb320", "INVALID_CONFIG", "variant");
  f.rejected("variant tusb320 0x47", "INVALID_CONFIG", "variant");
  f.denyTarget = true;
  f.rejected("variant tusb320 0x60", "INVALID_CONFIG", "variant");
  f.denyTarget = false;
  CHECK(f.command("model tusb320 0x60").find("code=OK ") != std::string::npos);
  CHECK(f.cli.targetConfig().variant == t::Variant::TUSB320 &&
      f.cli.targetConfig().address == t::DeviceAddress::ADDR_LOW_0X60);
  CHECK(sameProfile(f.cli.stagedProfile(), t::DeviceProfile{}));
  f.command("debounce 133");
  const auto staged = f.cli.stagedProfile();
  const auto generation = f.cli.stagedGeneration();
  f.command("addr 97");
  CHECK(sameProfile(staged, f.cli.stagedProfile()) && f.cli.stagedGeneration() == generation);
  for (const auto* control : {"preference", "preference normal", "ufpaccessory", "ufpaccessory on", "termination", "termination on"}) {
    const std::string command(control);
    const auto canonical = command.substr(0, command.find(' '));
    f.rejected(control, "UNSUPPORTED_FEATURE", canonical.c_str());
  }
  f.command("variant tusb320lai 0x47");
  CHECK(f.cli.stagedProfile().ccDebounce == t::CcDebounce::MS_168);
  f.rejected("debounce 133", "UNSUPPORTED_FEATURE", "debounce");
  for (const auto* part : {"tusb320li", "tusb320hi", "tusb320lai", "tusb320hai", "tusb322i"}) {
    CHECK(f.command((std::string("variant ") + part + " 0x47").c_str()).find("code=OK ") != std::string::npos);
  }
  for (const auto* command : {"current 3a0", "duty 30", "ufpaccessory off"})
    CHECK(f.command(command).find("code=OK ") != std::string::npos);
  f.rejected("mode port", "UNSUPPORTED_FEATURE", "mode");
  f.command("variant tusb320i 0x60");
  CHECK(f.cli.targetConfig().variant == t::Variant::TUSB320I);
  tusb32x_cli::CliTestAccess::stageGeneration(f.cli, std::numeric_limits<uint32_t>::max() - 1);
  f.command("current 1a5"); f.command("current 3a0");
  CHECK(f.cli.stagedGeneration() == std::numeric_limits<uint32_t>::max());
}

void malformedAndPoisonedInput() {
  Fixture f;
  const char* invalid[] = {"", " ", "unknown", "MODE drp", "mode DRP", "mode dfp extra", "current 1.5",
      "duty +30", "duty -30", "debounce 118x", "debounce 99999999999999999999999999999999",
      "addr -1", "addr +71", "addr 0x", "addr 0x47g", "addr 256", "addr 99999999999999999999999999999999",
      "variant TUSB320LAI", "variant lai", "variant tusb320lai 0x47 extra", "profile invalid",
      "profile show extra", "profile reset extra", "profile validate extra", "cfg extra", "color yes", "verbose on",
      "ufpaccessory 1", "termination false", "rd-rp 0", "help all extra", "help unknown", "help raw", "help scan",
      "help firmware", "hilmark none", "hilmark a.b", "hilmark", "hilmark abc extra", "bind extra", "end extra",
      "forget-evidence extra", "version extra", "bus extra", "sample extra", "freshness extra",
      "profile.show", "a a a a a a a a a a a a a", "12345678901234567890123456789012345"};
  for (const auto* input : invalid) f.rejected(input);
  auto feedLine = [&f](const std::string& bytes) {
    f.output.clear();
    std::size_t offset = 0;
    while (offset < bytes.size()) {
      const auto consumed = f.cli.feed(bytes.data() + offset, bytes.size() - offset);
      CHECK(consumed <= 64);
      offset += consumed;
      f.cli.tick();
      if (consumed == 0 && offset < bytes.size()) CHECK(false);
    }
    f.flush();
    CHECK(f.output.find("command=invalid ") != std::string::npos);
  };
  const auto staged = f.cli.stagedProfile();
  const auto generation = f.cli.stagedGeneration();
  feedLine(std::string("mode dfp\0\b\n", 11));
  feedLine("mode dfp\x01\b\n");
  feedLine("mode dfp\x1b\b\n");
  feedLine(std::string(192, ' ') + "mode dfp\b\n");
  CHECK(sameProfile(staged, f.cli.stagedProfile()) && generation == f.cli.stagedGeneration());
  f.output.clear();
  CHECK(f.cli.feed("mo", 2) == 2);
  CHECK(!f.cli.processCommand("current 3a0"));
  CHECK(f.cli.feed("de dfx\bp\r", 9) == 9);
  CHECK(f.cli.stagedProfile().portMode == t::PortMode::DFP);
  CHECK(f.cli.feed("\ncurrent 3a0\n", 13) == 0);
  f.cli.tick();
  CHECK(f.cli.feed("\ncurrent 3a0\n", 13) == 13);
  f.flush();
  CHECK(f.cli.stagedProfile().sourceCurrent == t::SourceCurrent::MA_3000);
  CHECK(!f.cli.processCommand(nullptr));
  f.command("profile reset");
  std::string spaced(58, ' ');
  CHECK(f.cli.feed(spaced.data(), 58) == 58);
  CHECK(f.cli.feed("mode dfp\n", 9) == 6);
  CHECK(f.cli.feed("fp\n", 3) == 0);
  f.cli.tick();
  CHECK(f.cli.feed("fp\n", 3) == 3);
  f.flush();
  CHECK(f.cli.stagedProfile().portMode == t::PortMode::DFP);
  f.output.clear();
  CHECK(f.cli.processCommand("version"));
  CHECK(!f.cli.processCommand("mode ufp") && f.cli.feed("mode ufp\n", 9) == 0);
  f.flush();
  CHECK(f.cli.stagedProfile().portMode == t::PortMode::DFP);
  const std::string longCommand = std::string("mode") + std::string(180, ' ') + "ufp";
  CHECK(f.cli.processCommand(longCommand.c_str())); f.flush();
  CHECK(f.cli.stagedProfile().portMode == t::PortMode::UFP);
  f.rejected((std::string("mode") + std::string(188, ' ') + "dfp").c_str());
  f.output.clear();
  CHECK(f.cli.feed("mode dfp\r", 9) == 9);
  f.cli.tick();
  CHECK(f.cli.feed("\n", 1) == 1);
  CHECK(f.cli.processCommand("version"));
  f.flush();
  CHECK(f.cli.feed("x\b", 2) == 2);
  CHECK(f.cli.processCommand("mode ufp"));
  f.flush();
  CHECK(f.cli.stagedProfile().portMode == t::PortMode::UFP);
  CHECK(f.cli.feed("\0", 1) == 1);
  f.cli.tick();
  CHECK(!f.cli.processCommand("mode dfp"));
  CHECK(f.cli.feed("\n", 1) == 1);
  f.flush();
  CHECK(f.cli.stagedProfile().portMode == t::PortMode::UFP);
}

void retainedEvidenceAndNoPoll() {
  Fixture f;
  f.initialize();
  const auto initialized = f.driver.configurationSnapshot();
  CHECK(initialized.rawConfigValidMask != 0 && initialized.observedValidFields != 0);
  f.command("current 3a0");
  CHECK(sameConfiguration(initialized, f.driver.configurationSnapshot()));
  f.rejected("addr 0x67", "BUSY", "addr");
  f.rejected("variant tusb320 0x60", "BUSY", "variant");
  f.rejected("forget-evidence", "INVALID_CONFIG", "forget-evidence");
  t::DeviceProfile desired;
  desired.sourceCurrent = t::SourceCurrent::MA_1500;
  t::OperationToken apply;
  CHECK(f.driver.startApplyProfile(desired, {100, 1000}, apply).inProgress());
  f.command("sample"); f.rejected("addr 0x67", "BUSY", "addr");
  CHECK(f.driver.progress().state == t::OperationState::ACTIVE);
  CHECK(f.driver.cancel(apply).ok());
  f.rejected("variant tusb320 0x60", "RESULT_PENDING", "variant");
  CHECK(f.command("profile discard").find("code=OK ") != std::string::npos);
  CHECK(sameProfile(f.cli.stagedProfile(), desired));
  f.rejected("mode invalid");
  f.command("end");
  f.rejected("forget-evidence", "RESULT_PENDING", "forget-evidence");
  t::OperationResult result;
  CHECK(f.driver.takeResult(apply, result).ok());
  f.command("forget-evidence");
  f.command("variant tusb320 0x60");
  CHECK(f.cli.targetConfig().variant == t::Variant::TUSB320);

  Fixture unresolved;
  unresolved.initialize();
  auto& failure = unresolved.bus.expectWrite(0x47, 0x08, 0x40);
  failure.status = t::Status::Error(t::Err::I2C_TIMEOUT, "fixture uncertain write", -2147483647);
  CHECK(unresolved.driver.writeRegisterSync(0x08, 0x40).is(t::Err::I2C_TIMEOUT));
  unresolved.command("end");
  unresolved.rejected("variant tusb320 0x60", "CONFIGURATION_UNKNOWN", "variant");
  unresolved.command("forget-evidence");
  unresolved.command("variant tusb320 0x60");
  CHECK(unresolved.cli.targetConfig().variant == t::Variant::TUSB320);

  Fixture failed;
  failed.initialize();
  auto& readFailure = failed.bus.expectRead(0x47, 0x08, 1);
  readFailure.status = t::Status::Error(t::Err::I2C_TIMEOUT, "retained failure", -2147483647);
  t::OperationToken read;
  CHECK(failed.driver.startReadProfile({100, 1000}, read).inProgress());
  failed.driver.poll(read, 100);
  CHECK(failed.driver.progress().resultPending);
  CHECK(failed.driver.resultSnapshot(read, result).ok() && result.status.is(t::Err::I2C_TIMEOUT));
  const auto attempts = failed.bus.calls;
  for (const auto* query : {"version", "bus", "console", "capabilities", "variant", "addr", "cfg", "sample", "freshness", "mode"})
    CHECK(failed.command(query).find("code=OK ") != std::string::npos);
  failed.rejected("mode dfp extra");
  failed.rejected("addr 0x67", "RESULT_PENDING", "addr");
  CHECK(failed.bus.calls == attempts && failed.driver.resultSnapshot(read, result).ok());
  CHECK(result.status.detail == -2147483647 && result.status.is(t::Err::I2C_TIMEOUT));

  Fixture attribution;
  attribution.command("variant tusb320 0x60");
  attribution.initialize();
  attribution.command("end");
  attribution.command("variant tusb320lai 0x47");
  const auto profiles = attribution.command("cfg");
  for (const auto* source : {"observed", "applied"}) {
    const auto start = profiles.find(std::string("source=") + source + " ");
    CHECK(start != std::string::npos);
    if (start == std::string::npos) continue;
    const auto record = profiles.substr(start, profiles.find('\n', start) - start);
    CHECK(record.find("variant=tusb320 ") != std::string::npos);
    CHECK(record.find("ufpaccessory=unknown") != std::string::npos &&
        record.find("termination=unknown") != std::string::npos &&
        record.find("preference=unknown") != std::string::npos);
  }
  CHECK(profiles.find("source=staged variant=tusb320lai ") != std::string::npos);
}

void outputAndSessionBoundaries() {
  Fixture f;
  CHECK(f.command("hilmark A_0-z").find("TUSB MARK schema=1 nonce=A_0-z request=1\n") != std::string::npos);
  CHECK(f.output.find("TUSB ACK") == std::string::npos);
  CHECK(f.command("ver").find("nonce=A_0-z request=2 command=version") != std::string::npos);
  f.rejected("hilmark none");
  CHECK(f.command("version").find("nonce=A_0-z") != std::string::npos);
  const char* nonce32 = "01234567890123456789012345678901";
  CHECK(f.command((std::string("hilmark ") + nonce32).c_str()).find("TUSB MARK") != std::string::npos);
  f.rejected("hilmark 012345678901234567890123456789012");
  f.writeLimit = 0; f.output.clear();
  CHECK(f.cli.processCommand("version"));
  auto writes = f.writes;
  f.cli.tick();
  CHECK(f.writes == writes + 1 && f.output.empty());
  f.writeLimit = 7; writes = f.writes; f.cli.tick();
  CHECK(f.writes == writes + 1 && f.output.size() == 7);
  f.flush();
  f.writeLimit = 0;
  for (unsigned index = 0; index < 200; ++index) { CHECK(f.cli.processCommand("version")); f.cli.tick(); }
  CHECK(f.cli.console().droppedRecords() > 0 && f.cli.console().lossPending());
  f.writeLimit = 256; f.output.clear(); f.flush();
  CHECK(f.output.find("TUSB LOSS schema=1 dropped_records=") != std::string::npos);
  std::size_t start = 0;
  while (start < f.output.size()) {
    const auto end = f.output.find('\n', start);
    CHECK(end != std::string::npos);
    if (end == std::string::npos) break;
    const auto line = f.output.substr(start, end - start);
    CHECK(line.size() + 2 <= 512);
    if (line.rfind("TUSB ", 0) == 0) CHECK(line.find("schema=1 ") != std::string::npos);
    start = end + 1;
  }
  Fixture widths;
  widths.command("mode port");
  widths.command("preference try-source");
  widths.output.clear();
  tusb32x_cli::CliTestAccess::stageGeneration(widths.cli, std::numeric_limits<uint32_t>::max());
  tusb32x_cli::CliTestAccess::maximumRecord(widths.cli);
  widths.flush();
  CHECK(widths.cli.console().droppedRecords() == 0);
  CHECK(widths.output.find("18446744073709551615") != std::string::npos);
  CHECK(widths.output.find("4294967295") != std::string::npos);
  start = 0;
  while (start < widths.output.size()) {
    const auto end = widths.output.find('\n', start);
    CHECK(end != std::string::npos && end - start + 2 <= 512);
    if (end == std::string::npos) break;
    start = end + 1;
  }
  Fixture exhausted;
  tusb32x_cli::CliTestAccess::requestBase(exhausted.cli, std::numeric_limits<uint64_t>::max() - 1);
  CHECK(exhausted.cli.printHelp("all"));
  exhausted.cli.tick();
  CHECK(!exhausted.cli.processCommand("current 3a0"));
  CHECK(exhausted.cli.exhausted());
  exhausted.flush();
  CHECK(exhausted.output.find("request=18446744073709551615") != std::string::npos);
  CHECK(exhausted.output.find("TUSB SESSION") != std::string::npos);
  CHECK(exhausted.output.find("command=help outcome=SUCCEEDED") != std::string::npos);
  CHECK(exhausted.cli.feed("version\n", 8) == 0 && !exhausted.cli.printHelp());
  CHECK(exhausted.cli.setup().is(t::Err::INVALID_CONFIG));

  Fixture congested;
  congested.writeLimit = 0;
  for (unsigned index = 0; index < 100; ++index) {
    CHECK(congested.cli.processCommand("version"));
    congested.cli.tick();
  }
  CHECK(congested.cli.console().droppedRecords() > 0);
  tusb32x_cli::CliTestAccess::requestBase(congested.cli, std::numeric_limits<uint64_t>::max());
  const auto candidate = congested.cli.stagedProfile();
  CHECK(!congested.cli.processCommand("mode dfp"));
  CHECK(congested.cli.exhausted());
  const auto losses = congested.cli.console().droppedRecords();
  for (unsigned turn = 0; turn < 100; ++turn) {
    congested.cli.tick();
    CHECK(congested.cli.console().droppedRecords() == losses);
  }
  congested.writeLimit = 256;
  congested.flush();
  CHECK(congested.output.find("reason=request_exhausted") != std::string::npos);
  CHECK(sameProfile(candidate, congested.cli.stagedProfile()));
  congested.output.clear();
  CHECK(!congested.cli.processCommand("mode ufp"));
  congested.flush();
  CHECK(congested.output.find("request=0 ") == std::string::npos);

  Fixture maximum;
  auto platform = maximum.platform;
  platform.identity.git = "0123456789012345678901234567890123456789";
  platform.identity.tree = "01234567";
  platform.identity.build = "0123456789012345678901234567890123456789012345678901234567890123";
  platform.identity.framework = "01234567890123456789012345678901";
  platform.identity.frameworkVersion = platform.identity.framework;
  platform.identity.target = platform.identity.framework;
  platform.bus = platform.identity.framework;
  platform.framing = platform.identity.framework;
  platform.busHz = std::numeric_limits<uint32_t>::max();
  platform.sdaPin = std::numeric_limits<int32_t>::min();
  platform.sclPin = std::numeric_limits<int32_t>::min();
  platform.irqPin = std::numeric_limits<int32_t>::min();
  platform.counters = Fixture::maximumCounters;
  Cli maximumCli(maximum.driver, maximum.config, platform);
  CHECK(maximumCli.setup().ok());
  CHECK(maximumCli.processCommand("hilmark 01234567890123456789012345678901"));
  for (unsigned turn = 0; turn < 64; ++turn) maximumCli.tick();
  tusb32x_cli::CliTestAccess::requestBase(maximumCli, std::numeric_limits<uint64_t>::max() - 2);
  CHECK(maximumCli.processCommand("version"));
  for (unsigned turn = 0; turn < 64; ++turn) maximumCli.tick();
  CHECK(maximumCli.processCommand("bus"));
  for (unsigned turn = 0; turn < 64; ++turn) maximumCli.tick();
  CHECK(maximum.output.find("adapter_writes=4294967295 adapter_write_reads=4294967295") != std::string::npos);
  CHECK(maximum.output.find("sda=-2147483648 scl=-2147483648 irq=-2147483648") != std::string::npos);
  CHECK(maximumCli.console().droppedRecords() == 0 && maximum.bus.calls == 0);
  start = 0;
  while (start < maximum.output.size()) {
    const auto end = maximum.output.find('\n', start);
    CHECK(end != std::string::npos && end - start + 2 <= 512);
    if (end == std::string::npos) break;
    start = end + 1;
  }
}

void portFormatterAndStatusMessageBounds() {
  Fixture f;
  // These copied values exercise the real pure formatter's widest fields; they do
  // not represent a physical observation or seed any private driver evidence.
  t::PortSnapshot port;
  port.rawStatus08 = 255;
  port.rawStatus09 = 255;
  port.rawValidMask = 3;
  port.validFields = 511;
  port.invalidated = true;
  port.attachment = t::Attachment::LOCAL_SOURCE;
  port.localRole = t::PortRole::NOT_APPLICABLE;
  port.peerRole = t::PortRole::NOT_APPLICABLE;
  port.orientation = t::Orientation::NOT_APPLICABLE;
  port.detectedCurrent = t::DetectedCurrent::CHARGE_THROUGH_500_MA;
  port.advertisedCurrent = t::SourceCurrent::DEFAULT;
  port.accessory = t::Accessory::DEBUG_LOCAL_DFP;
  port.activeCable = true;
  port.interruptPending = true;
  port.sequence = std::numeric_limits<uint64_t>::max();
  port.observedUptimeMs = std::numeric_limits<uint64_t>::max();
  port.observedTimeValid = true;
  port.configGeneration = std::numeric_limits<uint32_t>::max();
  port.configurationState = t::ConfigurationState::UNCONFIGURED;
  port.coherence = t::SnapshotCoherence::CHANGED_DURING_READ;
  const uint64_t maximumAge = (uint64_t{1} << 63) - 1;
  tusb32x_cli::CliTestAccess::maximumPort(f.cli, port, port.observedUptimeMs + maximumAge);
  f.flush();
  CHECK(f.output.find("TUSB PORT schema=1") != std::string::npos);
  CHECK(f.output.find("TUSB PORT_DETAIL schema=1") != std::string::npos);
  CHECK(f.output.find("sequence=18446744073709551615 observed_ms=18446744073709551615") !=
      std::string::npos);
  CHECK(f.output.find("observed_time_valid=1 age_ms=9223372036854775807") != std::string::npos);
  CHECK(f.output.find("detected_current=CHARGE_THROUGH_500_MA") != std::string::npos);
  CHECK(f.output.find("config_generation=4294967295") != std::string::npos);
  CHECK(f.cli.console().droppedRecords() == 0 && f.bus.calls == 0);
  std::size_t start = 0;
  while (start < f.output.size()) {
    const auto end = f.output.find('\n', start);
    CHECK(end != std::string::npos && end - start + 2 <= 512);
    if (end == std::string::npos) break;
    start = end + 1;
  }
  f.output.clear();
  tusb32x_cli::CliTestAccess::maximumPort(f.cli, port, port.observedUptimeMs - 1);
  f.flush();
  CHECK(f.output.find("observed_time_valid=1 age_ms=unknown") != std::string::npos);
  port.observedTimeValid = false;
  f.output.clear();
  tusb32x_cli::CliTestAccess::maximumPort(f.cli, port, port.observedUptimeMs + maximumAge);
  f.flush();
  CHECK(f.output.find("observed_time_valid=0 age_ms=unknown") != std::string::npos);

  f.command("color off");
  f.denyTarget = true;
  f.targetErrorDetail = std::numeric_limits<int32_t>::min();
  static const std::string MESSAGE_255(255, 'x');
  static const std::string MESSAGE_256(256, 'x');
  f.targetErrorMessage = MESSAGE_255.c_str();
  const auto complete = f.command("addr 0x67");
  CHECK(complete.find(MESSAGE_255) != std::string::npos);
  for (const auto* message : {MESSAGE_256.c_str(), "embedded\nnewline", "escape\033[31m",
      "high\x80" "byte"}) {
    f.targetErrorMessage = message;
    const auto text = f.command("addr 0x67");
    CHECK(text.find("Message omitted; inspect typed code and detail") != std::string::npos);
    CHECK(text.find("code=INVALID_CONFIG detail=-2147483648") != std::string::npos);
    CHECK(text.find("command=addr outcome=FAILED code=INVALID_CONFIG detail=-2147483648 token=0 callbacks=0") !=
        std::string::npos);
    CHECK(text.find('\033') == std::string::npos);
    CHECK(f.cli.targetConfig().address == t::DeviceAddress::ADDR_LOW_0X47);
    CHECK(f.cli.console().droppedRecords() == 0 && f.bus.calls == 0);
  }
}
}  // namespace

void operationFormatterAndExhaustionBounds() {
  Fixture maximum;
  t::OperationResult result;
  result.token.value = std::numeric_limits<uint64_t>::max();
  result.kind = t::OperationKind::ACKNOWLEDGE_INTERRUPT;
  result.state = t::OperationState::FAILED;
  result.status = t::Status::Error(t::Err::CONFIGURATION_MISMATCH, "literal formatter fixture",
      std::numeric_limits<int32_t>::min());
  result.hardwareEffect = t::HardwareEffect::INDETERMINATE;
  result.transfers = std::numeric_limits<uint32_t>::max();
  result.startedUptimeMs = result.completedUptimeMs = std::numeric_limits<uint64_t>::max();
  result.completedTimeValid = true;
  result.configuration.desiredGeneration = result.configuration.verifiedGeneration =
      std::numeric_limits<uint32_t>::max();
  result.configuration.terminationState = t::TerminationState::DISABLED;
  result.configuration.resetState = t::ResetState::MAY_BE_PENDING;
  result.interrupt = {true, true, true, true, true, true};
  tusb32x_cli::CliTestAccess::maximumResult(maximum.cli, result);
  maximum.flush();
  CHECK(maximum.output.find("origin_request=18446744073709551615 token=18446744073709551615") !=
      std::string::npos);
  CHECK(maximum.output.find("callbacks=4294967295") != std::string::npos);
  CHECK(maximum.output.find("detail=-2147483648") != std::string::npos);
  CHECK(maximum.cli.console().droppedRecords() == 0 && maximum.bus.calls == 0);
  std::size_t lines = 0;
  for (std::size_t start = 0; start < maximum.output.size();) {
    const auto end = maximum.output.find('\n', start);
    CHECK(end != std::string::npos);
    if (end == std::string::npos) break;
    CHECK(end - start + 2 <= 512);
    ++lines;
    start = end + 1;
  }
  CHECK(lines == 3);

  Fixture active;
  active.command("bind");
  active.bus.expectRead(0x47, 9, 1).rx[0] = 0;
  active.bus.expectRead(0x47, 8, 1).rx[0] = 0;
  active.bus.expectRead(0x47, 9, 1).rx[0] = 0;
  tusb32x_cli::CliTestAccess::requestBase(active.cli, std::numeric_limits<uint64_t>::max() - 1);
  CHECK(active.cli.processCommand("read"));
  active.cli.tick();
  CHECK(!active.cli.processCommand("version") && active.cli.exhausted());
  active.flush();
  CHECK(active.bus.calls == 3 && active.bus.complete() && active.driver.progress().resultPending);
  CHECK(active.output.find("request=18446744073709551615 command=read outcome=SUCCEEDED") !=
      std::string::npos);
  CHECK(active.output.find("request=0 ") == std::string::npos);
}

int runCliOperationTests();
int runCliDiagnosticsTests();

void diagnosticFormatterAndSaturationBounds() {
  constexpr uint32_t maximum = std::numeric_limits<uint32_t>::max();
  for (unsigned field = 0; field < 3; ++field) {
    t::TransportStats stats;
    if (field == 0) stats.totalAttempts = maximum;
    else if (field == 1) stats.totalSuccess = maximum;
    else stats.totalFailures = maximum;
    CHECK(tusb32x_cli::CliTestAccess::assertCounters(stats, stats.totalAttempts,
        stats.totalSuccess, stats.totalFailures, true).is(t::Err::INVALID_PARAM));
    CHECK(tusb32x_cli::CliTestAccess::assertCounters(stats, stats.totalAttempts, 0, 0, false)
        .is(t::Err::INVALID_PARAM));
  }
  t::TransportStats stats;
  stats.totalAttempts = maximum - 1; stats.totalSuccess = maximum - 2; stats.totalFailures = 1;
  CHECK(tusb32x_cli::CliTestAccess::assertCounters(stats, maximum - 1, maximum - 2, 1, true).ok());
  CHECK(tusb32x_cli::CliTestAccess::assertCounters(stats, maximum - 1, 0, 0, false).ok());
  CHECK(tusb32x_cli::CliTestAccess::assertCounters(stats, maximum - 1, 0, 0, true)
      .is(t::Err::INVALID_PARAM));

  // Literal copied values bound actual formatters without seeding a driver or claiming observations.
  Fixture f;
  t::DriverDiagnostics diagnostics;
  diagnostics.bound = diagnostics.initialized = true;
  diagnostics.configurationState = t::ConfigurationState::UNCONFIGURED;
  diagnostics.hardwareEffect = t::HardwareEffect::INDETERMINATE;
  diagnostics.resetState = t::ResetState::MAY_BE_PENDING;
  diagnostics.terminationState = t::TerminationState::DISABLED;
  diagnostics.configurationWriteUnresolved = diagnostics.recoverySequenceUnresolved =
      diagnostics.modeTransitionUnresolved = diagnostics.resetAcceptanceUncertain = true;
  diagnostics.lastOperationKind = t::OperationKind::ACKNOWLEDGE_INTERRUPT;
  diagnostics.lastOperationStatus = t::Status::Error(t::Err::CONFIGURATION_MISMATCH,
      "Literal formatting fixture", std::numeric_limits<int32_t>::min());
  diagnostics.lastFailedTransportStatus = diagnostics.lastOperationStatus;
  diagnostics.lastFailedTransportUptimeMs = std::numeric_limits<uint64_t>::max();
  diagnostics.lastFailedTransportTimeValid = true;
  auto& managed = diagnostics.managedTransport;
  managed.initialized = true;
  managed.totalSuccess = managed.totalFailures = managed.consecutiveFailures = maximum;
  managed.lastStatus = diagnostics.lastOperationStatus;
  managed.lastSuccessUptimeMs = managed.lastFailureUptimeMs = std::numeric_limits<uint64_t>::max();
  managed.lastSuccessTimeValid = managed.lastFailureTimeValid = true;
  diagnostics.identity.variant = t::Variant::TUSB320LAI;
  diagnostics.identity.address = t::DeviceAddress::ADDR_HIGH_0X67;
  diagnostics.identity.identityValid = diagnostics.identity.revisionValid = true;
  std::memset(diagnostics.identity.rawIdentity, 255, 8);
  diagnostics.identity.rawRevision = 255;
  t::ConfigurationSnapshot configuration;
  configuration.observedUptimeMs = std::numeric_limits<uint64_t>::max();
  configuration.observedTimeValid = configuration.mismatchValid = true;
  configuration.mismatchRegister = configuration.mismatchExpected =
      configuration.mismatchObserved = configuration.mismatchMask = 255;
  configuration.resetState = t::ResetState::MAY_BE_PENDING;
  stats.totalAttempts = stats.totalSuccess = stats.totalFailures = maximum;
  stats.lastStatus = diagnostics.lastOperationStatus;
  t::HealthSnapshot health;
  health.state = t::DriverState::DEGRADED; health.evidence = managed; health.offlineThreshold = maximum;
  tusb32x_cli::CliTestAccess::maximumDiagnostics(f.cli, diagnostics, configuration, stats, health);
  f.flush();
  for (const auto* type : {"CONFIG_DETAIL", "DIAG", "MANAGED", "FAILURE", "IDENTITY", "IRQ",
      "XFER", "DRV", "REG", "DECODE", "DECODE_PROFILE"})
    CHECK(f.output.find(std::string("TUSB ") + type + " schema=1 nonce=01234567890123456789012345678901 request=18446744073709551615") != std::string::npos);
  CHECK(f.output.find("mismatch_register=255 mismatch_expected=255 mismatch_observed=255 mismatch_mask=255")
      != std::string::npos);
  CHECK(f.output.find("bytes=FFFFFFFFFFFFFFFF revision=255") != std::string::npos);
  CHECK(f.output.find("source=all_callbacks attempts=4294967295") != std::string::npos);
  CHECK(f.output.find("source=managed_callbacks state=DEGRADED threshold=4294967295") != std::string::npos);
  CHECK(f.cli.console().droppedRecords() == 0 && f.bus.calls == 0);
  for (std::size_t start = 0; start < f.output.size();) {
    const auto end = f.output.find('\n', start);
    CHECK(end != std::string::npos && end - start + 2 <= 512);
    if (end == std::string::npos) break;
    start = end + 1;
  }
}

int main() {
  {
    Fixture interleaved;
    CHECK(interleaved.cli.processCommand("help all"));
    interleaved.cli.tick();
    CHECK(interleaved.cli.processCommand("hilmark newer_request"));
    interleaved.cli.tick();
    CHECK(interleaved.cli.processCommand("mode ufp"));
    interleaved.cli.tick();
    CHECK(interleaved.cli.processCommand("help profile"));
    interleaved.cli.tick();
    interleaved.flush();
    const auto& output = interleaved.output;
    CHECK(output.find("TUSB MARK schema=1 nonce=newer_request request=2\n") != std::string::npos);
    CHECK(output.find("nonce=newer_request request=3 command=mode code=OK") != std::string::npos);
    CHECK(output.find("nonce=newer_request request=4 command=help code=BUSY") != std::string::npos);
    const std::string closure = "TUSB END schema=1 nonce=none request=1 command=help ";
    const auto end = output.find(closure);
    CHECK(end != std::string::npos && output.find(closure, end + 1) == std::string::npos);
    CHECK(output.find("TUSB HELP schema=1 nonce=newer_request") == std::string::npos);
    CHECK(output.find("TUSB HELP schema=1 nonce=none request=1 canonical=rd-rp") != std::string::npos);
    CHECK(interleaved.cli.console().droppedRecords() == 0 && interleaved.bus.calls == 0);
  }
  routesAndAliases(); setupAndAppearanceContracts(); stagingAndAtomicTargets(); malformedAndPoisonedInput();
  retainedEvidenceAndNoPoll(); outputAndSessionBoundaries();
  portFormatterAndStatusMessageBounds();
  operationFormatterAndExhaustionBounds();
  diagnosticFormatterAndSaturationBounds();
  std::printf("CLI: %u checks, %u failures\n", checks, failures);
  const int operationFailures = runCliOperationTests();
  const int diagnosticFailures = runCliDiagnosticsTests();
  return operationFailures == 0 && diagnosticFailures == 0 && failures == 0 ? 0 : 1;
}
