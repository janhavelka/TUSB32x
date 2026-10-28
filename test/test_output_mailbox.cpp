/// @file test_output_mailbox.cpp
/// @brief Exercise output stalls, partial writes and owner offers during a worker callback.
#include "OutputMailbox.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <type_traits>

namespace {
unsigned checks = 0;
unsigned failures = 0;
#define CHECK(condition) do { \
  ++checks; \
  if (!(condition)) { \
    std::printf("FAIL line %d: %s\n", __LINE__, #condition); \
    ++failures; \
  } \
} while (false)

using tusb32x_cli::OutputMailbox;
static_assert(!std::is_copy_constructible<OutputMailbox>::value, "Ownership cannot be copied");
static_assert(!std::is_move_constructible<OutputMailbox>::value, "Active storage cannot move");

struct Sink {
  std::size_t allowance = OutputMailbox::CAPACITY;
  std::size_t calls = 0;
  std::size_t largest = 0;
  std::string bytes;
  bool overreport = false;
  OutputMailbox* offerDuringWrite = nullptr;

  static std::size_t write(const char* data, std::size_t length, void* context) {
    auto& sink = *static_cast<Sink*>(context);
    ++sink.calls;
    sink.largest = std::max(sink.largest, length);
    CHECK(length != 0 && length <= OutputMailbox::CAPACITY);
    if (sink.offerDuringWrite) {
      // Simulate a suspended SDK call while the independent owner tries to drain again.
      // The offer must reject immediately without altering bytes borrowed by this call.
      const std::string before(data, length);
      for (unsigned turn = 0; turn < 20; ++turn) {
        CHECK(sink.offerDuringWrite->offer("new output", 10) == 0);
        CHECK(std::string(data, length) == before);
      }
    }
    const std::size_t accepted = std::min(length, sink.allowance);
    sink.bytes.append(data, accepted);
    return sink.overreport ? length + 1 : accepted;
  }
};

void emptyAndInvalidInput() {
  OutputMailbox mailbox;
  Sink sink;
  CHECK(mailbox.drain(Sink::write, &sink) == 0);
  CHECK(sink.calls == 0);
  CHECK(mailbox.offer(nullptr, 1) == 0);
  CHECK(mailbox.offer("", 0) == 0);
  CHECK(mailbox.drain(Sink::write, &sink) == 0);
  CHECK(sink.calls == 0);

  char source[] = "abc";
  CHECK(mailbox.offer(source, 3) == 3);
  source[0] = 'X';
  CHECK(mailbox.offer("other", 5) == 0);
  CHECK(mailbox.drain(nullptr, &sink) == 0);
  CHECK(mailbox.offer("other", 5) == 0);
  CHECK(mailbox.drain(Sink::write, &sink) == 3);
  CHECK(sink.calls == 1 && sink.bytes == "abc");
  CHECK(mailbox.drain(Sink::write, &sink) == 0);
  CHECK(sink.calls == 1);
}

void stalledWorkerAndPartialOrder() {
  OutputMailbox mailbox;
  Sink sink;
  sink.allowance = 0;
  sink.offerDuringWrite = &mailbox;
  CHECK(mailbox.offer("abcdef", 6) == 6);
  for (unsigned turn = 0; turn < 3; ++turn) {
    const std::size_t callsBefore = sink.calls;
    CHECK(mailbox.drain(Sink::write, &sink) == 0);
    CHECK(sink.calls == callsBefore + 1);
    CHECK(mailbox.offer("later", 5) == 0);
    CHECK(sink.bytes.empty());
  }
  sink.allowance = 2;
  CHECK(mailbox.drain(Sink::write, &sink) == 2);
  CHECK(sink.bytes == "ab");
  CHECK(mailbox.offer("later", 5) == 0);
  CHECK(mailbox.drain(Sink::write, &sink) == 2);
  CHECK(sink.bytes == "abcd");
  CHECK(mailbox.drain(Sink::write, &sink) == 2);
  CHECK(sink.bytes == "abcdef");
  CHECK(mailbox.offer("ghi", 3) == 3);
  sink.allowance = OutputMailbox::CAPACITY;
  CHECK(mailbox.drain(Sink::write, &sink) == 3);
  CHECK(sink.bytes == "abcdefghi");
  CHECK(sink.calls == 7);
  CHECK(sink.largest == 6);
}

void maximumPrefixAndRepeatedHandoffs() {
  OutputMailbox mailbox;
  Sink sink;
  char source[OutputMailbox::CAPACITY + 17];
  std::memset(source, 'x', sizeof(source));
  CHECK(mailbox.offer(source, sizeof(source)) == OutputMailbox::CAPACITY);
  CHECK(mailbox.drain(Sink::write, &sink) == OutputMailbox::CAPACITY);
  CHECK(sink.calls == 1 && sink.largest == OutputMailbox::CAPACITY);
  CHECK(sink.bytes == std::string(OutputMailbox::CAPACITY, 'x'));
  CHECK(mailbox.offer(source + OutputMailbox::CAPACITY, 17) == 17);
  CHECK(mailbox.drain(Sink::write, &sink) == 17);
  CHECK(sink.bytes == std::string(sizeof(source), 'x'));

  for (unsigned turn = 0; turn < 64; ++turn) {
    CHECK(mailbox.offer("z", 1) == 1);
    CHECK(mailbox.drain(Sink::write, &sink) == 1);
  }
  CHECK(sink.bytes == std::string(sizeof(source), 'x') + std::string(64, 'z'));
  CHECK(sink.calls == 66);
}

void invalidBackendCountIsClamped() {
  OutputMailbox mailbox;
  Sink sink;
  sink.overreport = true;
  CHECK(mailbox.offer("abc", 3) == 3);
  CHECK(mailbox.drain(Sink::write, &sink) == 3);
  CHECK(mailbox.drain(Sink::write, &sink) == 0);
  CHECK(mailbox.offer("d", 1) == 1);
  CHECK(mailbox.drain(Sink::write, &sink) == 1);
  CHECK(sink.calls == 2 && sink.bytes == "abcd");
}

void producerProgressWhileWorkerIsBlocked() {
  struct BlockingSink {
    std::atomic<bool> entered{false};
    std::atomic<bool> release{false};
    bool timedOut = false;
    std::string bytes;

    static std::size_t write(const char* data, std::size_t length, void* context) {
      auto& sink = *static_cast<BlockingSink*>(context);
      sink.entered.store(true, std::memory_order_release);
      // Even a failing test releases its worker within this bound; it cannot hang forever.
      const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
      while (!sink.release.load(std::memory_order_acquire)) {
        if (std::chrono::steady_clock::now() >= deadline) {
          sink.timedOut = true;
          return 0;
        }
        std::this_thread::yield();
      }
      const std::size_t accepted = std::min(length, std::size_t{2});
      sink.bytes.append(data, accepted);
      return accepted;
    }
  } sink;

  OutputMailbox mailbox;
  CHECK(mailbox.offer("abcdef", 6) == 6);
  std::size_t workerAccepted = 0;
  std::thread worker([&] { workerAccepted = mailbox.drain(BlockingSink::write, &sink); });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!sink.entered.load(std::memory_order_acquire) &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::yield();
  }
  CHECK(sink.entered.load(std::memory_order_acquire));
  unsigned independentOwnerTurns = 0;
  for (unsigned turn = 0; turn < 1024; ++turn) {
    CHECK(mailbox.offer("later", 5) == 0);
    ++independentOwnerTurns;
  }
  CHECK(independentOwnerTurns == 1024);
  sink.release.store(true, std::memory_order_release);
  worker.join();
  CHECK(!sink.timedOut && workerAccepted == 2 && sink.bytes == "ab");
  CHECK(mailbox.offer("later", 5) == 0);

  // Consumer ownership changes only after join; the remaining bytes retain their order.
  Sink rest;
  CHECK(mailbox.drain(Sink::write, &rest) == 4);
  CHECK(rest.bytes == "cdef");
  CHECK(mailbox.offer("ghi", 3) == 3);
  CHECK(mailbox.drain(Sink::write, &rest) == 3);
  CHECK(sink.bytes + rest.bytes == "abcdefghi");
}
} // namespace

int main() {
  emptyAndInvalidInput();
  stalledWorkerAndPartialOrder();
  maximumPrefixAndRepeatedHandoffs();
  invalidBackendCountIsClamped();
  producerProgressWhileWorkerIsBlocked();
  std::printf("Output mailbox: %u checks, %u failures\n", checks, failures);
  return failures == 0 ? 0 : 1;
}
