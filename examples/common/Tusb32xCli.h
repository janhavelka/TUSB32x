/// @file Tusb32xCli.h
/// @brief Application-owned CLI, fixed staging/records and core operation flow; no extra engine.
#pragma once

#include <TUSB32x/TUSB32x.h>

#include <cstddef>
#include <cstdint>

#include "ConsoleBuffer.h"

/// @brief Portable example console; the application serializes access and owns all borrowed hooks.
namespace tusb32x_cli {

/// @brief Application-reported callback counts, never inferred physical transactions or bytes.
struct AdapterCounters {
  bool available = false; ///< False means neither number is evidence.
  uint32_t writes = 0; ///< Saturating adapter write-callback count, if available.
  uint32_t writeReads = 0; ///< Saturating adapter combined-callback count, if available.
};

/// @brief Borrowed stable firmware tokens; unknown host identity is never flashed-firmware proof.
struct FirmwareIdentity {
  const char* git = "unknown"; ///< ASCII token, 1..40 bytes; firmware's own revision.
  const char* tree = "unknown"; ///< ASCII token, 1..8 bytes, normally clean/dirty/unknown.
  const char* build = "unknown"; ///< ASCII token, 1..64 bytes, supplied by the firmware build.
  const char* framework = "unknown"; ///< ASCII token, 1..32 bytes.
  const char* frameworkVersion = "unknown"; ///< ASCII token, 1..32 bytes.
  const char* target = "unknown"; ///< ASCII token, 1..32 bytes; no default board claim.
};

/// @brief Borrowed nonblocking platform functions and cached adapter configuration; no GPIO setup.
/// @details All functions run synchronously in the serialized owner. They must not retain buffers,
/// reenter Cli or perform hidden I2C. Identity/string storage lives as long as Cli. Output returns
/// accepted prefix length (zero is backpressure), never blocks. Time is owner uint64 milliseconds.
struct Platform {
  uint64_t (*nowMs)(void*) = nullptr; ///< Required fresh monotonic owner time; modulo uint64 domain.
  ConsoleBuffer::WriteFn write = nullptr; ///< Required nonblocking output; at most 256 offered per tick.
  bool (*readInput)(bool&, void*) = nullptr; ///< Optional pin reader; unused by this local subset.
  AdapterCounters (*counters)(void*) = nullptr; ///< Optional cached adapter counters; zero I2C.
  TUSB32x::Status (*validateTarget)(const TUSB32x::DriverConfig&, void*) = nullptr;
      ///< Optional bus-silent borrowed-handle/address validation before target edit or bind.
      ///< Return terminal Status with static message. Invalid/progress codes become INVALID_CONFIG,
      ///< preserving detail; human messages over 255 bytes or with controls are replaced safely.
  void* user = nullptr; ///< Borrowed platform context; independent of the core transport context.
  FirmwareIdentity identity{}; ///< Actual application identity; no Git lookup or generated host stamp.
  const char* bus = "unknown"; ///< Cached ASCII bus token, 1..32 bytes.
  const char* framing = "unknown"; ///< Cached ASCII framing token, 1..32 bytes; no measured claim.
  uint32_t busHz = 0; ///< Cached bus clock in hertz; zero is unknown.
  int32_t sdaPin = -1; ///< Cached application pin; negative means unknown, never configured here.
  int32_t sclPin = -1; ///< Cached application pin; negative means unknown.
  int32_t irqPin = -1; ///< Cached optional input pin; negative means unknown.
};

/// @brief One implemented command row, shared by executable help and behavioral route checks.
struct CommandRow {
  const char* command; ///< Fixed canonical machine ID; aliases never become different actions.
  const char* aliases; ///< Comma-separated aliases, or none; no arbitrary alias expansion.
  const char* topic; ///< Implemented help section, lowercase ASCII.
  const char* syntax; ///< Complete implemented syntax only, without the human padding field.
  const char* execution; ///< CACHE_ONLY, PURE or CORE_JOB; no raw/framework handlers yet.
  const char* effect; ///< NONE, LOCAL_STATE, READ_OBSERVATION or DEVICE_WRITE.
  const char* description; ///< Static human explanation including cached/stage effect label.
  const char* prerequisites = "none"; ///< Local admission/evidence prerequisite, no startup action.
  const char* capability = "all"; ///< Required value/control capability, checked before staging.
  const char* records = "ACK,END"; ///< Payload types; exact required keys are frozen in host schemas.
  uint8_t maxCallbacks = 0; ///< Maximum callbacks per owner poll; command admission remains silent.
  uint8_t txBytes = 0; ///< Maximum TX bytes per callback, not a whole-operation transaction count.
  uint8_t rxBytes = 0; ///< Maximum RX bytes per callback; zero for local-only rows.
  const char* fixture = "host"; ///< Behavioral evidence fixture; no hardware qualification.
  const char* test = "local_routes"; ///< Executed route coverage in the registered CLI suite.
};

/// @brief Return fixed implemented rows, without I2C, allocation or mutable registration.
/// @param[out] count Number of returned rows, valid for program lifetime.
/// @return Static row array; only these routes are advertised by help.
const CommandRow* commandRows(std::size_t& count);

/// @brief One bounded CLI borrowing a driver, transport configuration and platform functions.
/// @details setup does not bind or execute a startup command. feed/processCommand precede tick in
/// each turn. Streaming feed accepts at most 64 bytes across calls and both entry points share a
/// one-command limit. processCommand accepts an already collected bounded line. tick emits
/// at most two incremental help records, drains once at most 256 bytes and starts the next turn.
/// Only tick polls once, with automatic budget one or this turn's explicit job-step budget.
/// Automatic stepping advances only CLI-admitted work; an explicit step can advance represented
/// work. Basic Status callbacks require fresh time after each call, so even larger budgets invoke
/// at most one callback. Staging enters core desired only at accepted apply admission.
/// No ISR/concurrent access or callback reentry is supported. Results remain owned by core.
class Cli {
 public:
  /// @brief Copy local configuration/hooks and borrow the serialized driver; zero callbacks.
  /// @param[in] driver Application driver; must outlive Cli.
  /// @param[in] config Initial selected transport/part; never implicitly bound or adopted by core.
  /// @param[in] platform Borrowed platform functions/context and stable strings.
  Cli(TUSB32x::TUSB32x& driver, const TUSB32x::DriverConfig& config, const Platform& platform);
  Cli(const Cli&) = delete;
  Cli& operator=(const Cli&) = delete;
  /// @brief Validate all local inputs once; no startup output, bind, polling or device work.
  /// @return OK, or INVALID_CONFIG for invalid binding/hooks/identity/repeated setup.
  TUSB32x::Status setup();
  /// @brief Accept a prefix within this turn's remaining 64-byte/one-command budget.
  /// @param[in] data Borrowed bytes; NUL and illegal controls poison through the next terminator.
  /// @param[in] length Available bytes, including possible fragmented CRLF.
  /// @return Consumed prefix length; caller retains the suffix for later turns. Zero after a command,
  /// without setup, on null data or after session exhaustion. Complete poisoned lines get invalid ACK.
  std::size_t feed(const char* data, std::size_t length);
  /// @brief Submit one complete C string directly within the one-command turn limit.
  /// @param[in] line Borrowed terminated command; at most 191 payload bytes, no embedded controls.
  /// @return True if accepted (including malformed rejection); false without effects if turn busy,
  /// setup missing, partial input pending or session exhausted. Use feed for binary input tests.
  bool processCommand(const char* line);
  /// @brief Poll at most once, announce completion, advance finite help and drain once; no waits.
  /// @note Automatic stepping defaults on for CLI-admitted work. job step replaces that turn's
  /// budget, including zero. Polling/deadlines continue during backpressure and session exhaustion.
  /// The application must not also poll the same driver in this serialized CLI owner turn.
  void tick();
  /// @brief Request ordinary incremental help through the same turn/request rules.
  /// @param[in] topic Implemented topic/command or all; default all. Scan is bounded to 33 bytes.
  /// @return Whether the help request was accepted; null/empty/over 32-byte arguments return false.
  /// Other malformed topics get the ordinary typed command rejection; no startup/transport effects.
  bool printHelp(const char* topic = "all");
  /// @brief Copy independently staged candidate without callbacks.
  /// @return Current validated CLI candidate, not retained core desired or observed settings.
  TUSB32x::DeviceProfile stagedProfile() const { return _staged; }
  /// @brief Copy saturating accepted staging generation; default candidate starts at one.
  /// @return Generation is not hardware verification or an operation token.
  uint32_t stagedGeneration() const { return _stagedGeneration; }
  /// @brief Copy CLI-selected transport target; ownership remains with the application.
  /// @return Validated local config; target edits never bind or silently forget core evidence.
  TUSB32x::DriverConfig targetConfig() const { return _config; }
  /// @brief Inspect fixed console state/counters without mutations or callbacks.
  /// @return Borrowed console reference valid for Cli lifetime.
  const ConsoleBuffer& console() const { return _console; }
  /// @brief Inspect session exhaustion; IDs never wrap or reset on setup/markers.
  /// @return True after a command was refused because UINT64_MAX was already issued.
  bool exhausted() const { return _exhausted; }

 private:
  // Only the host fixture can seed boundary state; no production command or public test setter.
  friend struct CliTestAccess;
  struct Request {
    uint64_t id = 0;
    const char* command = "invalid";
    char nonce[33] = "none";
  };
  enum class Payload : uint8_t {
    NONE, VERSION, BUS, CONSOLE, CAPABILITIES, TARGET, PROFILE, PORT, JOB, RESULT
  };
  struct Response {
    TUSB32x::Status status{};
    const char* action = "QUERY";
    Payload payload = Payload::NONE;
    bool deferred = false;
    bool marker = false;
    bool stagedReset = false;
    TUSB32x::OperationToken token{};
    bool take = false;
  };
  TUSB32x::TUSB32x& _driver;
  TUSB32x::DriverConfig _config;
  Platform _platform;
  TUSB32x::DeviceProfile _staged{};
  uint32_t _stagedGeneration = 1;
  ConsoleBuffer _console{};
  bool _setup = false;
  bool _color = true;
  bool _verbose = false;
  bool _commandUsed = false;
  bool _exhausted = false;
  bool _sessionNoticePending = false;
  std::size_t _inputUsed = 0;
  uint64_t _lastRequest = 0;
  char _nonce[33] = "none";
  char _record[ConsoleBuffer::RECORD_CAPACITY]{};
  bool _helpActive = false;
  Request _helpRequest{};
  const char* _helpFilter = "all";
  const char* _helpSection = nullptr;
  std::size_t _helpIndex = 0;
  bool _helpTitle = false;
  Request _operationRequest{};
  TUSB32x::OperationToken _operationToken{};
  bool _completionAnnounced = false;
  bool _automatic = true;
  bool _stepExplicit = false;
  uint8_t _stepBudget = 1;
  Request _stepRequest{};
  bool _stepReport = false;
  void execute(const char* line, bool poisoned);
  bool sessionAvailable();
  void sessionNotice();
  Response dispatch(char (&tokens)[12][33], std::size_t count, Request& request);
  TUSB32x::Status targetEditable(bool changing) const;
  TUSB32x::Status validateTarget(const TUSB32x::DriverConfig& config) const;
  TUSB32x::Status stage(const TUSB32x::DeviceProfile& candidate);
  void record(const Request& request, const char* type, const char* format, ...);
  void human(const char* format, ...);
  void acknowledge(const Request& request, const Response& response);
  void close(const Request& request, const TUSB32x::Status& status,
      TUSB32x::OperationToken token = {}, uint32_t callbacks = 0,
      TUSB32x::OperationState state = TUSB32x::OperationState::IDLE);
  void payload(const Request& request, const Response& response);
  void helpStep();
  void profileOutput(const Request& request);
  void portOutput(const Request& request, const TUSB32x::PortSnapshot& port, uint64_t nowMs);
  void jobOutput(const Request& request, const TUSB32x::Status& status = {});
  void resultOutput(const Request& request, const TUSB32x::OperationResult& result);
  void announceCompletion();
  void announceCompletion(const TUSB32x::OperationResult& result);
  void deliverResult(const Request& request, Response response);
};

}  // namespace tusb32x_cli
