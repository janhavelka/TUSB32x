/// @file Tusb32xCli.cpp
/// @brief Strict dispatch/staging, one managed poll and explicit core Sync diagnostics.
#include "Tusb32xCli.h"

#include <cstring>
#include <limits>

namespace tusb32x_cli {
namespace {
namespace t = TUSB32x;
constexpr char SESSION_EXHAUSTED[] =
    "TUSB SESSION schema=1 code=TOKEN_EXHAUSTED detail=0 reason=request_exhausted\n";

// Admission is always bus-silent. Nonzero bounds describe one owner poll's
// callback ceiling and per-callback bytes, not physical transactions or an entire job.
// Keep topics contiguous so incremental help emits each section exactly once.
constexpr CommandRow ROWS[] = {
    {"help", "?", "common", "help [topic|command]", "CACHE_ONLY", "NONE",
     "[cached] Incremental help for implemented commands", "none", "all", "ACK,HELP,END"},
    {"version", "ver", "common", "version", "CACHE_ONLY", "NONE",
     "[cached] Library, firmware and framework identity", "none", "all", "ACK,VERSION,FIRMWARE,FRAMEWORK,END"},
    {"color", "none", "common", "color [on|off|0|1]", "PURE", "LOCAL_STATE",
     "[cached] Query/change human ANSI color; records stay plain",
     "none", "all", "ACK,CONSOLE,END"},
    {"verbose", "none", "common", "verbose [0|1]", "PURE", "LOCAL_STATE",
     "[cached] Query/change local progress chatter; errors stay visible",
     "none", "all", "ACK,CONSOLE,END"},
    {"hilmark", "none", "common", "hilmark <nonce>", "PURE", "LOCAL_STATE",
     "[cached] Set a validated session marker; none is reserved", "none", "all", "MARK,END"},
    {"bus", "none", "connection", "bus", "CACHE_ONLY", "NONE",
     "[cached] Selected target, adapter framing, pins and counters", "none", "all", "ACK,BUS,END"},
    {"console", "none", "connection", "console", "CACHE_ONLY", "NONE",
     "[cached] Fixed console bounds and retained output losses", "none", "all", "ACK,CONSOLE,END"},
    {"capabilities", "none", "connection", "capabilities", "CACHE_ONLY", "NONE",
     "[cached] Configured capability and separate observed identity", "none", "all", "ACK,CAPABILITIES,END"},
    {"variant", "model", "connection", "variant [name [address]]", "PURE", "LOCAL_STATE",
     "[cached] Query or atomically select a part/address; new part resets staging",
     "ended_idle_no_result", "valid_pair", "ACK,TARGET,END"},
    {"addr", "none", "connection", "addr [address]", "PURE", "LOCAL_STATE",
     "[cached] Query/edit strap address; preserve staging", "ended_idle_no_result", "valid_pair", "ACK,TARGET,END"},
    {"init", "initialize,probe", "connection", "init [timeout_ms]", "CORE_JOB", "READ_OBSERVATION",
     "[live read] Identify/observe only; never reset or replay staging",
     "bound", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"sample", "snapshot", "port", "sample", "CACHE_ONLY", "NONE",
     "[cached] Historical port fields with validity, age and provenance",
     "none", "all", "ACK,PORT,PORT_DETAIL,END"},
    {"freshness", "none", "port", "freshness", "CACHE_ONLY", "NONE",
     "[cached] Port age is unknown without supported owner time",
     "none", "all", "ACK,PORT,PORT_DETAIL,END"},
    {"read", "status", "port", "read", "CORE_JOB", "READ_OBSERVATION",
     "[live read] Refresh port without IRQ acknowledgement",
     "bound", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"profile.show", "cfg,settings", "profile", "profile [show]", "CACHE_ONLY", "NONE",
     "[cached] Separate staged, desired, observed and applied fields",
     "none", "all", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"profile.reset", "none", "profile", "profile reset", "PURE", "LOCAL_STATE",
     "[stage] Candidate defaults, not silicon reset or device writes",
     "none", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"profile.discard", "none", "profile", "profile discard", "PURE", "LOCAL_STATE",
     "[stage] Copy core desired; if absent use explicit profile reset",
     "retained_desired", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"profile.validate", "none", "profile", "profile validate", "PURE", "NONE",
     "[stage] Validate candidate values; no apply or write permission",
     "none", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"mode", "none", "profile", "mode [port|dfp|ufp|drp]", "PURE", "LOCAL_STATE",
     "[stage] Local role candidate; port unsupported on TUSB322I",
     "none", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"current", "none", "profile", "current [default|1a5|3a0]", "PURE", "LOCAL_STATE",
     "[stage] CC advertisement category, not measured current or PD",
     "none", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"duty", "none", "profile", "duty [30|40|50|60]", "PURE", "LOCAL_STATE",
     "[stage] DRP source duty percent; staging is not write permission",
     "none", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"debounce", "none", "profile", "debounce [milliseconds]", "PURE", "LOCAL_STATE",
     "[stage] Base 116/133/151/168; extended 118/134/152/168 ms",
     "none", "variant_values", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"preference", "none", "profile", "preference [normal|try-sink|try-source]", "PURE", "LOCAL_STATE",
     "[stage] Extended dual-role preference; absent on base parts",
     "none", "extended", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"ufpaccessory", "none", "profile", "ufpaccessory [on|off]", "PURE", "LOCAL_STATE",
     "[stage] Extended UFP accessory enabled/disabled, raw polarity separate",
     "none", "extended", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"termination", "none", "profile", "termination [on|off]", "PURE", "LOCAL_STATE",
     "[stage] Extended 0A termination control, distinct from 45 Rd/Rp",
     "none", "extended", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"rd-rp", "none", "profile", "rd-rp [on|off]", "PURE", "LOCAL_STATE",
     "[stage] Separate 45 Rd/Rp control; absent on TUSB322I",
     "none", "has_45", "ACK,CONFIG,CONFIG_DETAIL,PROFILE,END"},
    {"profile.read", "none", "profile", "profile read [timeout_ms]", "CORE_JOB", "READ_OBSERVATION",
     "[live read] Observe profile; no desired adoption or write replay",
     "bound", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"apply", "none", "profile", "apply [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
     "[device write] Admit staged candidate; change/prove actual fields",
     "known_actual", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"verify", "selfcheck,selftest", "profile", "verify [timeout_ms]", "CORE_JOB", "READ_OBSERVATION",
     "[live read] Compare retained desired; not board certification",
     "retained_desired", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"reconcile", "none", "profile", "reconcile [timeout_ms]", "CORE_JOB", "READ_OBSERVATION",
     "[live read] Reconcile current proof without replay or cleanup",
     "bound", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"irq.read", "none", "interrupt", "irq read", "CORE_JOB", "READ_OBSERVATION",
     "[live read] Refresh port/IRQ evidence; no W1C acknowledgement",
     "bound", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"irq.ack", "none", "interrupt", "irq ack [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
     "[device write] ACK using actual controls, never staged settings",
     "bound", "irq_ack", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"irq.service", "none", "interrupt", "irq service [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
     "[device write] Bounded observe/ACK/recheck; events may coalesce",
     "bound", "irq_ack", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"irq", "none", "interrupt", "irq", "CACHE_ONLY", "NONE",
     "[cached] Last managed IRQ operation; not a fresh latch observation",
     "none", "all", "ACK,IRQ,END", 0, 0, 0, "host", "diagnostic_routes"},
    {"intpin", "none", "interrupt", "intpin", "PLATFORM_ACTION", "NONE",
     "[live read] Opt-in application input level; no GPIO setup or I2C",
     "optional_input", "application", "ACK,PIN,END", 0, 0, 0, "host", "diagnostic_routes"},
    {"bind", "none", "lifecycle", "bind", "PURE", "LOCAL_STATE",
     "[cached] Bind selected transport; no initialize or evidence forgetting",
     "idle_no_result", "valid_pair", "ACK,TARGET,END"},
    {"end", "unbind", "lifecycle", "end", "PURE", "LOCAL_STATE",
     "[cached] End locally; retain result/effects; no hardware reset",
     "none", "all", "ACK,TARGET,END"},
    {"forget-evidence", "none", "lifecycle", "forget-evidence", "PURE", "LOCAL_STATE",
     "[cached] Explicitly forget target evidence; retain tokens and totals", "ended_idle_no_result",
     "all", "ACK,TARGET,END"},
    {"reset", "none", "lifecycle", "reset [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
     "[device write] Explicit reset/proof; no desired reapply",
     "bound", "all", "ACK,JOB,RESULT,RESULT_DETAIL,END", 1, 2, 8, "host", "operation_routes"},
    {"recover-dead-battery", "none", "lifecycle", "recover-dead-battery [timeout_ms]",
     "CORE_JOB", "DEVICE_WRITE",
     "[device write] Original 320/I source startup; application proves stable VDD",
     "base_stable_vdd", "base_only", "ACK,JOB,RESULT,RESULT_DETAIL,END",
     1, 2, 8, "host", "operation_routes"},
    {"invalidate", "none", "lifecycle", "invalidate <external-reset|power-loss|target-change>",
     "PURE", "LOCAL_STATE", "[cached] Explicit external-change notice; retain effects and result",
     "none", "all", "ACK,INVALIDATE,END", 0, 0, 0, "host", "diagnostic_routes"},
    {"job.status", "progress", "job", "job [status]", "CACHE_ONLY", "NONE",
     "[cached] Core progress, immutable deadline and retained-result fence",
     "none", "all", "ACK,JOB,END", 0, 0, 0, "host", "operation_routes"},
    {"job.auto", "none", "job", "job auto [0|1]", "PURE", "LOCAL_STATE",
     "[cached] Query/change automatic stepping; default one callback per turn",
     "none", "all", "ACK,JOB,END", 0, 0, 0, "host", "operation_routes"},
    {"job.step", "none", "job", "job step [0..255]", "CORE_JOB", "DEVICE_WRITE",
     "[device write] Advance admitted work; may read/write; zero suppresses traffic",
     "represented_operation", "all", "ACK,JOB,END", 1, 2, 8, "host", "operation_routes"},
    {"cancel", "stop,job.cancel", "job", "cancel [token]", "PURE", "LOCAL_STATE",
     "[cached] Cancel matching work locally; possible effects survive",
     "represented_operation", "all", "ACK,END", 0, 0, 0, "host", "operation_routes"},
    {"result", "job.result", "job", "result [token]", "CACHE_ONLY", "NONE",
     "[cached] Repeat immutable core result without consuming it",
     "retained_result", "all", "ACK,RESULT,RESULT_DETAIL,END", 0, 0, 0, "host", "operation_routes"},
    {"result.take", "none", "job", "result take <token>", "PURE", "LOCAL_STATE",
     "[cached] Exact-token delivery; reserve response before releasing core slot",
     "retained_result", "all", "ACK,RESULT,RESULT_DETAIL,END", 0, 0, 0, "host", "operation_routes"},
    {"reg", "rreg,rawread", "registers", "reg <address>", "RAW_I2C", "READ_OBSERVATION",
     "[live read] One raw byte; consuming semantics may be variant-dependent",
     "idle_no_result", "mapped_register", "ACK,REG,DECODE,DECODE_PROFILE,END",
     1, 1, 1, "host", "diagnostic_routes"},
    {"decode", "none", "registers", "decode <address> <byte>", "PURE", "NONE",
     "[cached] Supplied raw field codes, not paired port applicability or proof",
     "none", "mapped_register", "ACK,REG,DECODE,DECODE_PROFILE,END",
     0, 0, 0, "host", "diagnostic_routes"},
    {"wreg", "rawwrite", "registers", "wreg <address> <byte>", "RAW_I2C", "DEVICE_WRITE",
     "[device write] Core-safe image only; sequenced changes need typed operations",
     "idle_no_result", "known_safe_image", "ACK,REG,END", 1, 2, 0, "host", "diagnostic_routes"},
    {"drv", "health,state", "diagnostics", "drv", "CACHE_ONLY", "NONE",
     "[cached] Pure managed health; READY does not prove configuration or port",
     "none", "all", "ACK,DRV,END", 0, 0, 0, "host", "diagnostic_routes"},
    {"diag", "none", "diagnostics", "diag", "CACHE_ONLY", "NONE",
     "[cached] Independent trust, retained effects, identity and failure evidence",
     "none", "all", "ACK,DIAG,MANAGED,FAILURE,IDENTITY,IRQ,XFER,END",
     0, 0, 0, "host", "diagnostic_routes"},
    {"xfer_stats", "counters", "diagnostics", "xfer_stats", "CACHE_ONLY", "NONE",
     "[cached] Saturating all-callback totals, including raw; not bus transactions",
     "none", "all", "ACK,XFER,END", 0, 0, 0, "host", "diagnostic_routes"},
    {"xfer_reset", "none", "diagnostics", "xfer_reset", "PURE", "LOCAL_STATE",
     "[cached] Idle source totals clear only; retain managed health and effects",
     "idle_no_result", "all", "ACK,XFER,END", 0, 0, 0, "host", "diagnostic_routes"},
    {"xfer_assert", "none", "diagnostics", "xfer_assert <attempts> [successes failures]", "PURE", "NONE",
     "[cached] Exact all-source equality; any saturated counter rejects",
     "unsaturated_totals", "all", "ACK,XFER,END", 0, 0, 0, "host", "diagnostic_routes"},
};

bool equal(const char* a, const char* b) { return std::strcmp(a, b) == 0; }
t::Status error(t::Err code) { return t::Status::Error(code, t::errorName(code)); }

// Match one complete comma-separated alias; prefixes do not select commands.
bool alias(const char* list, const char* word) {
  if (equal(list, "none")) return false;
  const std::size_t size = std::strlen(word);
  for (const char* begin = list; *begin != '\0';) {
    const char* end = std::strchr(begin, ',');
    if (end == nullptr) end = begin + std::strlen(begin);
    if (static_cast<std::size_t>(end - begin) == size && std::strncmp(begin, word, size) == 0)
      return true;
    begin = *end == ',' ? end + 1 : end;
  }
  return false;
}

// Resolve canonical names and aliases through the single fixed command table.
const CommandRow* findRow(const char* name) {
  for (const auto& row : ROWS)
    if (equal(row.command, name) || alias(row.aliases, name)) return &row;
  return nullptr;
}

// Require bounded printable identity/correlation tokens without record delimiters.
bool token(const char* text, std::size_t maxLength) {
  if (text == nullptr || *text == '\0') return false;
  for (std::size_t i = 0; i <= maxLength; ++i) {
    const char ch = text[i];
    if (ch == '\0') return true;
    if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
          (ch >= '0' && ch <= '9') || ch == '_' || ch == '-' || ch == '.')) return false;
  }
  return false;
}

// Reserve "none" and dots so client nonces remain unambiguous session identifiers.
bool nonce(const char* text) {
  if (!token(text, 32) || equal(text, "none")) return false;
  return std::strchr(text, '.') == nullptr;
}

// Parse unsigned decimal or opt-in hex without overflow; preserve out on rejection.
bool number(const char* text, uint32_t maximum, uint32_t& out, bool hex = false) {
  unsigned base = 10;
  if (hex && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
    text += 2;
    base = 16;
  }
  if (*text == '\0') return false;
  uint32_t value = 0;
  for (; *text != '\0'; ++text) {
    const char ch = *text;
    const unsigned digit = ch >= '0' && ch <= '9' ? static_cast<unsigned>(ch - '0')
        : base == 16 && ch >= 'a' && ch <= 'f' ? static_cast<unsigned>(ch - 'a') + 10U
        : base == 16 && ch >= 'A' && ch <= 'F' ? static_cast<unsigned>(ch - 'A') + 10U : 16U;
    if (digit >= base || digit > maximum || value > (maximum - digit) / base) return false;
    value = value * base + digit;
  }
  out = value;
  return true;
}

// Accept only nonzero decimal uint64 owner tokens; never narrow them to CLI integers.
bool operationToken(const char* text, t::OperationToken& out) {
  uint64_t value = 0;
  if (*text == '\0') return false;
  for (; *text != '\0'; ++text) {
    if (*text < '0' || *text > '9') return false;
    const auto digit = static_cast<uint64_t>(*text - '0');
    if (value > (std::numeric_limits<uint64_t>::max() - digit) / 10) return false;
    value = value * 10 + digit;
  }
  if (value == 0) return false;
  out.value = value;
  return true;
}

// Accept exact 0/1, plus on/off only for command fields that explicitly permit words.
bool boolean(const char* text, bool& out, bool words) {
  if (equal(text, "1") || (words && equal(text, "on"))) out = true;
  else if (equal(text, "0") || (words && equal(text, "off"))) out = false;
  else return false;
  return true;
}

// Restrict enable controls to the documented on/off grammar.
bool enabled(const char* text, bool& out) {
  if (!equal(text, "on") && !equal(text, "off")) return false;
  return boolean(text, out, true);
}

// Map exact lower-case part names to the public variant enum without probing hardware.
bool variant(const char* name, t::Variant& out) {
  constexpr const char* NAMES[] = {"tusb320", "tusb320i", "tusb320li", "tusb320hi",
      "tusb320lai", "tusb320hai", "tusb322i"};
  for (std::size_t i = 0; i < sizeof(NAMES) / sizeof(NAMES[0]); ++i) {
    if (equal(name, NAMES[i])) {
      out = static_cast<t::Variant>(i + 1U);
      return true;
    }
  }
  return false;
}

// Split one bounded ASCII line into at most 12 tokens of 32 bytes; reject truncation.
bool tokenize(const char* line, char (&tokens)[12][33], std::size_t& count) {
  count = 0;
  std::size_t size = 0;
  bool inToken = false;
  for (std::size_t i = 0; i < ConsoleBuffer::LINE_CAPACITY; ++i) {
    const unsigned char ch = static_cast<unsigned char>(line[i]);
    if (ch == 0) return true;
    if (ch < 0x20 && ch != '\t') return false;
    if (ch >= 0x7F) return false;
    if (ch == ' ' || ch == '\t') { inToken = false; continue; }
    if (!inToken) {
      if (count == 12) return false;
      ++count;
      size = 0;
      inToken = true;
    }
    if (size == 32) return false;
    tokens[count - 1][size++] = static_cast<char>(ch);
    tokens[count - 1][size] = '\0';
  }
  return false;
}

// Saturate local generations so an increment cannot wrap to an older visible value.
void increment(uint32_t& value) {
  if (value != std::numeric_limits<uint32_t>::max()) ++value;
}

}  // namespace

const CommandRow* commandRows(std::size_t& count) {
  count = sizeof(ROWS) / sizeof(ROWS[0]);
  return ROWS;
}

Cli::Cli(TUSB32x::TUSB32x& driver, const TUSB32x::DriverConfig& config, const Platform& platform)
    : _driver(driver), _config(config), _platform(platform) {}

t::Status Cli::setup() {
  if (_setup || _platform.nowMs == nullptr || _platform.write == nullptr ||
      !token(_platform.identity.git, 40) || !token(_platform.identity.tree, 8) ||
      !token(_platform.identity.build, 64) || !token(_platform.identity.framework, 32) ||
      !token(_platform.identity.frameworkVersion, 32) || !token(_platform.identity.target, 32) ||
      !token(_platform.bus, 32) || !token(_platform.framing, 32)) return error(t::Err::INVALID_CONFIG);
  const auto binding = t::validateDriverConfig(_config);
  if (!binding.ok()) return binding;
  const auto candidate = t::validateProfile(_config.variant, _staged);
  if (!candidate.ok()) return candidate;
  _setup = true;
  return t::Status::Ok();
}

std::size_t Cli::feed(const char* data, std::size_t length) {
  if (!_setup || _commandUsed || _exhausted || data == nullptr || length == 0 ||
      _inputUsed == ConsoleBuffer::INPUT_BUDGET)
    return 0;
  if (!sessionAvailable()) return 0;
  const std::size_t remaining = ConsoleBuffer::INPUT_BUDGET - _inputUsed;
  const auto used = _console.feed(data, length < remaining ? length : remaining);
  _inputUsed += used;
  if (_console.lineReady()) {
    _commandUsed = true;
    execute(_console.line(), _console.linePoisoned());
    _console.consumeLine();
  }
  return used;
}

bool Cli::processCommand(const char* line) {
  if (!_setup || _commandUsed || _exhausted || line == nullptr ||
      _console.line()[0] != '\0' || _console.linePoisoned()) return false;
  if (!sessionAvailable()) return false;
  _commandUsed = true;
  execute(line, false);
  return true;
}

void Cli::tick() {
  if (!_setup) return;
  const auto progress = _driver.progress();
  t::Status pollStatus{};
  if (progress.state == t::OperationState::ACTIVE &&
      (_stepExplicit || (_automatic && progress.token.value == _operationToken.value))) {
    // This is the only poll call site. Basic callbacks always return to the owner
    // for fresh time, even when an explicit budget is larger than one.
    pollStatus = _driver.poll(progress.token, _platform.nowMs(_platform.user), _stepExplicit ? _stepBudget : 1);
  }
  const bool pollRejected = !pollStatus.ok() && !pollStatus.inProgress() &&
      _driver.progress().state == t::OperationState::ACTIVE;
  if (_stepReport) {
    jobOutput(_stepRequest, pollStatus);
    close(_stepRequest, pollRejected ? pollStatus : t::Status::Ok(), _driver.progress().token);
    _stepReport = false;
  } else if (pollRejected) jobOutput(_operationRequest, pollStatus);
  _stepExplicit = false;
  announceCompletion();
  helpStep();
  _console.drain(_platform.write, _platform.user);
  sessionNotice();
  _commandUsed = false;
  _inputUsed = 0;
}

bool Cli::printHelp(const char* topic) {
  if (topic == nullptr) return false;
  std::size_t length = 0;
  while (length <= 32 && topic[length] != '\0') ++length;
  if (length == 0 || length > 32) return false;
  char line[38] = "help ";
  std::memcpy(line + 5, topic, length);
  line[5 + length] = '\0';
  return processCommand(line);
}

// Combine pure core validation with the optional application-owned target check.
t::Status Cli::validateTarget(const t::DriverConfig& config) const {
  const auto status = t::validateDriverConfig(config);
  if (!status.ok()) return status;
  if (_platform.validateTarget == nullptr) return t::Status::Ok();
  const auto backend = _platform.validateTarget(config, _platform.user);
  if (backend.inProgress() || backend.code > t::Err::I2C_BUSY)
    return t::Status::Error(t::Err::INVALID_CONFIG, "Target validation must return a terminal status",
                            backend.detail);
  return backend;
}

// Reject edits while bound/busy or while a changed target would discard unresolved effects.
t::Status Cli::targetEditable(bool changing) const {
  const auto progress = _driver.progress();
  const auto diagnostics = _driver.diagnostics();
  if (progress.state == t::OperationState::ACTIVE) return error(t::Err::BUSY);
  if (progress.resultPending) return error(t::Err::RESULT_PENDING);
  if (diagnostics.bound) return error(t::Err::BUSY);
  if (changing && (diagnostics.configurationWriteUnresolved || diagnostics.recoverySequenceUnresolved ||
      diagnostics.modeTransitionUnresolved || diagnostics.resetAcceptanceUncertain ||
      diagnostics.resetState == t::ResetState::MAY_BE_PENDING))
    return error(t::Err::CONFIGURATION_UNKNOWN);
  return t::Status::Ok();
}

// Publish a valid CLI-only candidate and advance its generation; perform no device work.
t::Status Cli::stage(const t::DeviceProfile& candidate) {
  const auto status = t::validateProfile(_config.variant, candidate);
  if (!status.ok()) return status;
  _staged = candidate;
  increment(_stagedGeneration);
  return t::Status::Ok();
}

bool Cli::countersSaturated(const t::TransportStats& stats) {
  constexpr uint32_t maximum = std::numeric_limits<uint32_t>::max();
  return stats.totalAttempts == maximum || stats.totalSuccess == maximum || stats.totalFailures == maximum;
}

t::Status Cli::assertCounters(const t::TransportStats& stats, uint32_t attempts,
    uint32_t success, uint32_t failures, bool all) {
  if (countersSaturated(stats))
    return t::Status::Error(t::Err::INVALID_PARAM, "Exact assertion unavailable for saturated callback totals");
  if (stats.totalAttempts != attempts || (all && (stats.totalSuccess != success || stats.totalFailures != failures)))
    return t::Status::Error(t::Err::INVALID_PARAM, "All-source callback totals differ from expectation");
  return t::Status::Ok();
}

// Stop admission at request-ID exhaustion rather than reuse a correlation identity.
bool Cli::sessionAvailable() {
  if (_lastRequest == std::numeric_limits<uint64_t>::max()) {
    _exhausted = true;
    _sessionNoticePending = !_console.enqueue(SESSION_EXHAUSTED, sizeof(SESSION_EXHAUSTED) - 1);
    return false;
  }
  return true;
}

// Retain the terminal session notice until one complete output record can be queued.
void Cli::sessionNotice() {
  // Preserve the reason across output loss. Retry only after capacity returns;
  // repeated zero-write turns must not generate an unbounded stream of losses.
  if (_sessionNoticePending && _console.available() >= sizeof(SESSION_EXHAUSTED) - 1)
    _sessionNoticePending = !_console.enqueue(SESSION_EXHAUSTED, sizeof(SESSION_EXHAUSTED) - 1);
}

// Allocate one request ID, validate the complete line and frame its bounded response.
void Cli::execute(const char* line, bool poisoned) {
  Request request{};
  request.id = ++_lastRequest;
  std::memcpy(request.nonce, _nonce, sizeof(_nonce));
  char tokens[12][33]{};
  std::size_t count = 0;
  Response response{};
  if (poisoned || !tokenize(line, tokens, count) || count == 0)
    response.status = error(t::Err::INVALID_PARAM);
  else response = dispatch(tokens, count, request);
  if (response.payload == Payload::RESULT) {
    deliverResult(request, response);
    return;
  }
  if (response.marker) {
    std::memcpy(request.nonce, _nonce, sizeof(_nonce));
    record(request, "MARK", "");
  } else acknowledge(request, response);
  if (response.status.ok() || response.status.inProgress() || response.payload == Payload::XFER ||
      response.payload == Payload::PIN) payload(request, response);
  if (!response.deferred) close(request, response.status, response.token, response.callbacks);
}

// Check command-specific arity and values before routing one core action or local edit.
Cli::Response Cli::dispatch(char (&tokens)[12][33], std::size_t count, Request& request) {
  Response response{};
  const auto invalid = [&]() {
    request.command = "invalid";
    Response rejected{};
    rejected.status = error(t::Err::INVALID_PARAM);
    return rejected;
  };
  const CommandRow* row = nullptr;
  if (equal(tokens[0], "profile")) {
    if (count == 1 || (count == 2 && equal(tokens[1], "show"))) row = findRow("profile.show");
    else if (count == 2 && equal(tokens[1], "reset")) row = findRow("profile.reset");
    else if (count == 2 && equal(tokens[1], "discard")) row = findRow("profile.discard");
    else if (count == 2 && equal(tokens[1], "validate")) row = findRow("profile.validate");
    else if ((count == 2 || count == 3) && equal(tokens[1], "read")) row = findRow("profile.read");
    else return invalid();
  } else if (equal(tokens[0], "irq")) {
    if (count == 1) row = findRow("irq");
    else if (count >= 2 && equal(tokens[1], "read")) row = findRow("irq.read");
    else if (count >= 2 && equal(tokens[1], "ack")) row = findRow("irq.ack");
    else if (count >= 2 && equal(tokens[1], "service")) row = findRow("irq.service");
    else return invalid();
  } else if (equal(tokens[0], "job")) {
    if (count == 1 || (count == 2 && equal(tokens[1], "status"))) row = findRow("job.status");
    else if (count >= 2 && equal(tokens[1], "auto")) row = findRow("job.auto");
    else if (count >= 2 && equal(tokens[1], "step")) row = findRow("job.step");
    else if (count >= 2 && equal(tokens[1], "cancel")) row = findRow("cancel");
    else if (count >= 2 && equal(tokens[1], "result")) row = findRow("result");
    else return invalid();
  } else if (equal(tokens[0], "result") && count >= 2 && equal(tokens[1], "take")) {
    row = findRow("result.take");
  } else if (std::strchr(tokens[0], '.') == nullptr) row = findRow(tokens[0]);
  if (row == nullptr) return invalid();
  const char* command = row->command;
  request.command = command;

  if (equal(command, "reg") || equal(command, "decode") || equal(command, "wreg")) {
    const bool writing = equal(command, "wreg");
    const bool supplied = equal(command, "decode");
    uint32_t reg = 0, value = 0;
    if (count != (supplied || writing ? 3U : 2U) || !number(tokens[1], 255, reg, true) ||
        ((supplied || writing) && !number(tokens[2], 255, value, true))) return invalid();
    response.reg = static_cast<uint8_t>(reg);
    response.value = static_cast<uint8_t>(value);
    response.source = supplied ? "supplied" : writing ? "write" : "read";
    if (supplied) {
      response.status = t::decodeRegister(_config.variant, response.reg, response.value, response.decoded);
    } else {
      response.status = writing ? _driver.writeRegisterSync(response.reg, response.value)
          : _driver.readRegisterSync(response.reg, response.value);
      // Public raw API gates return local non-I2C errors. OK/I2C_* mean exactly
      // one invoked callback, even when source counters have saturated.
      response.callbacks = response.status.ok() || (response.status.code >= t::Err::I2C_ERROR &&
          response.status.code <= t::Err::I2C_BUSY) ? 1 : 0;
      if (response.status.ok() && !writing)
        response.status = t::decodeRegister(_driver.diagnostics().identity.variant,
            response.reg, response.value, response.decoded);
      if (response.callbacks != 0) response.action = "ATTEMPTED";
    }
    if (response.status.is(t::Err::UNSUPPORTED_FEATURE))
      response.status.msg = writing
          ? "Raw access is absent, gated or requires a typed mode/reset/termination/recovery operation"
          : "Register is absent on this variant; a supplied byte cannot establish that capability";
    response.payload = Payload::REGISTER;
    return response;
  }
  if (equal(command, "invalidate")) {
    if (count != 2) return invalid();
    const char* message = nullptr;
    if (equal(tokens[1], "external-reset")) {
      response.reason = "external-reset";
      message = "Application reported an external device reset";
    } else if (equal(tokens[1], "power-loss")) {
      response.reason = "power-loss";
      message = "Application reported device power loss";
    } else if (equal(tokens[1], "target-change")) {
      response.reason = "target-change";
      message = "Application reported a physical target change";
    } else return invalid();
    response.status = _driver.invalidateDeviceState(t::Status::Error(t::Err::CONFIGURATION_UNKNOWN, message));
    response.action = "LOCAL";
    response.payload = Payload::INVALIDATE;
    return response;
  }
  if (equal(command, "xfer_assert")) {
    if ((count != 2 && count != 4) ||
        !number(tokens[1], std::numeric_limits<uint32_t>::max(), response.expectedAttempts) ||
        (count == 4 && (!number(tokens[2], std::numeric_limits<uint32_t>::max(), response.expectedSuccess) ||
                       !number(tokens[3], std::numeric_limits<uint32_t>::max(), response.expectedFailures))))
      return invalid();
    response.asserting = true;
    response.assertAll = count == 4;
    const auto stats = _driver.transportStats();
    response.status = assertCounters(stats, response.expectedAttempts,
        response.expectedSuccess, response.expectedFailures, response.assertAll);
    response.assertion = response.status.ok() ? "PASS" : countersSaturated(stats) ? "SATURATED" : "FAIL";
    response.payload = Payload::XFER;
    return response;
  }
  if (equal(command, "irq") || equal(command, "intpin") || equal(command, "drv") ||
      equal(command, "diag") || equal(command, "xfer_stats") || equal(command, "xfer_reset")) {
    if (count != 1) return invalid();
    if (equal(command, "irq")) response.payload = Payload::IRQ;
    else if (equal(command, "intpin")) {
      response.payload = Payload::PIN;
      if (_platform.readInput == nullptr)
        response.status = t::Status::Error(t::Err::UNSUPPORTED_FEATURE, "Application input reader is not configured");
      else {
        response.inputAvailable = _platform.readInput(response.inputLevel, _platform.user);
        if (!response.inputAvailable)
          response.status = t::Status::Error(t::Err::INVALID_CONFIG, "Application input reader did not provide a level");
      }
    } else if (equal(command, "drv")) {
      response.status = t::projectHealth(_driver.diagnostics().managedTransport, {}, response.health);
      response.payload = Payload::DRV;
    } else if (equal(command, "diag")) response.payload = Payload::DIAG;
    else {
      if (equal(command, "xfer_reset")) {
        const auto progress = _driver.progress();
        if (progress.state == t::OperationState::ACTIVE) response.status = error(t::Err::BUSY);
        else if (progress.resultPending) response.status = error(t::Err::RESULT_PENDING);
        else {
          _driver.clearTransportStats();
          response.action = "LOCAL";
        }
      }
      response.payload = Payload::XFER;
    }
    return response;
  }

  if (equal(row->execution, "CORE_JOB") && !equal(command, "job.step")) {
    const bool nested = equal(tokens[0], "profile") || equal(tokens[0], "irq");
    const std::size_t arguments = nested ? 2U : 1U;
    const bool noTimeout = equal(command, "read") || equal(command, "irq.read");
    uint32_t timeout = 1000;
    if (count < arguments || count > arguments + (noTimeout ? 0U : 1U) ||
        (count > arguments && (!number(tokens[arguments], 2147483647U, timeout) || timeout == 0)))
      return invalid();
    const uint64_t now = _platform.nowMs(_platform.user);
    const t::OperationTiming timing{now, now + timeout};
    if (equal(command, "init")) response.status = _driver.startInitialize(timing, response.token);
    else if (equal(command, "read") || equal(command, "irq.read"))
      response.status = _driver.startRefreshPort(timing, response.token);
    else if (equal(command, "profile.read")) response.status = _driver.startReadProfile(timing, response.token);
    else if (equal(command, "apply")) response.status = _driver.startApplyProfile(_staged, timing, response.token);
    else if (equal(command, "verify")) response.status = _driver.startVerifyProfile(timing, response.token);
    else if (equal(command, "reconcile")) response.status = _driver.startReconcile(timing, response.token);
    else if (equal(command, "irq.ack")) response.status = _driver.startAcknowledgeInterrupt(timing, response.token);
    else if (equal(command, "irq.service")) response.status = _driver.startServiceInterrupt(timing, response.token);
    else if (equal(command, "reset")) response.status = _driver.startReset(timing, response.token);
    else response.status = _driver.startRecoverDeadBattery(timing, response.token);
    if (response.status.is(t::Err::UNSUPPORTED_FEATURE)) {
      if (equal(command, "recover-dead-battery"))
        response.status.msg = "Dead-battery recovery is qualified only for original TUSB320/I after stable VDD";
      else if (equal(command, "irq.ack") || equal(command, "irq.service"))
        response.status.msg = "IRQ/fault-clear writes are not qualified on this variant";
      else response.status.msg = "Requested changes require a gated variant capability; unchanged proved fields may remain usable";
    }
    if (response.status.inProgress()) {
      _operationRequest = request;
      _operationToken = response.token;
      _completionAnnounced = false;
      response.action = "ADMITTED";
      response.deferred = true;
      response.payload = Payload::JOB;
    }
    return response;
  }
  if (equal(command, "job.status") || equal(command, "job.auto") || equal(command, "job.step")) {
    const bool nested = equal(tokens[0], "job");
    const std::size_t arguments = nested && count > 1 ? 2U : 1U;
    const bool status = equal(command, "job.status");
    if (status && count != arguments) return invalid();
    if (!status && (count < arguments || count > arguments + 1)) return invalid();
    response.payload = Payload::JOB;
    response.token = _driver.progress().token;
    if (equal(command, "job.auto") && count > arguments) {
      bool value = false;
      if (!boolean(tokens[arguments], value, false)) return invalid();
      _automatic = value;
      response.action = "LOCAL";
    } else if (equal(command, "job.step")) {
      uint32_t budget = 1;
      if (count > arguments && !number(tokens[arguments], 255, budget)) return invalid();
      if (!response.token.valid()) {
        response.status = error(t::Err::RESULT_NOT_AVAILABLE);
        return response;
      }
      _stepExplicit = true;
      _stepBudget = static_cast<uint8_t>(budget);
      _stepRequest = request;
      _stepReport = true;
      response.action = "LOCAL";
      response.payload = Payload::NONE;
      response.deferred = true;
    }
    return response;
  }
  if (equal(command, "cancel") || equal(command, "result") || equal(command, "result.take")) {
    const bool take = equal(command, "result.take");
    const std::size_t arguments = take || equal(tokens[0], "job") ? 2U : 1U;
    if (count < arguments || count > arguments + 1 || (take && count != arguments + 1))
      return invalid();
    response.token = _driver.progress().token;
    if (count > arguments && !operationToken(tokens[arguments], response.token)) return invalid();
    if (equal(command, "cancel")) {
      response.status = _driver.cancel(response.token);
      response.action = "LOCAL";
    } else {
      response.payload = Payload::RESULT;
      response.take = take;
      response.action = take ? "LOCAL" : "QUERY";
    }
    return response;
  }

  if (equal(command, "help")) {
    if (count > 2) return invalid();
    const char* filter = "all";
    if (count == 2) {
      filter = nullptr;
      if (equal(tokens[1], "all")) filter = "all";
      for (const auto& entry : ROWS)
        if (equal(entry.topic, tokens[1])) { filter = entry.topic; break; }
      if (filter == nullptr) {
        const auto* found = findRow(tokens[1]);
        if (found != nullptr) filter = found->command;
      }
      if (filter == nullptr) return invalid();
    }
    if (_helpActive) { response.status = error(t::Err::BUSY); return response; }
    _helpActive = true;
    _helpRequest = request;
    _helpFilter = filter;
    _helpIndex = 0;
    _helpSection = nullptr;
    _helpTitle = false;
    response.deferred = true;
    return response;
  }
  if (equal(command, "color") || equal(command, "verbose")) {
    if (count > 2) return invalid();
    bool value = false;
    const bool color = equal(command, "color");
    if (count == 2 && !boolean(tokens[1], value, color)) return invalid();
    if (count == 2) {
      if (color) _color = value;
      else _verbose = value;
      response.action = "LOCAL";
    }
    response.payload = Payload::CONSOLE;
    return response;
  }
  if (equal(command, "hilmark")) {
    if (count != 2 || !nonce(tokens[1])) return invalid();
    std::strcpy(_nonce, tokens[1]);
    response.action = "LOCAL";
    response.marker = true;
    return response;
  }
  if (equal(command, "variant") || equal(command, "addr")) {
    const bool selectingVariant = equal(command, "variant");
    if (count > (selectingVariant ? 3U : 2U)) return invalid();
    response.payload = Payload::TARGET;
    if (count == 1) return response;
    auto config = _config;
    uint32_t address = 0;
    if (selectingVariant) {
      if (!variant(tokens[1], config.variant)) return invalid();
      if (count == 3 && !number(tokens[2], 255, address, true)) return invalid();
      if (count == 3) config.address = static_cast<t::DeviceAddress>(address);
    } else {
      if (!number(tokens[1], 255, address, true)) return invalid();
      config.address = static_cast<t::DeviceAddress>(address);
    }
    // Validate the pair and replacement candidate before any state or generation changes.
    response.status = validateTarget(config);
    if (!response.status.ok()) return response;
    const bool variantChanged = config.variant != _config.variant;
    const bool changing = variantChanged || config.address != _config.address;
    t::DeviceProfile candidate{};
    if (variantChanged) {
      response.status = t::validateProfile(config.variant, candidate);
      if (!response.status.ok()) return response;
    }
    response.status = targetEditable(changing);
    if (!response.status.ok()) return response;
    _config = config;
    if (variantChanged) {
      _staged = candidate;
      increment(_stagedGeneration);
      response.stagedReset = true;
    }
    response.action = changing ? "LOCAL" : "UNCHANGED";
    return response;
  }
  if (equal(command, "profile.show") || equal(command, "profile.reset") ||
      equal(command, "profile.discard") || equal(command, "profile.validate")) {
    // The profile parser already checked its complete subcommand; aliases take no arguments.
    if (!equal(tokens[0], "profile") && count != 1) return invalid();
    response.payload = Payload::PROFILE;
    if (equal(command, "profile.reset")) {
      response.status = stage(t::DeviceProfile{});
      response.action = "STAGED";
    } else if (equal(command, "profile.discard")) {
      const auto config = _driver.configurationSnapshot();
      response.status = config.hasDesiredProfile ? stage(config.desiredProfile)
          : t::Status::Error(t::Err::CONFIGURATION_UNKNOWN, "No retained desired profile; use profile reset");
      response.action = "STAGED";
    } else if (equal(command, "profile.validate"))
      response.status = t::validateProfile(_config.variant, _staged);
    return response;
  }
  // These explicit rows have no arguments, so reject extras before touching core lifecycle.
  if (equal(command, "version") || equal(command, "bus") || equal(command, "console") ||
      equal(command, "capabilities") || equal(command, "sample") || equal(command, "freshness") ||
      equal(command, "bind") || equal(command, "end") || equal(command, "forget-evidence")) {
    if (count != 1) return invalid();
    if (equal(command, "version")) response.payload = Payload::VERSION;
    else if (equal(command, "bus")) response.payload = Payload::BUS;
    else if (equal(command, "console")) response.payload = Payload::CONSOLE;
    else if (equal(command, "capabilities")) response.payload = Payload::CAPABILITIES;
    else if (equal(command, "sample") || equal(command, "freshness")) response.payload = Payload::PORT;
    else {
      if (equal(command, "bind")) {
        response.status = validateTarget(_config);
        if (response.status.ok()) response.status = _driver.bind(_config);
      } else if (equal(command, "end")) response.status = _driver.end();
      else response.status = _driver.forgetDeviceEvidence();
      response.action = "LOCAL";
      response.payload = Payload::TARGET;
    }
    return response;
  }

  // Remaining implemented rows are fixed profile control getters/setters.
  if (count > 2) return invalid();
  auto candidate = _staged;
  bool on = false;
  uint32_t value = 0;
  if (count == 2) {
    const char* arg = tokens[1];
    if (equal(command, "mode")) {
      if (equal(arg, "port")) candidate.portMode = t::PortMode::FOLLOW_PORT_PIN;
      else if (equal(arg, "dfp")) candidate.portMode = t::PortMode::DFP;
      else if (equal(arg, "ufp")) candidate.portMode = t::PortMode::UFP;
      else if (equal(arg, "drp")) candidate.portMode = t::PortMode::DRP;
      else return invalid();
    } else if (equal(command, "current")) {
      if (equal(arg, "default")) candidate.sourceCurrent = t::SourceCurrent::DEFAULT;
      else if (equal(arg, "1a5")) candidate.sourceCurrent = t::SourceCurrent::MA_1500;
      else if (equal(arg, "3a0")) candidate.sourceCurrent = t::SourceCurrent::MA_3000;
      else return invalid();
    } else if (equal(command, "preference")) {
      if (equal(arg, "normal")) candidate.sourcePreference = t::SourcePreference::NORMAL;
      else if (equal(arg, "try-sink")) candidate.sourcePreference = t::SourcePreference::TRY_SNK;
      else if (equal(arg, "try-source")) candidate.sourcePreference = t::SourcePreference::TRY_SRC;
      else return invalid();
    } else if (equal(command, "duty")) {
      if (!number(arg, 60, value) || (value != 30 && value != 40 && value != 50 && value != 60))
        return invalid();
      candidate.drpDutyCycle = static_cast<t::DrpDutyCycle>((value - 30U) / 10U);
    } else if (equal(command, "debounce")) {
      if (!number(arg, std::numeric_limits<uint32_t>::max(), value)) return invalid();
      switch (value) {
        case 116: candidate.ccDebounce = t::CcDebounce::MS_116; break;
        case 118: candidate.ccDebounce = t::CcDebounce::MS_118; break;
        case 133: candidate.ccDebounce = t::CcDebounce::MS_133; break;
        case 134: candidate.ccDebounce = t::CcDebounce::MS_134; break;
        case 151: candidate.ccDebounce = t::CcDebounce::MS_151; break;
        case 152: candidate.ccDebounce = t::CcDebounce::MS_152; break;
        case 168: candidate.ccDebounce = t::CcDebounce::MS_168; break;
        default: response.status = error(t::Err::UNSUPPORTED_FEATURE); return response;
      }
    } else {
      if (!enabled(arg, on)) return invalid();
      if (equal(command, "ufpaccessory")) candidate.disableUfpAccessory = !on;
      else if (equal(command, "termination")) candidate.disableTerm = !on;
      else if (equal(command, "rd-rp")) candidate.disableRdRp = !on;
      else return invalid();
    }
  }
  const bool base = _config.variant == t::Variant::TUSB320 || _config.variant == t::Variant::TUSB320I;
  if ((base && (equal(command, "preference") || equal(command, "ufpaccessory") ||
                equal(command, "termination"))) ||
      (_config.variant == t::Variant::TUSB322I && equal(command, "rd-rp"))) {
    response.status = t::Status::Error(t::Err::UNSUPPORTED_FEATURE, "This control is absent on the selected variant");
    return response;
  }
  response.payload = Payload::PROFILE;
  if (count == 2) {
    response.status = stage(candidate);
    response.action = "STAGED";
  }
  return response;
}

}  // namespace tusb32x_cli
