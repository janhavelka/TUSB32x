/// @file test_cli_operations.cpp
/// @brief Executed managed CLI routing, owner turns, retained results and literal device effects.
/// @details Byte oracles: archived TI SLLSEQ8D Tables 8-10 pp17-19 (08 current, 09 W1C/duty,
/// 0A mode/reset), SLLSEN9F section 7.4.3 p14 and Table 7-7 p19 (base recovery), and
/// SLLSEO5D Table 6-10 p20 (322I mode). Scripts never call production encoders/composers.
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
  if (!condition) { ++failures; std::printf("FAIL CLI operations line %d: %s\n", line, expression); }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

/// @brief Independent callback script; only explicit writes change the fixture register image.
struct Step {
  uint8_t reg = 0;
  uint8_t value = 0;
  bool write = false;
  uint32_t timeout = 20;
  t::Status status = t::Status::Ok();
  bool accepted = true;
  uint64_t elapsedMs = 0;
};

struct Fixture {
  t::TUSB32x driver;
  t::DriverConfig config;
  tusb32x_cli::Platform platform;
  tusb32x_cli::Cli cli;
  std::vector<Step> script;
  std::size_t calls = 0;
  uint8_t registers[256]{};
  uint64_t now = 100;
  uint64_t requests = 0;
  std::size_t writeLimit = 256;
  std::string output;

  explicit Fixture(t::Variant variant = t::Variant::TUSB320LAI)
      : config(configuration(this, variant)), platform(hooks(this)), cli(driver, config, platform) {
    registers[0x0A] = 0x30; // TI MODE_SELECT=DRP, default debounce, termination enabled.
    registers[9] = 0x04; // TI DRP_DUTY_CYCLE=10, the independently staged 50% default.
    registers[0xA0] = 2;
    CHECK(cli.setup().ok());
  }
  static t::DriverConfig configuration(Fixture* fixture, t::Variant variant) {
    t::DriverConfig result;
    result.variant = variant;
    result.address = variant == t::Variant::TUSB320 || variant == t::Variant::TUSB320I
        ? t::DeviceAddress::ADDR_LOW_0X60 : t::DeviceAddress::ADDR_LOW_0X47;
    result.i2cUser = fixture;
    result.i2cWrite = writeBus;
    result.i2cWriteRead = readBus;
    return result;
  }
  static uint64_t clock(void* user) { return static_cast<Fixture*>(user)->now; }
  static std::size_t outputWrite(const char* bytes, std::size_t length, void* user) {
    auto& fixture = *static_cast<Fixture*>(user);
    CHECK(length <= 256);
    const auto accepted = std::min(length, fixture.writeLimit);
    fixture.output.append(bytes, accepted);
    return accepted;
  }
  static tusb32x_cli::Platform hooks(Fixture* fixture) {
    tusb32x_cli::Platform result;
    result.nowMs = clock; result.write = outputWrite; result.user = fixture;
    return result;
  }
  t::Status invoke(bool writing, uint8_t address, const uint8_t* tx, std::size_t txLength,
                   uint8_t* rx, std::size_t rxLength, uint32_t timeout) {
    const auto index = calls++;
    CHECK(index < script.size());
    if (index >= script.size()) return t::Status::Error(t::Err::I2C_ERROR, "unexpected callback", -90001);
    const auto expected = script[index];
    CHECK(address == static_cast<uint8_t>(config.address));
    CHECK(tx != nullptr && txLength == (writing ? 2U : 1U));
    CHECK(writing == expected.write && tx[0] == expected.reg && timeout == expected.timeout);
    CHECK(rxLength == (writing ? 0U : expected.reg == 0 ? 8U : 1U));
    if (writing) {
      CHECK(tx[1] == expected.value && rx == nullptr);
      if (expected.accepted) {
        if (tx[0] == 9) {
          // TI 09[4] is W1C; RO attachment bits and an unrelated pending latch survive W1C=0.
          const uint8_t pending = (tx[1] & 0x10U) ? 0 : registers[9] & 0x10U;
          registers[9] = static_cast<uint8_t>((registers[9] & 0xE0U) | (tx[1] & 0x07U) | pending);
        } else registers[tx[0]] = tx[1];
      }
    } else if (rx != nullptr) {
      if (expected.reg == 0) {
        const uint8_t id[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
        std::memcpy(rx, id, sizeof(id));
        if (config.variant == t::Variant::TUSB322I) rx[0] = 0x32;
      } else rx[0] = registers[expected.reg];
    }
    now += expected.elapsedMs;
    return expected.status;
  }
  static t::Status writeBus(uint8_t address, const uint8_t* tx, std::size_t txLength,
                            uint32_t timeout, void* user) {
    return static_cast<Fixture*>(user)->invoke(true, address, tx, txLength, nullptr, 0, timeout);
  }
  static t::Status readBus(uint8_t address, const uint8_t* tx, std::size_t txLength,
                           uint8_t* rx, std::size_t rxLength, uint32_t timeout, void* user) {
    return static_cast<Fixture*>(user)->invoke(false, address, tx, txLength, rx, rxLength, timeout);
  }
  bool base() const { return config.variant == t::Variant::TUSB320 || config.variant == t::Variant::TUSB320I; }
  void read(uint8_t reg, uint32_t timeout = 20) { script.push_back({reg, 0, false, timeout}); }
  void write(uint8_t reg, uint8_t value) { script.push_back({reg, value, true, 20}); }
  void profile() {
    read(8); read(9); read(10);
    if (config.variant != t::Variant::TUSB322I) read(0x45);
  }
  void proof() {
    read(0);
    if (!base()) read(0xA0);
    profile();
  }
  void refresh() { read(9); read(8); read(9); }
  void tick() {
    const auto before = calls;
    cli.tick();
    CHECK(calls - before <= 1); // Basic status-only callbacks stop even with an N budget.
  }
  void drain() {
    for (unsigned turn = 0; turn < 256 && cli.console().queued() != 0; ++turn) tick();
    if (writeLimit != 0) CHECK(cli.console().queued() == 0);
  }
  void send(const std::string& command) {
    const auto before = calls;
    CHECK(cli.processCommand(command.c_str()));
    ++requests;
    CHECK(calls == before); // Admission, cached queries and cancellation are all bus-silent.
    tick();
  }
  std::string command(const std::string& line) {
    output.clear(); send(line); drain(); return output;
  }
  void manual() { CHECK(command("job auto 0").find("code=OK ") != std::string::npos); }
  void initialize() {
    CHECK(driver.bind(config).ok());
    proof();
    t::OperationToken token;
    CHECK(driver.startInitialize({now, now + 1000}, token).inProgress());
    for (unsigned turn = 0; turn < 32 && driver.progress().state == t::OperationState::ACTIVE; ++turn)
      driver.poll(token, now);
    t::OperationResult result;
    CHECK(driver.takeResult(token, result).ok() && result.status.ok());
    CHECK(calls == script.size());
  }
  void finishManual() {
    for (unsigned turn = 0; turn < 32 && driver.progress().state == t::OperationState::ACTIVE; ++turn)
      send("job step 255");
    CHECK(driver.progress().resultPending && calls == script.size());
    drain();
  }
  t::OperationResult result() const {
    t::OperationResult result;
    CHECK(driver.resultSnapshot(driver.progress().token, result).ok());
    return result;
  }
  void take() {
    const auto token = driver.progress().token.value;
    CHECK(command("result take " + std::to_string(token)).find("command=result.take code=OK ") != std::string::npos);
    CHECK(!driver.progress().resultPending);
  }
  void establishDesired() {
    proof(); send("apply"); finishManual(); CHECK(result().status.ok()); take();
  }
};

std::size_t occurrences(const std::string& text, const std::string& needle) {
  std::size_t count = 0;
  for (std::size_t at = 0; (at = text.find(needle, at)) != std::string::npos; at += needle.size()) ++count;
  return count;
}

void admissionAndAliases() {
  const char* init[] = {"init", "initialize", "probe"};
  for (const auto* alias : init) {
    Fixture f; f.manual();
    CHECK(f.command(alias).find("code=NOT_BOUND ") != std::string::npos);
    f.command("bind");
    f.output.clear();
    CHECK(f.cli.processCommand(alias));
    ++f.requests;
    CHECK(f.calls == 0 && f.output.empty() && f.cli.console().queued() != 0);
    f.tick(); f.drain();
    const auto text = f.output;
    CHECK(text.find("command=init code=IN_PROGRESS ") != std::string::npos);
    CHECK(text.find("action=ADMITTED token=") != std::string::npos && f.calls == 0);
    const auto job = text.find("TUSB JOB ");
    CHECK(job != std::string::npos && text.find("TUSB ACK ") < job);
    CHECK(occurrences(text, "TUSB JOB ") == 1);
    CHECK(text.find("kind=INITIALIZE state=ACTIVE phase=READING code=IN_PROGRESS detail=0 callbacks=0") != std::string::npos);
    CHECK(text.find("TUSB END ") == std::string::npos);
    CHECK(f.driver.progress().kind == t::OperationKind::INITIALIZE);
    f.command("cancel"); CHECK(f.result().status.is(t::Err::CANCELLED)); f.take();
  }
  Fixture f; f.manual(); f.initialize(); f.establishDesired();
  struct Route { const char* input; const char* canonical; t::OperationKind kind; };
  const Route routes[] = {
      {"read", "read", t::OperationKind::REFRESH_PORT},
      {"status", "read", t::OperationKind::REFRESH_PORT},
      {"profile read", "profile.read", t::OperationKind::READ_PROFILE},
      {"apply", "apply", t::OperationKind::APPLY_PROFILE},
      {"verify", "verify", t::OperationKind::VERIFY_PROFILE},
      {"selfcheck", "verify", t::OperationKind::VERIFY_PROFILE},
      {"selftest", "verify", t::OperationKind::VERIFY_PROFILE},
      {"reconcile", "reconcile", t::OperationKind::RECONCILE},
      {"irq read", "irq.read", t::OperationKind::REFRESH_PORT},
      {"irq ack", "irq.ack", t::OperationKind::ACKNOWLEDGE_INTERRUPT},
      {"irq service", "irq.service", t::OperationKind::SERVICE_INTERRUPT},
      {"reset", "reset", t::OperationKind::RESET}};
  unsigned index = 0;
  for (const auto& route : routes) {
    const auto before = f.calls;
    const auto text = f.command(route.input);
    CHECK(text.find(std::string("command=") + route.canonical + " code=IN_PROGRESS ") != std::string::npos);
    CHECK(f.driver.progress().kind == route.kind && f.calls == before);
    const auto job = f.command("job");
    CHECK(job.find("TUSB JOB ") != std::string::npos);
    for (const auto* field : {" token=", " kind=", " state=", " phase=", " callbacks=",
         " last_poll_callbacks=", " deadline_ms=", " waiting=", " ready_at_ms=",
         " ready_at_valid=", " result_pending=", " auto="}) CHECK(job.find(field) != std::string::npos);
    CHECK(f.command("job status").find("state=ACTIVE ") != std::string::npos);
    CHECK(f.command("progress").find("state=ACTIVE ") != std::string::npos);
    CHECK(f.command("read").find("code=BUSY ") != std::string::npos);
    CHECK(f.command("result").find("code=RESULT_NOT_AVAILABLE ") != std::string::npos);
    const char* cancels[] = {"cancel", "job cancel", "stop"};
    f.command(cancels[index++ % 3]);
    CHECK(f.result().status.is(t::Err::CANCELLED) && f.calls == before);
    CHECK(f.command("read").find("code=RESULT_PENDING ") != std::string::npos);
    f.take();
  }
  Fixture noDesired; noDesired.manual(); noDesired.initialize();
  CHECK(noDesired.command("selftest").find("code=CONFIGURATION_UNKNOWN ") != std::string::npos);
  const char* invalid[] = {"init 0", "init -1", "init +1", "init 0x10", "init 2147483648",
      "init 99999999999999999999999999999999", "apply 1x", "verify 1 2", "read 10",
      "status 10", "irq read 10", "job auto 2", "job auto -1", "job step -1", "job step 256",
      "job step 0x1", "cancel +1", "cancel 18446744073709551616", "result take", "result take 0x1"};
  for (const auto* line : invalid) {
    const auto before = noDesired.calls;
    CHECK(noDesired.command(line).find("code=INVALID_PARAM ") != std::string::npos);
    CHECK(noDesired.calls == before && noDesired.driver.progress().state == t::OperationState::IDLE);
  }
}

void successfulReadOnlyRoutesAndReset() {
  for (const auto* alias : {"init", "initialize", "probe"}) {
    Fixture f; f.manual(); f.command("bind"); f.registers[9] = 0x14;
    f.proof(); f.send(alias); f.finishManual();
    CHECK(f.result().status.ok() && f.result().hardwareEffect == t::HardwareEffect::NONE);
    CHECK(f.registers[9] == 0x14 && !f.driver.configurationSnapshot().hasDesiredProfile);
    for (const auto& step : f.script) CHECK(!step.write);
    f.take();
  }
  Fixture f; f.manual(); f.initialize(); f.establishDesired();
  for (const auto* alias : {"profile read", "verify", "selfcheck", "selftest", "reconcile"}) {
    const auto desiredGeneration = f.driver.configurationSnapshot().desiredGeneration;
    const auto first = f.script.size();
    if (std::strcmp(alias, "profile read") == 0) f.profile(); else f.proof();
    f.registers[9] = 0x14;
    f.send(alias); f.finishManual();
    CHECK(f.result().status.ok() && f.result().hardwareEffect == t::HardwareEffect::NONE);
    CHECK(f.registers[9] == 0x14 && f.driver.configurationSnapshot().desiredGeneration == desiredGeneration);
    for (auto index = first; index < f.script.size(); ++index) CHECK(!f.script[index].write);
    f.take();
  }
  for (const auto variant : {t::Variant::TUSB320, t::Variant::TUSB320LAI, t::Variant::TUSB322I}) {
    Fixture reset(variant); reset.manual(); reset.initialize();
    reset.read(10); reset.write(10, 0x38);
    reset.send("reset"); reset.send("job step"); reset.send("job step"); reset.send("job step 0");
    CHECK(reset.registers[10] == 0x38 && reset.driver.progress().waiting);
    reset.now = reset.driver.progress().readyAtMs;
    // Supply explicit post-reset observations; the fixture does not infer reset completion.
    reset.registers[10] = 0x30; reset.proof(); reset.finishManual();
    CHECK(reset.result().status.ok() && reset.result().hardwareEffect == t::HardwareEffect::CONFIRMED);
    CHECK(reset.result().configuration.resetState == t::ResetState::COMPLETED);
    CHECK(reset.result().transfers == (variant == t::Variant::TUSB320LAI ? 8U : 7U));
    CHECK(!reset.driver.configurationSnapshot().hasDesiredProfile);
  }
}

void budgetsAndAutomaticProgress() {
  Fixture f; f.manual(); f.command("bind"); f.proof();
  f.command("init 2147483647");
  const auto token = f.driver.progress().token;
  const auto deadline = f.now + 2147483647U;
  CHECK(f.driver.progress().deadlineMs == deadline);
  f.command("job step 0"); CHECK(f.calls == 0 && f.driver.progress().lastPollTransfers == 0);
  f.command("job step"); CHECK(f.calls == 1);
  f.command("job step 255"); CHECK(f.calls == 2 && f.driver.progress().lastPollTransfers == 1);
  f.command("job auto 1");
  // Draining advances admitted work, but a response may fit before the operation finishes.
  for (unsigned turn = 0; turn < 8 && !f.driver.progress().resultPending; ++turn) f.tick();
  CHECK(f.driver.progress().resultPending && f.calls == 6);
  CHECK(f.driver.progress().deadlineMs == deadline && f.driver.progress().token.value == token.value);
  f.take();

  Fixture overrideBudget; overrideBudget.manual(); overrideBudget.command("bind"); overrideBudget.proof();
  overrideBudget.send("init");
  overrideBudget.send("job auto 1"); CHECK(overrideBudget.calls == 1);
  overrideBudget.send("job step 0"); CHECK(overrideBudget.calls == 1);
  overrideBudget.send("job step 255"); CHECK(overrideBudget.calls == 2);
  overrideBudget.send("not-a-command"); CHECK(overrideBudget.calls == 3);
  overrideBudget.send("sample"); CHECK(overrideBudget.calls == 4);
  overrideBudget.send("job status"); CHECK(overrideBudget.calls == 5);
  overrideBudget.tick(); CHECK(overrideBudget.calls == 6);
  overrideBudget.tick(); CHECK(overrideBudget.driver.progress().resultPending);

  Fixture automatic; automatic.command("bind"); automatic.refresh();
  automatic.send("read"); CHECK(automatic.calls == 1);
  automatic.tick(); CHECK(automatic.calls == 2);
  automatic.tick(); CHECK(automatic.calls == 3);
  automatic.tick(); CHECK(automatic.result().status.ok());

  // A pre-existing application operation is never adopted or polled by this CLI.
  Fixture foreign; CHECK(foreign.driver.bind(foreign.config).ok());
  t::OperationToken foreignToken;
  CHECK(foreign.driver.startRefreshPort({foreign.now, foreign.now + 1000}, foreignToken).inProgress());
  foreign.command("version"); foreign.command("job status"); foreign.tick();
  CHECK(foreign.calls == 0 && foreign.driver.progress().token.value == foreignToken.value);
  CHECK(foreign.driver.progress().state == t::OperationState::ACTIVE);
}

void immutableTimeAndLateSuccess() {
  Fixture minimum; minimum.manual(); minimum.command("bind"); minimum.read(0, 1);
  minimum.send("init 1");
  CHECK(minimum.driver.progress().deadlineMs == minimum.now + 1);
  minimum.send("job step"); CHECK(minimum.calls == 1);
  ++minimum.now; minimum.send("job step 0");
  CHECK(minimum.result().status.is(t::Err::TIMEOUT));

  Fixture clipped; clipped.manual(); clipped.command("bind");
  clipped.read(9, 3); clipped.send("read");
  const auto deadline = clipped.driver.progress().deadlineMs;
  clipped.now = deadline - 3;
  clipped.send("job step 1"); CHECK(clipped.calls == 1);
  CHECK(clipped.driver.progress().deadlineMs == deadline);
  clipped.now = deadline;
  clipped.send("job step 0");
  CHECK(clipped.result().status.is(t::Err::TIMEOUT) && clipped.calls == 1);

  Fixture late; late.manual(); late.command("bind"); late.refresh();
  late.script[2].elapsedMs = 1000;
  late.send("read");
  late.send("job step 1"); late.send("job step 1"); late.send("job step 1");
  CHECK(late.calls == 3 && late.driver.progress().state == t::OperationState::ACTIVE);
  late.send("job step 0");
  CHECK(late.result().status.is(t::Err::TIMEOUT));
  CHECK(late.driver.portSnapshot().rawValidMask == 0); // Late OK cannot publish pending bytes.

  Fixture waiting; waiting.manual(); waiting.initialize();
  waiting.read(10); waiting.write(10, 0x38); // TI reset is 0A[3], preserving actual 0x30 controls.
  waiting.send("reset 150");
  const auto resetDeadline = waiting.driver.progress().deadlineMs;
  waiting.send("job step"); waiting.send("job step"); waiting.send("job step 0");
  const auto ready = waiting.driver.progress().readyAtMs;
  CHECK(waiting.driver.progress().waiting && ready == waiting.now + 96);
  const auto callbacks = waiting.calls;
  waiting.now = ready - 1;
  waiting.send("job step 255");
  CHECK(waiting.calls == callbacks && waiting.driver.progress().waiting);
  CHECK(waiting.driver.progress().deadlineMs == resetDeadline);
  waiting.now = resetDeadline;
  waiting.send("job step 0"); CHECK(waiting.result().status.is(t::Err::TIMEOUT));
  CHECK(waiting.result().hardwareEffect == t::HardwareEffect::PARTIAL);
}

void rejectedOwnerClockSamples() {
  for (const uint64_t invalidNow : {uint64_t{99}, uint64_t{100} + (uint64_t{1} << 63U)}) {
    Fixture manual; manual.manual(); manual.command("bind"); manual.refresh(); manual.send("read");
    const auto before = manual.driver.progress();
    manual.now = invalidNow;
    const auto text = manual.command("job step 255");
    const auto after = manual.driver.progress();
    CHECK(text.find("command=job.step outcome=FAILED code=INVALID_PARAM") != std::string::npos);
    CHECK(text.find("code=INVALID_PARAM detail=0") != std::string::npos);
    CHECK(text.find("TUSB RESULT ") == std::string::npos && text.find("command=read outcome=") == std::string::npos);
    CHECK(after.state == t::OperationState::ACTIVE && after.token.value == before.token.value);
    CHECK(after.deadlineMs == before.deadlineMs && after.transfers == before.transfers);
    CHECK(after.lastPollTransfers == before.lastPollTransfers && !after.resultPending && manual.calls == 0);
    manual.now = 100; manual.send("job step 1"); CHECK(manual.calls == 1);

    Fixture automatic; automatic.command("bind"); automatic.refresh(); automatic.send("read");
    const auto active = automatic.driver.progress();
    const auto origin = automatic.requests;
    CHECK(automatic.calls == 1);
    automatic.now = invalidNow; automatic.output.clear(); automatic.send("version");
    const auto rejected = automatic.driver.progress();
    CHECK(rejected.state == t::OperationState::ACTIVE && rejected.token.value == active.token.value);
    CHECK(rejected.deadlineMs == active.deadlineMs && rejected.transfers == active.transfers);
    CHECK(rejected.lastPollTransfers == active.lastPollTransfers && !rejected.resultPending && automatic.calls == 1);
    automatic.send("job auto 0"); automatic.drain();
    CHECK(automatic.output.find("TUSB JOB schema=1 nonce=none request=" + std::to_string(origin) + " token=" +
        std::to_string(active.token.value)) != std::string::npos);
    CHECK(automatic.output.find("code=INVALID_PARAM detail=0") != std::string::npos);
    CHECK(automatic.output.find("TUSB RESULT ") == std::string::npos);
    CHECK(automatic.output.find("command=read outcome=") == std::string::npos);
    automatic.now = 100; automatic.send("job step 1"); CHECK(automatic.calls == 2);
  }
}

void retainedResultsAndCorrelation() {
  Fixture f; f.manual(); f.initialize();
  // Fill requests so that apply is precisely 24 and the later cached status is 25.
  while (f.requests < 22) f.command("version");
  f.command("hilmark original");
  f.proof(); f.output.clear(); f.send("apply");
  const auto token = f.driver.progress().token.value;
  CHECK(f.requests == 24);
  f.send("job status"); CHECK(f.requests == 25);
  f.send("hilmark newer");
  f.finishManual();
  const auto spontaneous = f.output;
  CHECK(spontaneous.find("TUSB RESULT schema=1 nonce=original request=24 origin_request=24 token=" + std::to_string(token)) != std::string::npos);
  CHECK(spontaneous.find("TUSB RESULT_DETAIL schema=1 nonce=original request=24 origin_request=24 token=" + std::to_string(token)) != std::string::npos);
  CHECK(spontaneous.find("TUSB END schema=1 nonce=original request=24 command=apply outcome=SUCCEEDED") != std::string::npos);
  CHECK(occurrences(spontaneous, "TUSB RESULT ") == 1 && occurrences(spontaneous, "command=apply outcome=") == 1);
  for (const auto* field : {" kind=", " state=", " code=", " detail=", " effect=", " callbacks=",
       " desired_generation=", " applied_generation=", " configuration=", " started_ms=",
       " completed_ms=", " completed_time_valid=", " termination=", " reset=", " irq_observed=",
       " irq_before=", " ack_attempted=", " ack_confirmed=", " irq_after=", " irq_after_valid="})
    CHECK(spontaneous.find(field) != std::string::npos);
  f.output.clear(); f.drain(); f.tick(); f.drain();
  CHECK(f.output.find("TUSB RESULT ") == std::string::npos);
  for (const auto* alias : {"result", "job result"}) {
    const auto text = f.command(alias);
    CHECK(text.find("nonce=newer") != std::string::npos && text.find("origin_request=24 token=" + std::to_string(token)) != std::string::npos);
    CHECK(f.driver.progress().resultPending);
  }
  CHECK(f.command("result " + std::to_string(token + 1)).find("code=TOKEN_MISMATCH ") != std::string::npos);
  CHECK(f.command("result take " + std::to_string(token + 1)).find("code=TOKEN_MISMATCH ") != std::string::npos);
  CHECK(f.command("cancel " + std::to_string(token + 1)).find("code=TOKEN_MISMATCH ") != std::string::npos);
  CHECK(f.driver.progress().resultPending);
  f.take();
  CHECK(f.command("result take " + std::to_string(token)).find("code=RESULT_NOT_AVAILABLE ") != std::string::npos);
}

void outputLossAndTakeReservation() {
  Fixture f; f.manual(); f.command("bind");
  f.refresh(); f.send("read");
  f.writeLimit = 0;
  for (unsigned index = 0; index < 80; ++index) f.send("version");
  const auto dropped = f.cli.console().droppedRecords();
  CHECK(dropped != 0);
  f.finishManual();
  const auto token = f.driver.progress().token.value;
  CHECK(f.result().status.ok()); // Poll and completion proceed even when terminal records are lost.
  f.send("result take " + std::to_string(token));
  CHECK(f.driver.progress().resultPending);
  CHECK(f.cli.console().droppedRecords() > dropped);
  f.writeLimit = 256; f.output.clear(); f.drain();
  CHECK(f.output.find("TUSB LOSS ") != std::string::npos);
  const auto replay = f.command("result");
  CHECK(replay.find("TUSB RESULT ") != std::string::npos && replay.find("code=OK ") != std::string::npos);
  f.take();

  Fixture reserve; reserve.manual(); reserve.command("bind"); reserve.refresh();
  reserve.send("read"); reserve.finishManual(); reserve.drain();
  reserve.writeLimit = 0;
  // The actual short response would fit, but the full worst-case response reservation would not.
  for (unsigned turn = 0; turn < 80 && reserve.cli.console().available() >= 5U * 511U; ++turn)
    reserve.send("version");
  CHECK(reserve.cli.console().available() < 5U * 511U && reserve.cli.console().available() > 1500);
  CHECK(reserve.cli.console().droppedRecords() == 0);
  reserve.send("result take " + std::to_string(reserve.driver.progress().token.value));
  CHECK(reserve.driver.progress().resultPending);
  CHECK(reserve.cli.console().droppedRecords() == 0 && reserve.cli.console().droppedBytes() == 0);
  reserve.writeLimit = 256; reserve.drain();
  CHECK(reserve.output.find("command=result.take code=BUSY") != std::string::npos);
  CHECK(reserve.output.find("command=result.take outcome=FAILED code=BUSY") != std::string::npos);
  reserve.take();

  for (const auto* command : {"cancel", "end"}) {
    Fixture blocked; blocked.manual(); blocked.initialize(); blocked.command("current 1a5");
    blocked.write(8, 0x40); blocked.send("apply"); blocked.send("job step 1");
    CHECK(blocked.registers[8] == 0x40);
    blocked.writeLimit = 0;
    for (unsigned turn = 0; turn < 80; ++turn) blocked.send("version");
    CHECK(blocked.cli.console().droppedRecords() != 0);
    const auto calls = blocked.calls;
    blocked.send(command);
    CHECK(blocked.calls == calls && blocked.driver.progress().resultPending);
    CHECK(blocked.result().status.is(t::Err::CANCELLED));
    CHECK(blocked.result().hardwareEffect == t::HardwareEffect::PARTIAL);
    CHECK(blocked.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    CHECK(blocked.registers[8] == 0x40); // Congested output never schedules rollback.
    if (std::strcmp(command, "end") == 0) CHECK(!blocked.driver.diagnostics().bound);
  }

  Fixture expiry; expiry.command("bind"); expiry.writeLimit = 0;
  for (unsigned turn = 0; turn < 80; ++turn) expiry.send("version");
  expiry.read(0, 1); expiry.send("init 1");
  CHECK(expiry.calls == 1 && expiry.driver.progress().deadlineMs == expiry.now + 1);
  ++expiry.now; expiry.tick();
  CHECK(expiry.calls == 1 && expiry.driver.progress().resultPending);
  CHECK(expiry.result().status.is(t::Err::TIMEOUT));
}

void irqAndApplyEffects() {
  Fixture f; f.manual(); f.registers[9] = 0x04; f.initialize();
  CHECK(f.driver.configurationSnapshot().verifiedProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_50);
  f.command("duty 30");
  CHECK(f.cli.stagedProfile().drpDutyCycle == t::DrpDutyCycle::PERCENT_30);
  f.command("apply"); f.command("cancel");
  CHECK(f.result().hardwareEffect == t::HardwareEffect::NONE);
  CHECK(f.driver.configurationSnapshot().desiredProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_30);
  CHECK(f.driver.configurationSnapshot().verifiedProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_50);
  f.take();
  f.registers[9] = 0x14;
  f.read(9); f.write(9, 0x14);
  f.send("irq ack"); f.finishManual();
  CHECK(f.registers[9] == 0x04 && f.result().interrupt.acknowledgeConfirmed);
  CHECK(f.driver.configurationSnapshot().verifiedProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_50);
  f.take();

  f.registers[9] = 0x14;
  f.refresh(); f.write(9, 0x14); f.read(9);
  f.send("irq service"); f.finishManual();
  CHECK(f.result().status.ok() && f.result().transfers == 5);
  CHECK(f.result().interrupt.pendingBefore && f.result().interrupt.pendingAfterValid);
  CHECK(!f.result().interrupt.pendingAfter && f.registers[9] == 0x04);
  f.take();

  f.registers[9] = 0x14;
  f.write(9, 0x00); f.proof(); // 30% is code00; W1C=0 must preserve the pending latch.
  f.send("apply"); f.finishManual();
  CHECK(f.result().status.ok() && f.registers[9] == 0x10);
  CHECK(f.driver.configurationSnapshot().verifiedProfile.drpDutyCycle == t::DrpDutyCycle::PERCENT_30);
  f.take();
  f.refresh(); f.send("irq read"); f.finishManual();
  CHECK(f.driver.portSnapshot().interruptPending && !f.driver.portSnapshot().invalidated);
  const auto historical = f.driver.portSnapshot(); f.take();
  f.command("current 1a5");
  f.write(8, 0x40); f.send("apply"); f.send("job step");
  CHECK(f.registers[8] == 0x40 && f.driver.portSnapshot().invalidated);
  f.command("cancel " + std::to_string(f.driver.progress().token.value));
  const auto cancelled = f.result();
  CHECK(cancelled.status.is(t::Err::CANCELLED) && cancelled.hardwareEffect == t::HardwareEffect::PARTIAL);
  CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  const auto port = f.driver.portSnapshot();
  CHECK(port.sequence == historical.sequence && port.rawValidMask == historical.rawValidMask && port.invalidated);
  CHECK(f.command("sample").find("invalidated=1") != std::string::npos);
  const auto take = f.command("result take " + std::to_string(cancelled.token.value));
  CHECK(take.find("state=CANCELLED code=CANCELLED") != std::string::npos);
  CHECK(take.find("command=result.take outcome=SUCCEEDED code=OK") != std::string::npos);
}

void failuresAndVariantGates() {
  for (bool readbackFailure : {false, true}) {
    Fixture f; f.manual(); f.initialize(); f.command("current 1a5");
    f.write(8, 0x40);
    if (readbackFailure) f.read(0);
    auto& failing = f.script.back();
    failing.status = t::Status::Error(t::Err::I2C_TIMEOUT, "script timeout", std::numeric_limits<int32_t>::min());
    f.send("apply"); f.finishManual();
    const auto result = f.result();
    CHECK(result.status.is(t::Err::I2C_TIMEOUT) && result.status.detail == std::numeric_limits<int32_t>::min());
    CHECK(!result.completedTimeValid);
    CHECK(result.hardwareEffect == (readbackFailure ? t::HardwareEffect::PARTIAL : t::HardwareEffect::INDETERMINATE));
    CHECK(f.registers[8] == 0x40 && f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
    const auto text = f.command("result");
    CHECK(text.find("detail=-2147483648 ") != std::string::npos);
    CHECK(text.find("configuration=UNKNOWN") != std::string::npos);
  }
  Fixture unsupported; unsupported.manual(); unsupported.initialize();
  const auto before = unsupported.calls;
  CHECK(unsupported.command("recover-dead-battery").find("code=UNSUPPORTED_FEATURE ") != std::string::npos);
  CHECK(unsupported.calls == before);
  Fixture recovery(t::Variant::TUSB320); recovery.manual(); recovery.initialize();
  recovery.read(0x45); recovery.write(0x45, 0x04);
  recovery.send("recover-dead-battery");
  CHECK(recovery.driver.progress().kind == t::OperationKind::RECOVER_DEAD_BATTERY);
  recovery.send("job step"); recovery.send("job step"); recovery.send("job step 0");
  CHECK(recovery.driver.progress().waiting && recovery.registers[0x45] == 4);
  const auto ready = recovery.driver.progress().readyAtMs;
  CHECK(ready == recovery.now + 31);
  recovery.now = ready - 1; recovery.send("job step 255"); CHECK(recovery.calls == recovery.script.size());
  recovery.write(0x45, 0); recovery.read(0x45); recovery.now = ready; recovery.finishManual();
  CHECK(recovery.result().status.ok() && recovery.registers[0x45] == 0);

  Fixture gate(t::Variant::TUSB322I); gate.manual(); gate.initialize();
  const auto initialCalls = gate.calls;
  CHECK(gate.command("irq ack").find("code=UNSUPPORTED_FEATURE ") != std::string::npos);
  CHECK(gate.command("irq service").find("code=UNSUPPORTED_FEATURE ") != std::string::npos);
  CHECK(gate.command("recover-dead-battery").find("code=UNSUPPORTED_FEATURE ") != std::string::npos);
  gate.command("current 1a5");
  CHECK(gate.command("apply").find("code=UNSUPPORTED_FEATURE ") != std::string::npos);
  CHECK(gate.calls == initialCalls && !gate.driver.configurationSnapshot().hasDesiredProfile);
  gate.command("current default"); gate.command("mode dfp");
  gate.write(10, 0x31); gate.write(10, 0x21); gate.write(10, 0x20); gate.proof();
  gate.send("apply"); gate.finishManual();
  CHECK(gate.result().status.ok() && gate.registers[10] == 0x20);
}
} // namespace

/// @brief Execute shared-source operation contracts; return failures for the CLI suite owner.
int runCliOperationTests() {
  admissionAndAliases();
  successfulReadOnlyRoutesAndReset();
  budgetsAndAutomaticProgress();
  immutableTimeAndLateSuccess();
  rejectedOwnerClockSamples();
  retainedResultsAndCorrelation();
  outputLossAndTakeReservation();
  irqAndApplyEffects();
  failuresAndVariantGates();
  std::printf("CLI operations: %u checks, %u failures\n", checks, failures);
  return static_cast<int>(failures);
}
