/// @file test_console.cpp
/// @brief Behavioral input framing, poison, queue loss and nonblocking drain boundaries.
#include "ConsoleBuffer.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

namespace {
using Console = tusb32x_cli::ConsoleBuffer;
int failures = 0;
std::size_t checks = 0;
void check(bool condition, const char* expression, int line) {
  ++checks;
  if (!condition) {
    if (failures < 40) std::printf("FAIL console line %d: %s\n", line, expression);
    ++failures;
  }
}
#define CHECK(expression) check((expression), #expression, __LINE__)

// The backend owns its capacity decision; a zero result models a disconnected host.
struct Sink {
  std::string bytes;
  std::size_t calls = 0;
  std::size_t maximumOffer = 0;
  std::size_t lastOffer = 0;
  std::size_t acceptLimit = Console::DRAIN_BUDGET;
  bool invalidCount = false;

  static std::size_t write(const char* data, std::size_t length, void* context) {
    auto& sink = *static_cast<Sink*>(context);
    ++sink.calls;
    sink.lastOffer = length;
    if (length > sink.maximumOffer) sink.maximumOffer = length;
    CHECK(length != 0);
    CHECK(length <= Console::DRAIN_BUDGET);
    const std::size_t accepted = length < sink.acceptLimit ? length : sink.acceptLimit;
    sink.bytes.append(data, accepted);
    return sink.invalidCount ? length + 1 : accepted;
  }
};

void feedAll(Console& console, const std::string& text, std::size_t fragment = 64) {
  std::size_t offset = 0;
  while (offset < text.size()) {
    const std::size_t remaining = text.size() - offset;
    const std::size_t count = remaining < fragment ? remaining : fragment;
    const std::size_t consumed = console.feed(text.data() + offset, count);
    CHECK(consumed != 0);
    CHECK(consumed <= Console::INPUT_BUDGET);
    if (consumed == 0) break;
    offset += consumed;
  }
  CHECK(offset == text.size());
}

void drainAll(Console& console, Sink& sink) {
  // A finite test bound catches lost-progress bugs instead of hanging the suite.
  for (std::size_t turn = 0; turn < Console::OUTPUT_CAPACITY + 128 &&
       (console.queued() != 0 || console.lossPending()); ++turn) {
    const std::size_t previousCalls = sink.calls;
    const std::size_t accepted = console.drain(Sink::write, &sink);
    CHECK(sink.calls == previousCalls + 1);
    CHECK(accepted != 0);
    CHECK(accepted <= Console::DRAIN_BUDGET);
  }
  CHECK(console.queued() == 0);
  CHECK(!console.lossPending());
}

std::string record(std::size_t length, char payload = 'x') {
  std::string result(length, payload);
  if (!result.empty()) result.back() = '\n';
  return result;
}

void initialAndInputBudgets() {
  static_assert(Console::LINE_CAPACITY == 192);
  static_assert(Console::INPUT_BUDGET == 64);
  static_assert(Console::OUTPUT_CAPACITY == 8192);
  static_assert(Console::RECORD_CAPACITY == 512);
  static_assert(Console::DRAIN_BUDGET == 256);
  static_assert(std::is_same_v<Console::WriteFn,
      std::size_t (*)(const char*, std::size_t, void*)>);
  Console console;
  CHECK(!console.lineReady());
  CHECK(!console.linePoisoned());
  CHECK(std::strcmp(console.line(), "") == 0);
  CHECK(console.queued() == 0);
  CHECK(console.available() == 8192);
  CHECK(console.droppedRecords() == 0);
  CHECK(console.droppedBytes() == 0);
  CHECK(!console.lossPending());
  CHECK(console.feed(nullptr, 99) == 0);
  CHECK(console.feed("a", 0) == 0);
  const std::string input(70, 'a');
  CHECK(console.feed(input.data(), input.size()) == 64);
  CHECK(std::strlen(console.line()) == 64);
  console.consumeLine();
  CHECK(std::strlen(console.line()) == 64);  // Incomplete input cannot be silently discarded.
  CHECK(console.feed("\nsecond\n", 8) == 1);
  CHECK(console.lineReady());
  CHECK(console.feed("second\n", 7) == 0);
  CHECK(std::strlen(console.line()) == 64);
  console.consumeLine();
  CHECK(console.feed("second\n", 7) == 7);
  CHECK(std::strcmp(console.line(), "second") == 0);
}

void terminatorsAndFragmentation() {
  for (std::size_t fragment = 1; fragment <= Console::INPUT_BUDGET; ++fragment) {
    Console console;
    feedAll(console, "profile\tshow\r", fragment);
    CHECK(console.lineReady());
    CHECK(!console.linePoisoned());
    CHECK(std::strcmp(console.line(), "profile\tshow") == 0);
    console.consumeLine();
    // The trailing LF is ignored even after consumption and across feed calls.
    CHECK(console.feed("\n", 1) == 1);
    CHECK(!console.lineReady());
    feedAll(console, "version\n", fragment);
    CHECK(console.lineReady());
    CHECK(std::strcmp(console.line(), "version") == 0);
  }
  Console console;
  CHECK(console.feed("\r\n\r\n\n", 5) == 1);
  console.consumeLine();
  CHECK(console.feed("\n\r\n\n", 4) == 2);
  CHECK(console.lineReady());
  CHECK(std::strcmp(console.line(), "") == 0);
  console.consumeLine();
  CHECK(console.feed("\n\n", 2) == 2);
  CHECK(console.lineReady());
  console.consumeLine();
  CHECK(console.feed("\ra\n", 3) == 1);
  console.consumeLine();
  CHECK(console.feed("a\n", 2) == 2);  // CR not followed by LF preserves the next byte.
  CHECK(std::strcmp(console.line(), "a") == 0);
}

void editingAndCapacity() {
  Console console;
  CHECK(console.feed("\b\177ab\bcd\177\t!\n", 11) == 11);
  CHECK(console.lineReady());
  CHECK(!console.linePoisoned());
  CHECK(std::strcmp(console.line(), "ac\t!") == 0);
  for (std::size_t length : {std::size_t{0}, std::size_t{1}, std::size_t{190},
       std::size_t{191}, std::size_t{192}, std::size_t{193}, std::size_t{1000}}) {
    for (std::size_t fragment : {std::size_t{1}, std::size_t{17}, std::size_t{64}}) {
      Console bounded;
      feedAll(bounded, std::string(length, 'a') + '\n', fragment);
      CHECK(bounded.lineReady());
      CHECK(bounded.linePoisoned() == (length >= Console::LINE_CAPACITY));
      CHECK(std::strlen(bounded.line()) == (length < 192 ? length : 191));
      bounded.consumeLine();
      CHECK(!bounded.linePoisoned());
      CHECK(!bounded.lineReady());
      CHECK(std::strcmp(bounded.line(), "") == 0);
    }
  }
  Console full;
  feedAll(full, std::string(191, 'a'));
  CHECK(!full.linePoisoned());
  CHECK(full.feed("\bZ\n", 3) == 3);
  CHECK(!full.linePoisoned());
  CHECK(std::strlen(full.line()) == 191);
  CHECK(full.line()[190] == 'Z');

  Console overflow;
  feedAll(overflow, std::string(192, 'a'));
  CHECK(overflow.linePoisoned());
  feedAll(overflow, std::string(192, '\b') + "bind\n");
  CHECK(overflow.linePoisoned());
  CHECK(std::strlen(overflow.line()) == 191);
}

void everyInputByteAndStickyPoison() {
  for (unsigned value = 0; value <= 255; ++value) {
    if (value == '\r' || value == '\n') continue;
    Console console;
    std::string input = "a";
    input.push_back(static_cast<char>(value));
    input += "\bbind\n";
    feedAll(console, input, 1);
    const bool valid = value == 8 || value == 9 || value == 127 ||
        (value >= 32 && value <= 126);
    CHECK(console.lineReady());
    CHECK(console.linePoisoned() == !valid);
    if (valid) CHECK(std::strcmp(console.line(), value == 8 || value == 127 ? "bind" :
        "abind") == 0);
    console.consumeLine();
    feedAll(console, "end\n");
    CHECK(!console.linePoisoned());
    CHECK(std::strcmp(console.line(), "end") == 0);
  }
}

void outputFramingAndCopies() {
  Console console;
  Sink sink;
  CHECK(console.drain(Sink::write, &sink) == 0);
  CHECK(sink.calls == 0);
  char borrowed[] = "first\nsecond\n";
  CHECK(console.enqueue(borrowed, sizeof(borrowed) - 1));
  borrowed[0] = 'X';
  CHECK(console.drain(nullptr, nullptr) == 0);
  CHECK(console.queued() == sizeof(borrowed) - 1);
  drainAll(console, sink);
  CHECK(sink.bytes == "first\nsecond\n");
  CHECK(sink.bytes.find('\0') == std::string::npos);
  CHECK(console.enqueue("\n", 1));
  const std::string maximum = record(511);
  CHECK(console.enqueue(maximum.data(), maximum.size()));
  const std::size_t savedQueued = console.queued();
  const char embedded[] = {'a', '\0', '\n'};
  CHECK(!console.enqueue(nullptr, 4));
  CHECK(!console.enqueue("", 0));
  CHECK(!console.enqueue("missing newline", 15));
  CHECK(!console.enqueue(embedded, sizeof(embedded)));
  const std::string oversized = record(512);
  CHECK(!console.enqueue(oversized.data(), oversized.size()));
  CHECK(console.queued() == savedQueued);
  CHECK(console.droppedRecords() == 5);
  CHECK(console.droppedBytes() == 4 + 15 + 3 + 512);
  CHECK(console.lossPending());
  drainAll(console, sink);
  CHECK(sink.bytes == std::string("first\nsecond\n\n") + maximum +
      "TUSB LOSS schema=1 dropped_records=5 dropped_bytes=534\n");
}

void shortZeroAndInvalidWrites() {
  Console console;
  Sink sink;
  const std::string payload = record(511);
  CHECK(console.enqueue(payload.data(), payload.size()));
  sink.acceptLimit = 0;
  for (int turn = 0; turn < 12; ++turn) {
    const std::size_t before = sink.calls;
    CHECK(console.drain(Sink::write, &sink) == 0);
    CHECK(sink.calls == before + 1);
    CHECK(console.queued() == 511);
  }
  sink.acceptLimit = 1;
  CHECK(console.drain(Sink::write, &sink) == 1);
  CHECK(console.queued() == 510);
  sink.acceptLimit = 13;
  drainAll(console, sink);
  CHECK(sink.bytes == payload);
  CHECK(sink.maximumOffer == 256);
  CHECK(console.droppedRecords() == 0);

  Console invalid;
  Sink invalidSink;
  invalidSink.invalidCount = true;
  CHECK(invalid.enqueue("abc\n", 4));
  CHECK(invalid.drain(Sink::write, &invalidSink) == 4);
  CHECK(invalid.queued() == 0);
  CHECK(invalidSink.calls == 1);
  CHECK(invalidSink.bytes == "abc\n");
}

void fullQueueAtomicLossAndWrap() {
  Console console;
  Sink sink;
  std::string expected;
  const std::string maximum = record(511, 'm');
  for (int i = 0; i < 16; ++i) {
    CHECK(console.enqueue(maximum.data(), maximum.size()));
    expected += maximum;
  }
  const std::string final = record(16, 'f');
  CHECK(console.enqueue(final.data(), final.size()));
  expected += final;
  CHECK(console.queued() == 8192);
  CHECK(console.available() == 0);
  CHECK(!console.enqueue("lost\n", 5));
  CHECK(!console.enqueue("also lost\n", 10));
  CHECK(console.queued() == 8192);
  CHECK(console.droppedRecords() == 2);
  CHECK(console.droppedBytes() == 15);
  sink.acceptLimit = 0;
  for (int turn = 0; turn < 20; ++turn) {
    CHECK(console.drain(Sink::write, &sink) == 0);
    CHECK(console.lossPending());
    CHECK(console.droppedRecords() == 2);
    CHECK(console.droppedBytes() == 15);
  }
  sink.acceptLimit = 256;
  const std::size_t callsBefore = sink.calls;
  CHECK(console.drain(Sink::write, &sink) == 256);
  CHECK(sink.calls == callsBefore + 1);
  CHECK(!console.lossPending());  // Queue admission, rather than backend delivery, clears it.
  const std::string marker = "TUSB LOSS schema=1 dropped_records=2 dropped_bytes=15\n";
  CHECK(console.queued() == 8192 - 256 + marker.size());
  drainAll(console, sink);
  CHECK(sink.bytes == expected + marker);
  CHECK(console.droppedRecords() == 2);

  // Repeated records alternate producer/consumer positions across the circular-buffer end.
  Console wrapped;
  Sink wrappedSink;
  wrappedSink.acceptLimit = 77;
  std::string allRecords;
  for (std::size_t index = 0; index < 1000; ++index) {
    const std::string next = record(17 + (index % 160), static_cast<char>('a' + index % 26));
    while (wrapped.available() < next.size()) {
      CHECK(wrapped.drain(Sink::write, &wrappedSink) != 0);
    }
    CHECK(wrapped.enqueue(next.data(), next.size()));
    allRecords += next;
    CHECK(wrapped.drain(Sink::write, &wrappedSink) != 0);
    CHECK(wrapped.queued() + wrapped.available() == 8192);
  }
  drainAll(wrapped, wrappedSink);
  CHECK(wrappedSink.bytes == allRecords);
  CHECK(wrapped.droppedRecords() == 0);
}

void markerInsertionAndCumulativeEvidence() {
  Console console;
  Sink sink;
  CHECK(!console.enqueue("bad", 3));
  CHECK(console.lossPending());
  CHECK(console.drain(nullptr, nullptr) == 0);
  CHECK(!console.lossPending());
  CHECK(console.queued() != 0);
  sink.acceptLimit = 0;
  CHECK(console.drain(Sink::write, &sink) == 0);
  CHECK(!console.lossPending());
  CHECK(!console.enqueue("again", 5));
  CHECK(console.lossPending());
  CHECK(console.drain(Sink::write, &sink) == 0);
  CHECK(!console.lossPending());
  sink.acceptLimit = 256;
  drainAll(console, sink);
  CHECK(sink.bytes == "TUSB LOSS schema=1 dropped_records=1 dropped_bytes=3\n"
      "TUSB LOSS schema=1 dropped_records=2 dropped_bytes=8\n");

  // Input rejection and pending input cannot prevent output progress or consume loss evidence.
  feedAll(console, "bind\n");
  CHECK(console.enqueue("ready\n", 6));
  CHECK(console.drain(Sink::write, &sink) == 6);
  CHECK(console.lineReady());
  CHECK(std::strcmp(console.line(), "bind") == 0);
  CHECK(console.droppedRecords() == 2);
}

void saturationWithoutPublicStateSeeds() {
  using tusb32x_cli::console_detail::saturatingAdd;
  constexpr uint32_t maximum = std::numeric_limits<uint32_t>::max();
  static_assert(saturatingAdd(0, 0) == 0);
  static_assert(saturatingAdd(maximum - 1, 1) == maximum);
  static_assert(saturatingAdd(maximum - 1, 2) == maximum);
  static_assert(saturatingAdd(maximum, 0) == maximum);
  static_assert(saturatingAdd(maximum, 1) == maximum);
  CHECK(saturatingAdd(0, std::numeric_limits<std::size_t>::max()) == maximum);
  CHECK(saturatingAdd(maximum - 7, 6) == maximum - 1);
  CHECK(saturatingAdd(maximum - 7, 7) == maximum);
  CHECK(saturatingAdd(maximum - 7, 8) == maximum);
  Console console;
  CHECK(!console.enqueue("not dereferenced", std::numeric_limits<std::size_t>::max()));
  CHECK(console.droppedRecords() == 1);
  CHECK(console.droppedBytes() == maximum);
  CHECK(!console.enqueue("bad", 3));
  CHECK(console.droppedBytes() == maximum);
  CHECK(console.droppedRecords() == 2);
  Sink sink;
  drainAll(console, sink);
  CHECK(sink.bytes == "TUSB LOSS schema=1 dropped_records=2 dropped_bytes=4294967295\n");
  CHECK(sink.bytes.size() < Console::RECORD_CAPACITY);
}
}  // namespace

int main() {
  initialAndInputBudgets();
  terminatorsAndFragmentation();
  editingAndCapacity();
  everyInputByteAndStickyPoison();
  outputFramingAndCopies();
  shortZeroAndInvalidWrites();
  fullQueueAtomicLossAndWrap();
  markerInsertionAndCumulativeEvidence();
  saturationWithoutPublicStateSeeds();
  std::printf("console: %zu checks, %d failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
