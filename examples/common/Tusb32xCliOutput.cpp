/// @file Tusb32xCliOutput.cpp
/// @brief Bounded records, core-owned result delivery, profile/port output and incremental help.
#include "Tusb32xCli.h"

#include <TUSB32x/Version.h>

#include <cinttypes>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace tusb32x_cli {
namespace {
namespace t = TUSB32x;
constexpr const char* UNKNOWN = "unknown";

// Use stable lower-case part names for machine records; invalid enums stay unknown.
const char* variantName(t::Variant value) {
  switch (value) {
    case t::Variant::TUSB320: return "tusb320";
    case t::Variant::TUSB320I: return "tusb320i";
    case t::Variant::TUSB320LI: return "tusb320li";
    case t::Variant::TUSB320HI: return "tusb320hi";
    case t::Variant::TUSB320LAI: return "tusb320lai";
    case t::Variant::TUSB320HAI: return "tusb320hai";
    case t::Variant::TUSB322I: return "tusb322i";
    case t::Variant::UNSPECIFIED: return "unspecified";
  }
  return UNKNOWN;
}

// Render configuration trust independently of communication health or attachment.
const char* configurationName(t::ConfigurationState value) {
  switch (value) {
    case t::ConfigurationState::UNCONFIGURED: return "UNCONFIGURED";
    case t::ConfigurationState::APPLYING: return "APPLYING";
    case t::ConfigurationState::KNOWN: return "KNOWN";
    case t::ConfigurationState::UNKNOWN: return "UNKNOWN";
  }
  return "UNKNOWN";
}

// Bounds-check the operation enum before selecting its static record name.
const char* kindName(t::OperationKind kind) {
  constexpr const char* names[] = {"NONE", "INITIALIZE", "READ_PROFILE", "APPLY_PROFILE",
      "VERIFY_PROFILE", "RECONCILE", "REFRESH_PORT", "ACKNOWLEDGE_INTERRUPT",
      "SERVICE_INTERRUPT", "RESET", "RECOVER_DEAD_BATTERY"};
  const auto index = static_cast<unsigned>(kind);
  return index < sizeof(names) / sizeof(names[0]) ? names[index] : "UNKNOWN";
}

// Keep active, terminal and idle outcomes distinct in operation records.
const char* operationStateName(t::OperationState state) {
  constexpr const char* names[] = {"IDLE", "ACTIVE", "SUCCEEDED", "FAILED", "CANCELLED", "TIMED_OUT"};
  const auto index = static_cast<unsigned>(state);
  return index < sizeof(names) / sizeof(names[0]) ? names[index] : "UNKNOWN";
}

// Name the public progress phase without exposing internal engine states.
const char* phaseName(t::OperationPhase phase) {
  constexpr const char* names[] = {"NONE", "READING", "WRITING", "WAITING", "FINISHING"};
  const auto index = static_cast<unsigned>(phase);
  return index < sizeof(names) / sizeof(names[0]) ? names[index] : "UNKNOWN";
}

// Preserve partial and indeterminate hardware effects in formatted output.
const char* effectName(t::HardwareEffect effect) {
  constexpr const char* names[] = {"NONE", "CONFIRMED", "PARTIAL", "INDETERMINATE"};
  const auto index = static_cast<unsigned>(effect);
  return index < sizeof(names) / sizeof(names[0]) ? names[index] : "UNKNOWN";
}

// Retain unknown termination evidence instead of presenting enabled as a default.
const char* terminationName(t::TerminationState state) {
  constexpr const char* names[] = {"UNKNOWN", "ENABLED", "DISABLED"};
  const auto index = static_cast<unsigned>(state);
  return index < sizeof(names) / sizeof(names[0]) ? names[index] : "UNKNOWN";
}

// Distinguish a possibly pending reset from proved completion.
const char* resetName(t::ResetState state) {
  constexpr const char* names[] = {"NONE", "MAY_BE_PENDING", "COMPLETED"};
  const auto index = static_cast<unsigned>(state);
  return index < sizeof(names) / sizeof(names[0]) ? names[index] : "UNKNOWN";
}

// Format the passive health projection without deriving configuration readiness.
const char* healthName(t::DriverState state) {
  switch (state) {
    case t::DriverState::UNINIT: return "UNINIT";
    case t::DriverState::READY: return "READY";
    case t::DriverState::DEGRADED: return "DEGRADED";
    case t::DriverState::OFFLINE: return "OFFLINE";
  }
  return "UNKNOWN";
}

// Select displayable profile fields by configured variant; this grants no write permission.
uint16_t applicableFields(t::Variant variant) {
  if (variant == t::Variant::TUSB320 || variant == t::Variant::TUSB320I) return 0x9B;
  if (variant == t::Variant::TUSB322I) return 0x7F;
  if (variant == t::Variant::TUSB320LI || variant == t::Variant::TUSB320HI ||
      variant == t::Variant::TUSB320LAI || variant == t::Variant::TUSB320HAI) return 0xFF;
  return 0;
}

// Match the CLI's profile mode spelling, including the physical PORT-pin selection.
const char* modeName(t::PortMode value) {
  switch (value) {
    case t::PortMode::FOLLOW_PORT_PIN: return "port";
    case t::PortMode::DFP: return "dfp";
    case t::PortMode::UFP: return "ufp";
    case t::PortMode::DRP: return "drp";
  }
  return UNKNOWN;
}

// Format CC advertisement settings; these values are not measured load currents.
const char* currentName(t::SourceCurrent value) {
  switch (value) {
    case t::SourceCurrent::DEFAULT: return "default";
    case t::SourceCurrent::MA_1500: return "1a5";
    case t::SourceCurrent::MA_3000: return "3a0";
  }
  return UNKNOWN;
}

// Map source-preference codes to the same words accepted by profile staging.
const char* preferenceName(t::SourcePreference value) {
  switch (value) {
    case t::SourcePreference::NORMAL: return "normal";
    case t::SourcePreference::TRY_SNK: return "try-sink";
    case t::SourcePreference::TRY_SRC: return "try-source";
  }
  return UNKNOWN;
}

// Return the configured DRP duty percentage as a static decimal token.
const char* dutyName(t::DrpDutyCycle value) {
  switch (value) {
    case t::DrpDutyCycle::PERCENT_30: return "30";
    case t::DrpDutyCycle::PERCENT_40: return "40";
    case t::DrpDutyCycle::PERCENT_50: return "50";
    case t::DrpDutyCycle::PERCENT_60: return "60";
  }
  return UNKNOWN;
}

// Return the nominal CC debounce setting in milliseconds, without inferring elapsed time.
const char* debounceName(t::CcDebounce value) {
  switch (value) {
    case t::CcDebounce::MS_116: return "116";
    case t::CcDebounce::MS_118: return "118";
    case t::CcDebounce::MS_133: return "133";
    case t::CcDebounce::MS_134: return "134";
    case t::CcDebounce::MS_151: return "151";
    case t::CcDebounce::MS_152: return "152";
    case t::CcDebounce::MS_168: return "168";
  }
  return UNKNOWN;
}

// Keep detached, accessory and local source/sink attachment labels distinct.
const char* attachmentName(t::Attachment value) {
  switch (value) {
    case t::Attachment::UNKNOWN: return "UNKNOWN";
    case t::Attachment::DETACHED: return "DETACHED";
    case t::Attachment::LOCAL_SOURCE: return "LOCAL_SOURCE";
    case t::Attachment::LOCAL_SINK: return "LOCAL_SINK";
    case t::Attachment::ACCESSORY: return "ACCESSORY";
  }
  return "UNKNOWN";
}

// Preserve unknown versus inapplicable role fields in cached observations.
const char* roleName(t::PortRole value) {
  switch (value) {
    case t::PortRole::UNKNOWN: return "UNKNOWN";
    case t::PortRole::NOT_APPLICABLE: return "NOT_APPLICABLE";
    case t::PortRole::SOURCE: return "SOURCE";
    case t::PortRole::SINK: return "SINK";
  }
  return "UNKNOWN";
}

// Display CC1/CC2 only when supplied as such; unknown and inapplicable remain explicit.
const char* orientationName(t::Orientation value) {
  switch (value) {
    case t::Orientation::UNKNOWN: return "UNKNOWN";
    case t::Orientation::NOT_APPLICABLE: return "NOT_APPLICABLE";
    case t::Orientation::CC1: return "CC1";
    case t::Orientation::CC2: return "CC2";
  }
  return "UNKNOWN";
}

// Render decoded CC current indication, including the charge-through accessory code.
const char* detectedName(t::DetectedCurrent value) {
  switch (value) {
    case t::DetectedCurrent::UNKNOWN: return "UNKNOWN";
    case t::DetectedCurrent::NOT_APPLICABLE: return "NOT_APPLICABLE";
    case t::DetectedCurrent::DEFAULT: return "DEFAULT";
    case t::DetectedCurrent::MA_1500: return "MA_1500";
    case t::DetectedCurrent::CHARGE_THROUGH_500_MA: return "CHARGE_THROUGH_500_MA";
    case t::DetectedCurrent::MA_3000: return "MA_3000";
  }
  return "UNKNOWN";
}

// Keep accessory classifications and local debug roles visible as separate tokens.
const char* accessoryName(t::Accessory value) {
  switch (value) {
    case t::Accessory::UNKNOWN: return "UNKNOWN";
    case t::Accessory::NOT_APPLICABLE: return "NOT_APPLICABLE";
    case t::Accessory::NONE: return "NONE";
    case t::Accessory::AUDIO: return "AUDIO";
    case t::Accessory::CHARGE_THROUGH: return "CHARGE_THROUGH";
    case t::Accessory::DEBUG: return "DEBUG";
    case t::Accessory::DEBUG_LOCAL_DFP: return "DEBUG_LOCAL_DFP";
    case t::Accessory::DEBUG_LOCAL_UFP: return "DEBUG_LOCAL_UFP";
  }
  return "UNKNOWN";
}

// Report the fixed read plan's coherence evidence without claiming hardware atomicity.
const char* coherenceName(t::SnapshotCoherence value) {
  switch (value) {
    case t::SnapshotCoherence::SEQUENTIAL: return "SEQUENTIAL";
    case t::SnapshotCoherence::BOOKENDS_MATCH: return "BOOKENDS_MATCH";
    case t::SnapshotCoherence::CHANGED_DURING_READ: return "CHANGED_DURING_READ";
  }
  return "UNKNOWN";
}

// Convert known help topic keys to display titles; retain an unknown key unchanged.
const char* sectionName(const char* topic) {
  if (std::strcmp(topic, "common") == 0) return "Common";
  if (std::strcmp(topic, "connection") == 0) return "Connection";
  if (std::strcmp(topic, "port") == 0) return "Port";
  if (std::strcmp(topic, "profile") == 0) return "Profile";
  if (std::strcmp(topic, "lifecycle") == 0) return "Lifecycle";
  if (std::strcmp(topic, "interrupt") == 0) return "Interrupt";
  if (std::strcmp(topic, "job") == 0) return "Job";
  if (std::strcmp(topic, "registers") == 0) return "Registers";
  if (std::strcmp(topic, "diagnostics") == 0) return "Diagnostics";
  return topic;
}

// Format a valid byte into caller storage; invalid evidence returns a static unknown token.
const char* rawByte(uint8_t byte, bool valid, char (&buffer)[4]) {
  if (!valid) return UNKNOWN;
  std::snprintf(buffer, sizeof(buffer), "%u", static_cast<unsigned>(byte));
  return buffer;
}

// Fit a uint32 decimal count plus NUL, or return unknown without using the buffer.
const char* optionalCount(uint32_t value, bool valid, char (&buffer)[11]) {
  if (!valid) return UNKNOWN;
  std::snprintf(buffer, sizeof(buffer), "%" PRIu32, value);
  return buffer;
}

// Borrow only a complete printable message within 256 bytes; otherwise use static fallback.
const char* statusMessage(const char* message) {
  // An application target validator may supply text. Scan a fixed prefix and reject
  // control/non-ASCII bytes rather than echoing terminal controls or partial messages.
  if (message) {
    for (std::size_t index = 0; index < 256; ++index) {
      const unsigned char byte = static_cast<unsigned char>(message[index]);
      if (byte == 0) return message;
      if (byte < 32 || byte > 126) break;
    }
  }
  return "Message omitted; inspect typed code and detail";
}
}  // namespace

// Queue a whole correlated record from fixed storage; oversize formatting records output loss.
void Cli::record(const Request& request, const char* type, const char* format, ...) {
  const int prefix = std::snprintf(_record, sizeof(_record),
      "TUSB %s schema=1 nonce=%s request=%" PRIu64, type, request.nonce, request.id);
  if (prefix < 0 || static_cast<std::size_t>(prefix) >= sizeof(_record) - 2) {
    _console.enqueue(nullptr, sizeof(_record));
    return;
  }
  std::size_t used = static_cast<std::size_t>(prefix);
  if (format[0] != '\0') _record[used++] = ' ';
  va_list arguments;
  va_start(arguments, format);
  const int body = std::vsnprintf(_record + used, sizeof(_record) - used,
      format, arguments);
  va_end(arguments);
  if (body < 0) {
    _console.enqueue(nullptr, sizeof(_record));
    return;
  }
  const std::size_t length = used + static_cast<std::size_t>(body) + 1;
  if (length >= sizeof(_record)) {
    // Size rejection happens before ConsoleBuffer reads bytes; never queue a truncated prefix.
    _console.enqueue(_record, length);
    return;
  }
  _record[length - 1] = '\n';
  _record[length] = '\0';
  _console.enqueue(_record, length);
}

// Format human output within one fixed record; the console rejects truncation and bad framing.
void Cli::human(const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const int length = std::vsnprintf(_record, sizeof(_record), format, arguments);
  va_end(arguments);
  if (length < 0) _console.enqueue(nullptr, sizeof(_record));
  else _console.enqueue(_record, static_cast<std::size_t>(length));
}

// Publish admission status and cached generations, with optional sanitized human diagnostics.
void Cli::acknowledge(const Request& request, const Response& response) {
  const auto config = _driver.configurationSnapshot();
  record(request, "ACK", "command=%s code=%s detail=%" PRId32
      " action=%s token=%" PRIu64 " staged_generation=%" PRIu32 " desired_generation=%" PRIu32
      " applied_generation=%" PRIu32 " configuration=%s", request.command,
      t::errorName(response.status.code), response.status.detail,
      response.status.ok() || response.status.inProgress() || response.callbacks != 0
          ? response.action : "UNCHANGED",
      response.token.value,
      _stagedGeneration, config.desiredGeneration, config.verifiedGeneration,
      configurationName(config.state));
  if (!response.status.ok() || _verbose) {
    const bool progress = response.status.inProgress();
    human("%s[%s] %s detail=%" PRId32 " %.255s%s\n",
        _color && !response.status.ok() ? progress ? "\033[33m" : "\033[31m" : "",
        response.status.ok() ? "OK" : progress ? "W" : "E",
        t::errorName(response.status.code), response.status.detail,
        statusMessage(response.status.msg), _color ? "\033[0m" : "");
  }
}

void Cli::close(const Request& request, const t::Status& status, t::OperationToken token,
    uint32_t callbacks, t::OperationState state) {
  const char* outcome = state == t::OperationState::IDLE
      ? status.ok() ? "SUCCEEDED" : "FAILED" : operationStateName(state);
  record(request, "END", "command=%s outcome=%s code=%s detail=%" PRId32
      " token=%" PRIu64 " callbacks=%" PRIu32, request.command, outcome,
      t::errorName(status.code), status.detail, token.value, callbacks);
}

// Format the selected response view from cached values or explicit application observations.
void Cli::payload(const Request& request, const Response& response) {
  const auto diagnostics = _driver.diagnostics();
  switch (response.payload) {
    case Payload::NONE: break;
    case Payload::JOB: jobOutput(request, response.status); break;
    case Payload::RESULT: break; // deliverResult owns reservation and the transient copied value.
    case Payload::REGISTER: registerOutput(request, response); break;
    case Payload::IRQ: interruptOutput(request, diagnostics.interrupt); break;
    case Payload::PIN:
      record(request, "PIN", "source=application available=%u value=%s",
          response.inputAvailable ? 1U : 0U,
          response.inputAvailable ? response.inputLevel ? "1" : "0" : UNKNOWN);
      break;
    case Payload::INVALIDATE: record(request, "INVALIDATE", "reason=%s", response.reason); break;
    case Payload::DRV: healthOutput(request, response.health, diagnostics); break;
    case Payload::DIAG: diagnosticsOutput(request, diagnostics, _driver.transportStats()); break;
    case Payload::XFER: transportOutput(request, response, _driver.transportStats()); break;
    case Payload::VERSION:
      record(request, "VERSION", "library=%s", t::VERSION);
      record(request, "FIRMWARE", "git=%s tree=%s build=%s", _platform.identity.git,
          _platform.identity.tree, _platform.identity.build);
      record(request, "FRAMEWORK", "name=%s version=%s target=%s", _platform.identity.framework,
          _platform.identity.frameworkVersion, _platform.identity.target);
      break;
    case Payload::BUS: {
      const AdapterCounters counters = _platform.counters ?
          _platform.counters(_platform.user) : AdapterCounters{};
      char writes[11], reads[11];
      record(request, "BUS", "variant=%s address=%u bound=%u bus=%s framing=%s bus_hz=%" PRIu32
          " timeout_ms=%" PRIu32 " sda=%" PRId32 " scl=%" PRId32 " irq=%" PRId32
          " adapter_available=%u adapter_writes=%s adapter_write_reads=%s",
          variantName(_config.variant), static_cast<unsigned>(_config.address),
          diagnostics.bound ? 1U : 0U, _platform.bus, _platform.framing, _platform.busHz,
          _config.i2cTimeoutMs, _platform.sdaPin, _platform.sclPin, _platform.irqPin,
          counters.available ? 1U : 0U, optionalCount(counters.writes, counters.available, writes),
          optionalCount(counters.writeReads, counters.available, reads));
      break;
    }
    case Payload::CONSOLE:
      record(request, "CONSOLE", "line_capacity=%zu input_budget=%zu output_capacity=%zu"
          " record_capacity=%zu drain_budget=%zu queued=%zu available=%zu"
          " dropped_records=%" PRIu32 " dropped_bytes=%" PRIu32
          " loss_pending=%u color=%u verbose=%u", ConsoleBuffer::LINE_CAPACITY,
          ConsoleBuffer::INPUT_BUDGET, ConsoleBuffer::OUTPUT_CAPACITY,
          ConsoleBuffer::RECORD_CAPACITY, ConsoleBuffer::DRAIN_BUDGET, _console.queued(),
          _console.available(), _console.droppedRecords(), _console.droppedBytes(),
          _console.lossPending() ? 1U : 0U, _color ? 1U : 0U, _verbose ? 1U : 0U);
      break;
    case Payload::CAPABILITIES: {
      const bool is322 = _config.variant == t::Variant::TUSB322I;
      const auto& identity = diagnostics.identity;
      record(request, "CAPABILITIES", "configured_variant=%s observed_variant=%s"
          " identity_valid=%u identity_matches=%u revision_valid=%u revision_matches=%u"
          " profile_fields=%u advertisement_write=%u duty_write=%u accessory_write=%u",
          variantName(_config.variant), identity.identityValid ?
          variantName(identity.variant) : UNKNOWN, identity.identityValid ? 1U : 0U,
          identity.identityMatches ? 1U : 0U, identity.revisionValid ? 1U : 0U,
          identity.revisionMatches ? 1U : 0U,
          static_cast<unsigned>(applicableFields(_config.variant)),
          is322 ? 0U : 1U, is322 ? 0U : 1U,
          !is322 && (applicableFields(_config.variant) &
          static_cast<uint16_t>(t::ProfileField::DISABLE_UFP_ACCESSORY)) != 0 ? 1U : 0U);
      break;
    }
    case Payload::TARGET:
      record(request, "TARGET", "variant=%s address=%u bound=%u staged_reset=%u",
          variantName(_config.variant), static_cast<unsigned>(_config.address),
          diagnostics.bound ? 1U : 0U, response.stagedReset ? 1U : 0U);
      break;
    case Payload::PROFILE: profileOutput(request); break;
    case Payload::PORT:
      portOutput(request, _driver.portSnapshot(), _platform.nowMs(_platform.user));
      break;
  }
}

// Publish supplied raw bytes and public decoder results; perform no register transfer here.
void Cli::registerOutput(const Request& request, const Response& response) {
  const auto variant = std::strcmp(response.source, "write") == 0
      ? _driver.diagnostics().identity.variant : response.decoded.variant;
  record(request, "REG", "source=%s variant=%s address=%u value=%u", response.source,
      variantName(variant), static_cast<unsigned>(response.reg), static_cast<unsigned>(response.value));
  if (std::strcmp(response.source, "write") == 0) return; // Requested image, not readback/proof.
  const auto& decoded = response.decoded;
  const auto statusValid = [&decoded](t::PortField field) {
    return (decoded.statusValidFields & static_cast<uint16_t>(field)) != 0;
  };
  record(request, "DECODE", "variant=%s address=%u value=%u profile_valid_fields=%u"
      " status_valid_fields=%u attachment=%s orientation=%s detected_current=%s accessory=%s"
      " active_cable=%s interrupt_pending=%s reset_requested=%s reset_valid=%u",
      variantName(decoded.variant), static_cast<unsigned>(decoded.reg), static_cast<unsigned>(decoded.value),
      static_cast<unsigned>(decoded.profileValidFields), static_cast<unsigned>(decoded.statusValidFields),
      statusValid(t::PortField::ATTACHMENT) ? attachmentName(decoded.attachment) : "UNKNOWN",
      statusValid(t::PortField::ORIENTATION) ? orientationName(decoded.orientation) : "UNKNOWN",
      statusValid(t::PortField::DETECTED_CURRENT) ? detectedName(decoded.detectedCurrent) : "UNKNOWN",
      statusValid(t::PortField::ACCESSORY) ? accessoryName(decoded.accessory) : "UNKNOWN",
      statusValid(t::PortField::ACTIVE_CABLE) ? decoded.activeCable ? "1" : "0" : UNKNOWN,
      statusValid(t::PortField::INTERRUPT_PENDING) ? decoded.interruptPending ? "1" : "0" : UNKNOWN,
      decoded.resetValid ? decoded.resetRequested ? "1" : "0" : UNKNOWN, decoded.resetValid ? 1U : 0U);
  const auto& profile = decoded.profileCodes;
  const auto valid = [&decoded](t::ProfileField field) {
    return (decoded.profileValidFields & static_cast<uint16_t>(field)) != 0;
  };
  const auto control = [&valid](t::ProfileField field, bool disabled) {
    return valid(field) ? disabled ? "off" : "on" : UNKNOWN;
  };
  const auto disable = [&valid](t::ProfileField field, bool disabled) {
    return valid(field) ? disabled ? "1" : "0" : UNKNOWN;
  };
  // Values come only from the public decoder; these are codes without paired port applicability.
  record(request, "DECODE_PROFILE", "variant=%s address=%u valid_fields=%u"
      " mode=%s current=%s duty=%s debounce=%s preference=%s ufpaccessory=%s termination=%s"
      " rd_rp=%s disable_ufp_accessory=%s disable_term=%s disable_rd_rp=%s",
      variantName(decoded.variant), static_cast<unsigned>(decoded.reg),
      static_cast<unsigned>(decoded.profileValidFields),
      valid(t::ProfileField::PORT_MODE) ? modeName(profile.portMode) : UNKNOWN,
      valid(t::ProfileField::SOURCE_CURRENT) ? currentName(profile.sourceCurrent) : UNKNOWN,
      valid(t::ProfileField::DRP_DUTY_CYCLE) ? dutyName(profile.drpDutyCycle) : UNKNOWN,
      valid(t::ProfileField::CC_DEBOUNCE) ? debounceName(profile.ccDebounce) : UNKNOWN,
      valid(t::ProfileField::SOURCE_PREFERENCE) ? preferenceName(profile.sourcePreference) : UNKNOWN,
      control(t::ProfileField::DISABLE_UFP_ACCESSORY, profile.disableUfpAccessory),
      control(t::ProfileField::DISABLE_TERM, profile.disableTerm),
      control(t::ProfileField::DISABLE_RD_RP, profile.disableRdRp),
      disable(t::ProfileField::DISABLE_UFP_ACCESSORY, profile.disableUfpAccessory),
      disable(t::ProfileField::DISABLE_TERM, profile.disableTerm),
      disable(t::ProfileField::DISABLE_RD_RP, profile.disableRdRp));
}

void Cli::interruptOutput(const Request& request, const t::InterruptSnapshot& interrupt) {
  record(request, "IRQ", "source=managed_operation observed=%u pending_before=%s ack_attempted=%u"
      " ack_confirmed=%u pending_after=%s pending_after_valid=%u",
      interrupt.observationValid ? 1U : 0U,
      interrupt.observationValid ? interrupt.pendingBefore ? "1" : "0" : UNKNOWN,
      interrupt.acknowledgeAttempted ? 1U : 0U, interrupt.acknowledgeConfirmed ? 1U : 0U,
      interrupt.pendingAfterValid ? interrupt.pendingAfter ? "1" : "0" : UNKNOWN,
      interrupt.pendingAfterValid ? 1U : 0U);
}

void Cli::transportOutput(const Request& request, const Response& response, const t::TransportStats& stats) {
  char attempts[11], success[11], failures[11];
  record(request, "XFER", "source=all_callbacks attempts=%" PRIu32 " success=%" PRIu32
      " failures=%" PRIu32 " last_code=%s last_detail=%" PRId32 " saturated=%u assertion=%s"
      " expected_attempts=%s expected_success=%s expected_failures=%s", stats.totalAttempts,
      stats.totalSuccess, stats.totalFailures, t::errorName(stats.lastStatus.code), stats.lastStatus.detail,
      countersSaturated(stats) ? 1U : 0U, response.assertion,
      optionalCount(response.expectedAttempts, response.asserting, attempts),
      optionalCount(response.expectedSuccess, response.asserting && response.assertAll, success),
      optionalCount(response.expectedFailures, response.asserting && response.assertAll, failures));
}

void Cli::healthOutput(const Request& request, const t::HealthSnapshot& health,
    const t::DriverDiagnostics& diagnostics) {
  record(request, "DRV", "source=managed_callbacks state=%s threshold=%" PRIu32 " bound=%u initialized=%u"
      " configuration=%s effect=%s termination=%s reset=%s managed_streak=%" PRIu32,
      healthName(health.state), health.offlineThreshold, diagnostics.bound ? 1U : 0U,
      diagnostics.initialized ? 1U : 0U, configurationName(diagnostics.configurationState),
      effectName(diagnostics.hardwareEffect), terminationName(diagnostics.terminationState),
      resetName(diagnostics.resetState), health.evidence.consecutiveFailures);
}

// Emit copied trust, managed failures, identity, IRQ and all-callback totals without I2C.
void Cli::diagnosticsOutput(const Request& request, const t::DriverDiagnostics& diagnostics,
    const t::TransportStats& stats) {
  record(request, "DIAG", "bound=%u initialized=%u configuration=%s effect=%s termination=%s reset=%s"
      " config_write_unresolved=%u recovery_unresolved=%u mode_unresolved=%u reset_acceptance_uncertain=%u"
      " last_kind=%s last_code=%s last_detail=%" PRId32,
      diagnostics.bound ? 1U : 0U, diagnostics.initialized ? 1U : 0U,
      configurationName(diagnostics.configurationState), effectName(diagnostics.hardwareEffect),
      terminationName(diagnostics.terminationState), resetName(diagnostics.resetState),
      diagnostics.configurationWriteUnresolved ? 1U : 0U, diagnostics.recoverySequenceUnresolved ? 1U : 0U,
      diagnostics.modeTransitionUnresolved ? 1U : 0U, diagnostics.resetAcceptanceUncertain ? 1U : 0U,
      kindName(diagnostics.lastOperationKind), t::errorName(diagnostics.lastOperationStatus.code),
      diagnostics.lastOperationStatus.detail);
  const auto& managed = diagnostics.managedTransport;
  record(request, "MANAGED", "source=managed_callbacks initialized=%u success=%" PRIu32
      " failures=%" PRIu32 " streak=%" PRIu32 " last_code=%s last_detail=%" PRId32
      " last_success_ms=%" PRIu64 " last_success_time_valid=%u last_failure_ms=%" PRIu64
      " last_failure_time_valid=%u", managed.initialized ? 1U : 0U, managed.totalSuccess,
      managed.totalFailures, managed.consecutiveFailures, t::errorName(managed.lastStatus.code),
      managed.lastStatus.detail, managed.lastSuccessUptimeMs, managed.lastSuccessTimeValid ? 1U : 0U,
      managed.lastFailureUptimeMs, managed.lastFailureTimeValid ? 1U : 0U);
  record(request, "FAILURE", "code=%s detail=%" PRId32 " time_ms=%" PRIu64 " time_valid=%u",
      t::errorName(diagnostics.lastFailedTransportStatus.code), diagnostics.lastFailedTransportStatus.detail,
      diagnostics.lastFailedTransportUptimeMs, diagnostics.lastFailedTransportTimeValid ? 1U : 0U);
  const auto& identity = diagnostics.identity;
  char bytes[17]{};
  for (std::size_t index = 0; index < 8; ++index)
    std::snprintf(bytes + index * 2, 3, "%02X", static_cast<unsigned>(identity.rawIdentity[index]));
  char revision[4];
  record(request, "IDENTITY", "variant=%s address=%u identity_valid=%u identity_matches=%u bytes=%s"
      " revision=%s revision_valid=%u revision_matches=%u", variantName(identity.variant),
      static_cast<unsigned>(identity.address), identity.identityValid ? 1U : 0U,
      identity.identityMatches ? 1U : 0U, identity.identityValid ? bytes : UNKNOWN,
      rawByte(identity.rawRevision, identity.revisionValid, revision), identity.revisionValid ? 1U : 0U,
      identity.revisionMatches ? 1U : 0U);
  interruptOutput(request, diagnostics.interrupt);
  transportOutput(request, Response{}, stats);
}

void Cli::jobOutput(const Request& request, const t::Status& status) {
  const auto progress = _driver.progress();
  record(request, "JOB", "token=%" PRIu64 " kind=%s state=%s phase=%s code=%s detail=%" PRId32
      " callbacks=%" PRIu32
      " last_poll_callbacks=%u deadline_ms=%" PRIu64 " waiting=%u ready_at_ms=%" PRIu64
      " ready_at_valid=%u result_pending=%u auto=%u", progress.token.value,
      kindName(progress.kind), operationStateName(progress.state), phaseName(progress.phase),
      t::errorName(status.code), status.detail,
      progress.transfers, static_cast<unsigned>(progress.lastPollTransfers), progress.deadlineMs,
      progress.waiting, progress.readyAtMs, progress.readyAtValid, progress.resultPending, _automatic);
}

// Format a supplied terminal result and original request correlation without consuming its slot.
void Cli::resultOutput(const Request& request, const t::OperationResult& result) {
  const uint64_t origin = result.token.value == _operationToken.value ? _operationRequest.id : 0;
  record(request, "RESULT", "origin_request=%" PRIu64 " token=%" PRIu64
      " kind=%s state=%s code=%s detail=%" PRId32 " effect=%s callbacks=%" PRIu32
      " desired_generation=%" PRIu32 " applied_generation=%" PRIu32 " configuration=%s",
      origin, result.token.value, kindName(result.kind), operationStateName(result.state),
      t::errorName(result.status.code), result.status.detail, effectName(result.hardwareEffect),
      result.transfers, result.configuration.desiredGeneration, result.configuration.verifiedGeneration,
      configurationName(result.configuration.state));
  record(request, "RESULT_DETAIL", "origin_request=%" PRIu64 " token=%" PRIu64
      " started_ms=%" PRIu64 " completed_ms=%" PRIu64 " completed_time_valid=%u"
      " termination=%s reset=%s irq_observed=%u irq_before=%u ack_attempted=%u"
      " ack_confirmed=%u irq_after=%u irq_after_valid=%u", origin, result.token.value,
      result.startedUptimeMs, result.completedUptimeMs, result.completedTimeValid,
      terminationName(result.configuration.terminationState), resetName(result.configuration.resetState),
      result.interrupt.observationValid, result.interrupt.pendingBefore,
      result.interrupt.acknowledgeAttempted, result.interrupt.acknowledgeConfirmed,
      result.interrupt.pendingAfter, result.interrupt.pendingAfterValid);
}

// Announce the CLI-owned completion once, including on output loss; retain the core result.
void Cli::announceCompletion(const TUSB32x::OperationResult& result) {
  if (_completionAnnounced || result.token.value != _operationToken.value) return;
  // Output loss never consumes the core slot or causes repeated terminal announcements.
  _completionAnnounced = true;
  resultOutput(_operationRequest, result);
  close(_operationRequest, result.status, result.token, result.transfers, result.state);
}

// Inspect only the CLI-owned retained token and publish a newly available completion.
void Cli::announceCompletion() {
  if (!_operationToken.valid() || _completionAnnounced) return;
  t::OperationResult result;
  if (_driver.resultSnapshot(_operationToken, result).ok()) announceCompletion(result);
}

// Reserve whole-response space before an explicit take so backpressure cannot consume a result.
void Cli::deliverResult(const Request& request, Response response) {
  t::OperationResult result;
  response.status = _driver.resultSnapshot(response.token, result);
  if (response.status.ok() && response.take) {
    // Serialized ownership reserves capacity until this synchronous response is queued.
    // Five whole maximum records cover ACK, optional human status, RESULT, detail and END.
    // Three more cover a still-unannounced original completion. No second result is retained.
    const bool announce = !_completionAnnounced && result.token.value == _operationToken.value;
    const std::size_t required = (announce ? 8U : 5U) * (ConsoleBuffer::RECORD_CAPACITY - 1);
    if (_console.available() < required) {
      // Rejection is not a lost record. Only actual response enqueue failures count as loss.
      response.status = t::Status::Error(t::Err::BUSY, "Insufficient output space; result retained");
    } else {
      if (announce) announceCompletion(result);
      response.status = _driver.takeResult(response.token, result);
    }
  }
  acknowledge(request, response);
  if (response.status.ok()) resultOutput(request, result);
  close(request, response.status, response.token);
}

// Emit raw validity, generations, trust and mismatch evidence from one copied configuration.
void Cli::configurationOutput(const Request& request, const t::ConfigurationSnapshot& snapshot) {
  char raw08[4], raw09[4], raw0a[4], raw45[4];
  record(request, "CONFIG", "raw_valid_mask=%u observed_valid_fields=%u has_desired=%u"
      " has_observed=%u has_applied=%u desired_generation=%" PRIu32
      " applied_generation=%" PRIu32 " configuration=%s raw08=%s raw09=%s raw0a=%s raw45=%s",
      static_cast<unsigned>(snapshot.rawConfigValidMask),
      static_cast<unsigned>(snapshot.observedValidFields), snapshot.hasDesiredProfile ? 1U : 0U,
      snapshot.hasObservedProfile ? 1U : 0U, snapshot.hasVerifiedProfile ? 1U : 0U,
      snapshot.desiredGeneration, snapshot.verifiedGeneration, configurationName(snapshot.state),
      rawByte(snapshot.rawConfig08, (snapshot.rawConfigValidMask & t::CONFIG_08_VALID) != 0, raw08),
      rawByte(snapshot.rawConfig09, (snapshot.rawConfigValidMask & t::CONFIG_09_VALID) != 0, raw09),
      rawByte(snapshot.rawConfig0A, (snapshot.rawConfigValidMask & t::CONFIG_0A_VALID) != 0, raw0a),
      rawByte(snapshot.rawConfig45,
          (snapshot.rawConfigValidMask & t::CONFIG_45_VALID) != 0, raw45));
  char reg[4], expected[4], observed[4], mask[4];
  record(request, "CONFIG_DETAIL", "observed_ms=%" PRIu64 " observed_time_valid=%u termination=%s reset=%s"
      " mismatch_valid=%u mismatch_register=%s mismatch_expected=%s mismatch_observed=%s mismatch_mask=%s",
      snapshot.observedUptimeMs, snapshot.observedTimeValid ? 1U : 0U,
      terminationName(snapshot.terminationState), resetName(snapshot.resetState), snapshot.mismatchValid ? 1U : 0U,
      rawByte(snapshot.mismatchRegister, snapshot.mismatchValid, reg),
      rawByte(snapshot.mismatchExpected, snapshot.mismatchValid, expected),
      rawByte(snapshot.mismatchObserved, snapshot.mismatchValid, observed),
      rawByte(snapshot.mismatchMask, snapshot.mismatchValid, mask));
}

// Keep staged, desired, observed and applied profiles tied to their own variant and valid fields.
void Cli::profileOutput(const Request& request) {
  const auto snapshot = _driver.configurationSnapshot();
  configurationOutput(request, snapshot);
  const t::Variant retainedVariant = _driver.diagnostics().identity.variant;
  const uint16_t stagedFields = applicableFields(_config.variant);
  const uint16_t retainedFields = applicableFields(retainedVariant);
  // Target edits change only CLI staging. Core profiles retain their original association.
  const struct {
    const char* name;
    t::Variant variant;
    const t::DeviceProfile* profile;
    bool available;
    uint32_t generation;
    uint16_t fields;
  } sources[] = {
      {"staged", _config.variant, &_staged, true, _stagedGeneration, stagedFields},
      {"desired", retainedVariant, &snapshot.desiredProfile, snapshot.hasDesiredProfile,
          snapshot.desiredGeneration, snapshot.hasDesiredProfile ? retainedFields : uint16_t{0}},
      {"observed", retainedVariant, &snapshot.observedProfile, snapshot.rawConfigValidMask != 0, 0,
          snapshot.observedValidFields},
      {"applied", retainedVariant, &snapshot.verifiedProfile, snapshot.hasVerifiedProfile,
          snapshot.verifiedGeneration, snapshot.hasVerifiedProfile ? retainedFields : uint16_t{0}},
  };
  for (const auto& source : sources) {
    const auto& profile = *source.profile;
    const auto valid = [&source](t::ProfileField field) {
      return (source.fields & static_cast<uint16_t>(field)) != 0;
    };
    const auto control = [&valid](t::ProfileField field, bool disabled) {
      return valid(field) ? disabled ? "off" : "on" : UNKNOWN;
    };
    const auto disable = [&valid](t::ProfileField field, bool disabled) {
      return valid(field) ? disabled ? "1" : "0" : UNKNOWN;
    };
    record(request, "PROFILE", "source=%s variant=%s available=%u generation=%" PRIu32
        " valid_fields=%u mode=%s current=%s duty=%s debounce=%s preference=%s"
        " ufpaccessory=%s termination=%s rd_rp=%s disable_ufp_accessory=%s"
        " disable_term=%s disable_rd_rp=%s", source.name, variantName(source.variant),
        source.available ? 1U : 0U,
        source.generation, static_cast<unsigned>(source.fields),
        valid(t::ProfileField::PORT_MODE) ? modeName(profile.portMode) : UNKNOWN,
        valid(t::ProfileField::SOURCE_CURRENT) ? currentName(profile.sourceCurrent) : UNKNOWN,
        valid(t::ProfileField::DRP_DUTY_CYCLE) ? dutyName(profile.drpDutyCycle) : UNKNOWN,
        valid(t::ProfileField::CC_DEBOUNCE) ? debounceName(profile.ccDebounce) : UNKNOWN,
        valid(t::ProfileField::SOURCE_PREFERENCE) ?
        preferenceName(profile.sourcePreference) : UNKNOWN,
        control(t::ProfileField::DISABLE_UFP_ACCESSORY, profile.disableUfpAccessory),
        control(t::ProfileField::DISABLE_TERM, profile.disableTerm),
        control(t::ProfileField::DISABLE_RD_RP, profile.disableRdRp),
        disable(t::ProfileField::DISABLE_UFP_ACCESSORY, profile.disableUfpAccessory),
        disable(t::ProfileField::DISABLE_TERM, profile.disableTerm),
        disable(t::ProfileField::DISABLE_RD_RP, profile.disableRdRp));
  }
}

// Format valid cached port fields and rollover-safe age; retain unknown or stale evidence.
void Cli::portOutput(const Request& request, const t::PortSnapshot& port, uint64_t nowMs) {
  const auto valid = [&port](t::PortField field) {
    return (port.validFields & static_cast<uint16_t>(field)) != 0;
  };
  char raw08[4], raw09[4], age[21];
  const uint64_t elapsed = nowMs - port.observedUptimeMs;
  const char* ageText = UNKNOWN;
  if (port.observedTimeValid && elapsed < (uint64_t{1} << 63)) {
    std::snprintf(age, sizeof(age), "%" PRIu64, elapsed);
    ageText = age;
  }
  record(request, "PORT", "raw08=%s raw09=%s raw_valid_mask=%u valid_fields=%u invalidated=%u"
      " attachment=%s local_role=%s peer_role=%s orientation=%s detected_current=%s"
      " sequence=%" PRIu64 " observed_ms=%" PRIu64 " observed_time_valid=%u age_ms=%s",
      rawByte(port.rawStatus08, (port.rawValidMask & t::STATUS_08_VALID) != 0, raw08),
      rawByte(port.rawStatus09, (port.rawValidMask & t::STATUS_09_VALID) != 0, raw09),
      static_cast<unsigned>(port.rawValidMask), static_cast<unsigned>(port.validFields),
      port.invalidated ? 1U : 0U, valid(t::PortField::ATTACHMENT) ?
      attachmentName(port.attachment) : "UNKNOWN", valid(t::PortField::LOCAL_ROLE) ?
      roleName(port.localRole) : "UNKNOWN", valid(t::PortField::PEER_ROLE) ?
      roleName(port.peerRole) : "UNKNOWN", valid(t::PortField::ORIENTATION) ?
      orientationName(port.orientation) : "UNKNOWN", valid(t::PortField::DETECTED_CURRENT) ?
      detectedName(port.detectedCurrent) : "UNKNOWN", port.sequence, port.observedUptimeMs,
      port.observedTimeValid ? 1U : 0U, ageText);
  record(request, "PORT_DETAIL", "advertised_current=%s accessory=%s active_cable=%s"
      " interrupt_pending=%s config_generation=%" PRIu32 " configuration=%s coherence=%s",
      valid(t::PortField::ADVERTISED_CURRENT) ? currentName(port.advertisedCurrent) : UNKNOWN,
      valid(t::PortField::ACCESSORY) ? accessoryName(port.accessory) : "UNKNOWN",
      valid(t::PortField::ACTIVE_CABLE) ? port.activeCable ? "1" : "0" : UNKNOWN,
      valid(t::PortField::INTERRUPT_PENDING) ? port.interruptPending ? "1" : "0" : UNKNOWN,
      port.configGeneration, configurationName(port.configurationState),
      coherenceName(port.coherence));
}

// Queue at most two help output attempts per owner turn while walking the fixed command table.
void Cli::helpStep() {
  if (!_helpActive) return;
  std::size_t count = 0;
  const CommandRow* rows = commandRows(count);
  unsigned attempts = 0;
  if (!_helpTitle) {
    human("%s=== TUSB32x CLI Help ===%s\n", _color ? "\033[36m" : "",
        _color ? "\033[0m" : "");
    _helpTitle = true;
    ++attempts;
  }
  while (_helpIndex < count && attempts < 2) {
    const auto& row = rows[_helpIndex];
    if (std::strcmp(_helpFilter, "all") != 0 && std::strcmp(_helpFilter, row.topic) != 0 &&
        std::strcmp(_helpFilter, row.command) != 0) {
      ++_helpIndex;
      continue;
    }
    if (!_helpSection || std::strcmp(_helpSection, row.topic) != 0) {
      human("%s[%s]%s\n", _color ? "\033[32m" : "", sectionName(row.topic),
          _color ? "\033[0m" : "");
      _helpSection = row.topic;
      ++attempts;
      continue;
    }
    if (attempts != 0) return;
    human("%s%-32s%s - %s\n", _color ? "\033[36m" : "", row.syntax,
        _color ? "\033[0m" : "", row.description);
    record(_helpRequest, "HELP", "canonical=%s aliases=%s topic=%s execution=%s effect=%s"
        " prerequisites=%s capability=%s records=%s max_callbacks=%u tx_bytes=%u rx_bytes=%u"
        " fixture=%s test=%s", row.command, row.aliases, row.topic, row.execution, row.effect,
        row.prerequisites, row.capability, row.records, static_cast<unsigned>(row.maxCallbacks),
        static_cast<unsigned>(row.txBytes), static_cast<unsigned>(row.rxBytes),
        row.fixture, row.test);
    ++_helpIndex;
    attempts += 2;
  }
  if (_helpIndex == count && attempts < 2) {
    close(_helpRequest, t::Status::Ok());
    _helpActive = false;
  }
}
}  // namespace tusb32x_cli
