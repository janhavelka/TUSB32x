/// @file test_cli_fault_fuzz.cpp
/// @brief Seeded, bounded CLI mutations with transport failures and output backpressure.
/// @details This is a reproducible generated campaign, not coverage-guided fuzzing or silicon
/// qualification. Each reported seed replays one independent case with --seed N --cases 1.
/// Literal LAI bytes follow the existing CLI oracle (TI SLLSEQ8D Tables 8-10): current 1.5 A
/// is 08=40, DRP/168 ms is 0A=30 and 50 percent duty is 09=04. No device model is inferred.
#include "Tusb32xCli.h"
#include "../support/FakeTusb32xDevice.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

namespace {
namespace t = TUSB32x;
using Fake = tusb32x_test::FakeTusb32xDevice;
using Console = tusb32x_cli::ConsoleBuffer;
uint32_t caseSeed = 0;
const char* campaign = "setup";
uint64_t checks = 0;

void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    std::fprintf(stderr, "FAIL CLI fuzz seed=%lu campaign=%s line=%d: %s\n"
        "Replay: --seed %lu --cases 1\n", static_cast<unsigned long>(caseSeed), campaign,
        line, expression, static_cast<unsigned long>(caseSeed));
    std::exit(1);
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

// Fixed integer algorithm avoids implementation-dependent standard distribution behavior.
struct Random {
  uint32_t state;
  explicit Random(uint32_t seed) : state(seed == 0 ? UINT32_C(0x6D2B79F5) : seed) {}
  uint32_t next() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
  }
  std::size_t below(std::size_t bound) { return static_cast<std::size_t>(next()) % bound; }
};

struct Fixture {
  Fake bus;
  t::TUSB32x driver;
  t::DriverConfig config;
  tusb32x_cli::Platform platform;
  tusb32x_cli::Cli cli;
  std::string output;
  std::size_t sinkLimit = Console::DRAIN_BUDGET;
  std::size_t sinkCalls = 0;

  Fixture() : config(configuration(&bus)), platform(hooks(this)), cli(driver, config, platform) {
    CHECK(cli.setup().ok());
    CHECK(driver.bind(config).ok());
    proof(0);
    t::OperationToken token;
    CHECK(driver.startInitialize({100, 1100}, token).inProgress());
    for (unsigned turn = 0; turn < 16 && driver.progress().state == t::OperationState::ACTIVE;
         ++turn) {
      const auto before = bus.calls;
      driver.poll(token, 100);
      CHECK(bus.calls - before <= 1);
    }
    t::OperationResult result;
    CHECK(driver.takeResult(token, result).ok() && result.status.ok());
    CHECK(bus.complete());
    bus = Fake{};
    command("job auto 0");
    drain();
    output.clear();
  }
  static t::DriverConfig configuration(Fake* bus) {
    t::DriverConfig result;
    result.variant = t::Variant::TUSB320LAI;
    result.i2cUser = bus;
    result.i2cWrite = Fake::write;
    result.i2cWriteRead = Fake::writeRead;
    return result;
  }
  static uint64_t clock(void*) { return 100; }
  static std::size_t write(const char* bytes, std::size_t length, void* user) {
    auto& f = *static_cast<Fixture*>(user);
    ++f.sinkCalls;
    CHECK(length > 0 && length <= Console::DRAIN_BUDGET);
    const auto accepted = std::min(length, f.sinkLimit);
    f.output.append(bytes, accepted);
    return accepted;
  }
  static tusb32x_cli::Platform hooks(Fixture* f) {
    tusb32x_cli::Platform result;
    result.nowMs = clock;
    result.write = write;
    result.user = f;
    return result;
  }
  void proof(uint8_t current) {
    const uint8_t identity[] = {0x30, 0x32, 0x33, 0x42, 0x53, 0x55, 0x54, 0};
    auto& id = bus.expectRead(0x47, 0, 8);
    std::memcpy(id.rx, identity, sizeof(identity));
    bus.expectRead(0x47, 0xA0, 1).rx[0] = 2;
    bus.expectRead(0x47, 8, 1).rx[0] = current;
    bus.expectRead(0x47, 9, 1).rx[0] = 0x04;
    bus.expectRead(0x47, 10, 1).rx[0] = 0x30;
    bus.expectRead(0x47, 0x45, 1).rx[0] = 0;
  }
  void tick() {
    const auto before = bus.calls;
    const auto outputs = sinkCalls;
    cli.tick();
    CHECK(bus.calls - before <= 1);
    CHECK(sinkCalls - outputs <= 1);
    CHECK(cli.console().queued() <= Console::OUTPUT_CAPACITY);
  }
  void command(const std::string& line) {
    const auto before = bus.calls;
    CHECK(cli.processCommand(line.c_str()));
    CHECK(bus.calls == before); // These generated commands exclude the explicit raw Sync routes.
    tick();
  }
  void drain() {
    sinkLimit = Console::DRAIN_BUDGET;
    for (unsigned turn = 0; turn < 64 &&
         (cli.console().queued() != 0 || cli.console().lossPending()); ++turn) tick();
    CHECK(cli.console().queued() == 0 && !cli.console().lossPending());
  }
  void feed(const std::string& bytes, Random& random) {
    std::size_t offset = 0;
    for (unsigned turn = 0; turn < 1024 && offset < bytes.size(); ++turn) {
      std::size_t consumedThisTurn = 0;
      const auto calls = bus.calls;
      // Two fragments in a turn exercise the cumulative byte limit, including split CRLF.
      for (unsigned fragment = 0; fragment < 2 && offset < bytes.size(); ++fragment) {
        const auto offered = std::min(bytes.size() - offset, 1 + random.below(97));
        const auto consumed = cli.feed(bytes.data() + offset, offered);
        CHECK(consumed <= offered);
        offset += consumed;
        consumedThisTurn += consumed;
      }
      CHECK(consumedThisTurn <= Console::INPUT_BUDGET);
      CHECK(bus.calls == calls);
      sinkLimit = random.below(Console::DRAIN_BUDGET + 1);
      tick();
    }
    CHECK(offset == bytes.size());
    drain();
  }
  t::OperationResult result() {
    t::OperationResult value;
    CHECK(driver.resultSnapshot(driver.progress().token, value).ok());
    return value;
  }
};

// Every mutation has an independent invalidity witness; arbitrary edits might create a valid
// write and therefore cannot support the stronger zero-callback assertion used here.
std::string malformed(Random& random) {
  const char* bases[] = {"wreg 8 64", "apply", "reset", "irq ack", "init", "current 1a5"};
  std::string line = bases[random.below(sizeof(bases) / sizeof(*bases))];
  switch (random.below(8)) {
    case 0: {
      const char poison[] = {'\0', '\x01', '\x02', '\x0B', '\x1B', '\x1F', '\x80', '\xFF'};
      line.insert(random.below(line.size() + 1), 1, poison[random.below(sizeof(poison))]);
      break;
    }
    case 1:
      line.append(Console::LINE_CAPACITY + random.below(160), ' ');
      break;
    case 2:
      line.insert(line.find(' ') == std::string::npos ? line.size() : line.find(' '), "!");
      break;
    case 3:
      for (unsigned i = 0; i < 13; ++i) line += " x";
      break;
    case 4:
      line = std::string("wreg 8").substr(0, 1 + random.below(6));
      break;
    case 5:
      line = "wreg 8 " + std::string(20 + random.below(50), '9');
      break;
    case 6:
      line = "wreg 8 0x" + std::string(1 + random.below(16), 'F') + "z";
      break;
    default:
      line.insert(0, 1, '\0');
      line.insert(1, 1 + random.below(32), '\b'); // Editing cannot repair poison.
      break;
  }
  line += random.below(2) == 0 ? "\n" : "\r\n";
  return line;
}

void parserMutations(Random& random) {
  campaign = "malformed-input";
  Fixture f;
  const auto generation = f.cli.stagedGeneration();
  const auto configuration = f.driver.configurationSnapshot();
  f.feed(malformed(random), random);
  CHECK(f.bus.calls == 0 && f.bus.errors == 0);
  CHECK(f.cli.stagedGeneration() == generation);
  CHECK(f.driver.progress().state == t::OperationState::IDLE);
  CHECK(f.driver.configurationSnapshot().state == configuration.state);
  CHECK(f.driver.configurationSnapshot().verifiedGeneration == configuration.verifiedGeneration);
  CHECK(f.output.find("code=INVALID_PARAM ") != std::string::npos);
  for (const auto* record : {"TUSB ACK ", "TUSB END "}) {
    const auto at = f.output.find(record);
    CHECK(at != std::string::npos && f.output.find(record, at + 1) == std::string::npos);
  }
  // A following valid fragmented command proves framing recovered after the rejected line.
  f.feed("current 1a5\r\n", random);
  CHECK(f.cli.stagedProfile().sourceCurrent == t::SourceCurrent::MA_1500);
  CHECK(f.cli.stagedGeneration() == generation + 1);
  CHECK(f.bus.calls == 0 && f.bus.errors == 0);
}

void transportAndOutputFaults(Random& random) {
  campaign = "transport-output";
  Fixture f;
  f.command("current 1a5");
  f.drain();
  auto& write = f.bus.expectWrite(0x47, 8, 0x40);
  f.proof(0x40);
  const auto failAt = random.below(f.bus.stepCount);
  // Only a failed write may or may not have acted; successful callbacks apply the literal byte.
  write.applyWrite = failAt != 0 || random.below(2) != 0;
  const bool acceptedWrite = write.applyWrite;
  const t::Err errors[] = {t::Err::I2C_ERROR, t::Err::I2C_TIMEOUT, t::Err::I2C_NACK_ADDR,
      t::Err::I2C_NACK_DATA, t::Err::I2C_BUS, t::Err::I2C_BUSY};
  const auto code = errors[random.below(sizeof(errors) / sizeof(*errors))];
  const auto detail = std::numeric_limits<int32_t>::min() +
      static_cast<int32_t>(random.next() & UINT32_C(0x7FFFFFFF));
  auto& failure = f.bus.script[failAt];
  failure.status = t::Status::Error(code, "seeded callback failure", detail);
  if (failure.kind == Fake::Kind::READ)
    failure.fillLength = random.below(failure.rxLength + 1);
  f.bus.stepCount = failAt + 1;
  f.sinkLimit = 0;
  f.command("apply");
  const auto token = f.driver.progress().token;
  CHECK(f.driver.progress().state == t::OperationState::ACTIVE && f.bus.calls == 0);
  for (unsigned turn = 0; turn < 32 && f.driver.progress().state == t::OperationState::ACTIVE;
       ++turn) {
    const auto calls = f.bus.calls;
    f.command("job step 0");
    CHECK(f.bus.calls == calls);
    f.sinkLimit = random.below(4) == 0 ? 0 : 1 + random.below(256);
    f.command("job step " + std::to_string(1 + random.below(255)));
  }
  CHECK(f.bus.complete());
  const auto retained = f.result();
  CHECK(retained.status.is(code) && retained.status.detail == detail);
  CHECK(retained.transfers == failAt + 1);
  CHECK(retained.hardwareEffect == (failAt == 0 ? t::HardwareEffect::INDETERMINATE
                                               : t::HardwareEffect::PARTIAL));
  CHECK(f.driver.configurationSnapshot().state == t::ConfigurationState::UNKNOWN);
  CHECK(f.bus.registers[8] == (acceptedWrite ? 0x40 : 0));
  f.drain();
  f.output.clear();

  // Vary free space below the documented five maximum-record take reservation. Include a
  // completely full queue; response loss must still leave the exact retained result intact.
  f.sinkLimit = 0;
  const std::size_t freeSpace = random.below(5 * (Console::RECORD_CAPACITY - 1));
  const std::string record(Console::RECORD_CAPACITY - 2, 'A');
  for (unsigned i = 0; i < 32 && f.cli.console().available() > freeSpace; ++i) {
    const auto length = std::min(Console::RECORD_CAPACITY - 1,
        f.cli.console().available() - freeSpace);
    const auto text = record.substr(0, length - 1) + "\n";
    CHECK(f.cli.enqueueApplicationRecord(text.data(), text.size()));
  }
  CHECK(f.cli.console().available() == freeSpace);
  const auto attempts = f.bus.calls;
  f.command("result take " + std::to_string(token.value));
  CHECK(f.driver.progress().resultPending);
  const auto after = f.result();
  CHECK(after.token.value == retained.token.value && after.status.code == retained.status.code);
  CHECK(after.status.detail == detail && after.hardwareEffect == retained.hardwareEffect);
  CHECK(f.bus.calls == attempts && f.bus.complete());
  f.drain();
  f.output.clear();
  f.command("result take " + std::to_string(token.value));
  f.drain();
  CHECK(!f.driver.progress().resultPending && f.bus.calls == attempts);
  CHECK(f.output.find("detail=" + std::to_string(detail) + " ") != std::string::npos);
  CHECK(f.output.find("command=result.take outcome=SUCCEEDED code=OK") != std::string::npos);
}

bool number(const char* text, uint32_t& value) {
  if (*text == '\0') return false;
  uint32_t parsed = 0;
  for (const char* at = text; *at != '\0'; ++at) {
    if (*at < '0' || *at > '9') return false;
    const auto digit = static_cast<uint32_t>(*at - '0');
    if (parsed > (std::numeric_limits<uint32_t>::max() - digit) / 10U) return false;
    parsed = parsed * 10U + digit;
  }
  value = parsed;
  return true;
}
}  // namespace

int main(int argc, char** argv) {
  uint32_t seed = UINT32_C(0x320C11);
  uint32_t cases = 256;
  for (int i = 1; i < argc; i += 2) {
    if (i + 1 >= argc ||
        (std::strcmp(argv[i], "--seed") != 0 && std::strcmp(argv[i], "--cases") != 0) ||
        !number(argv[i + 1], std::strcmp(argv[i], "--seed") == 0 ? seed : cases)) {
      std::fprintf(stderr, "Usage: %s [--seed uint32] [--cases 1..100000]\n", argv[0]);
      return 2;
    }
  }
  if (cases == 0 || cases > 100000) return 2;
  for (uint32_t index = 0; index < cases; ++index) {
    caseSeed = seed + index;
    Random random(caseSeed);
    parserMutations(random);
    transportAndOutputFaults(random);
  }
  std::printf("CLI fault fuzz PASS seed=%lu cases=%lu checks=%llu\n",
      static_cast<unsigned long>(seed), static_cast<unsigned long>(cases),
      static_cast<unsigned long long>(checks));
  return 0;
}
