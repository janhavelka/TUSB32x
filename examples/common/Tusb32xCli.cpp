/// @file Tusb32xCli.cpp
/// @brief Strict local command dispatch, target admission and independent staging; zero transport.
#include "Tusb32xCli.h"

#include <cstring>
#include <limits>

namespace tusb32x_cli {
namespace {
namespace t = TUSB32x;
constexpr char SESSION_EXHAUSTED[] =
    "TUSB SESSION schema=1 code=TOKEN_EXHAUSTED detail=0 reason=request_exhausted\n";

// These are executable rows, not future routes. Shared zero callback/byte bounds
// are intentional: all handlers in prompt20 are local and tick never polls core.
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
    {"bind", "none", "lifecycle", "bind", "PURE", "LOCAL_STATE",
     "[cached] Bind selected transport; no initialize or evidence forgetting",
     "idle_no_result", "valid_pair", "ACK,TARGET,END"},
    {"end", "unbind", "lifecycle", "end", "PURE", "LOCAL_STATE",
     "[cached] End locally; retain result/effects; no hardware reset",
     "none", "all", "ACK,TARGET,END"},
    {"forget-evidence", "none", "lifecycle", "forget-evidence", "PURE", "LOCAL_STATE",
     "[cached] Explicitly forget target evidence; retain tokens and totals", "ended_idle_no_result",
     "all", "ACK,TARGET,END"},
    {"sample", "snapshot", "port", "sample", "CACHE_ONLY", "NONE",
     "[cached] Historical port fields with validity, age and provenance",
     "none", "all", "ACK,PORT,PORT_DETAIL,END"},
    {"freshness", "none", "port", "freshness", "CACHE_ONLY", "NONE",
     "[cached] Port age is unknown without supported owner time",
     "none", "all", "ACK,PORT,PORT_DETAIL,END"},
    {"profile.show", "cfg,settings", "profile", "profile [show]", "CACHE_ONLY", "NONE",
     "[cached] Separate staged, desired, observed and applied fields",
     "none", "all", "ACK,CONFIG,PROFILE,END"},
    {"profile.reset", "none", "profile", "profile reset", "PURE", "LOCAL_STATE",
     "[stage] Candidate defaults, not silicon reset or device writes",
     "none", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"profile.discard", "none", "profile", "profile discard", "PURE", "LOCAL_STATE",
     "[stage] Copy core desired; if absent use explicit profile reset",
     "retained_desired", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"profile.validate", "none", "profile", "profile validate", "PURE", "NONE",
     "[stage] Validate candidate values; no apply or write permission",
     "none", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"mode", "none", "profile", "mode [port|dfp|ufp|drp]", "PURE", "LOCAL_STATE",
     "[stage] Local role candidate; port unsupported on TUSB322I",
     "none", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"current", "none", "profile", "current [default|1a5|3a0]", "PURE", "LOCAL_STATE",
     "[stage] CC advertisement category, not measured current or PD",
     "none", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"duty", "none", "profile", "duty [30|40|50|60]", "PURE", "LOCAL_STATE",
     "[stage] DRP source duty percent; staging is not write permission",
     "none", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"debounce", "none", "profile", "debounce [milliseconds]", "PURE", "LOCAL_STATE",
     "[stage] Base 116/133/151/168; extended 118/134/152/168 ms",
     "none", "variant_values", "ACK,CONFIG,PROFILE,END"},
    {"preference", "none", "profile", "preference [normal|try-sink|try-source]", "PURE", "LOCAL_STATE",
     "[stage] Extended dual-role preference; absent on base parts",
     "none", "extended", "ACK,CONFIG,PROFILE,END"},
    {"ufpaccessory", "none", "profile", "ufpaccessory [on|off]", "PURE", "LOCAL_STATE",
     "[stage] Extended UFP accessory enabled/disabled, raw polarity separate",
     "none", "extended", "ACK,CONFIG,PROFILE,END"},
    {"termination", "none", "profile", "termination [on|off]", "PURE", "LOCAL_STATE",
     "[stage] Extended 0A termination control, distinct from 45 Rd/Rp",
     "none", "extended", "ACK,CONFIG,PROFILE,END"},
    {"rd-rp", "none", "profile", "rd-rp [on|off]", "PURE", "LOCAL_STATE",
     "[stage] Separate 45 Rd/Rp control; absent on TUSB322I",
     "none", "has_45", "ACK,CONFIG,PROFILE,END"},
};

bool equal(const char* a, const char* b) { return std::strcmp(a, b) == 0; }
t::Status error(t::Err code) { return t::Status::Error(code, t::errorName(code)); }

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

const CommandRow* findRow(const char* name) {
  for (const auto& row : ROWS)
    if (equal(row.command, name) || alias(row.aliases, name)) return &row;
  return nullptr;
}

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

bool nonce(const char* text) {
  if (!token(text, 32) || equal(text, "none")) return false;
  return std::strchr(text, '.') == nullptr;
}

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

bool boolean(const char* text, bool& out, bool words) {
  if (equal(text, "1") || (words && equal(text, "on"))) out = true;
  else if (equal(text, "0") || (words && equal(text, "off"))) out = false;
  else return false;
  return true;
}

bool enabled(const char* text, bool& out) {
  if (!equal(text, "on") && !equal(text, "off")) return false;
  return boolean(text, out, true);
}

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

t::Status Cli::stage(const t::DeviceProfile& candidate) {
  const auto status = t::validateProfile(_config.variant, candidate);
  if (!status.ok()) return status;
  _staged = candidate;
  increment(_stagedGeneration);
  return t::Status::Ok();
}

bool Cli::sessionAvailable() {
  if (_lastRequest == std::numeric_limits<uint64_t>::max()) {
    _exhausted = true;
    _sessionNoticePending = !_console.enqueue(SESSION_EXHAUSTED, sizeof(SESSION_EXHAUSTED) - 1);
    return false;
  }
  return true;
}

void Cli::sessionNotice() {
  // Preserve the reason across output loss. Retry only after capacity returns;
  // repeated zero-write turns must not generate an unbounded stream of losses.
  if (_sessionNoticePending && _console.available() >= sizeof(SESSION_EXHAUSTED) - 1)
    _sessionNoticePending = !_console.enqueue(SESSION_EXHAUSTED, sizeof(SESSION_EXHAUSTED) - 1);
}

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
  if (response.marker) {
    std::memcpy(request.nonce, _nonce, sizeof(_nonce));
    record(request, "MARK", "");
  } else acknowledge(request, response);
  if (response.status.ok()) payload(request, response);
  if (!response.deferred) close(request, response.status);
}

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
    else return invalid();
  } else if (std::strchr(tokens[0], '.') == nullptr) row = findRow(tokens[0]);
  if (row == nullptr) return invalid();
  const char* command = row->command;
  request.command = command;

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
    response.status = error(t::Err::UNSUPPORTED_FEATURE);
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
