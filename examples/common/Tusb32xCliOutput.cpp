/// @file Tusb32xCliOutput.cpp
/// @brief Bounded local CLI records, cached profile/port rendering and incremental help.
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

const char* configurationName(t::ConfigurationState value) {
  switch (value) {
    case t::ConfigurationState::UNCONFIGURED: return "UNCONFIGURED";
    case t::ConfigurationState::APPLYING: return "APPLYING";
    case t::ConfigurationState::KNOWN: return "KNOWN";
    case t::ConfigurationState::UNKNOWN: return "UNKNOWN";
  }
  return "UNKNOWN";
}

uint16_t applicableFields(t::Variant variant) {
  if (variant == t::Variant::TUSB320 || variant == t::Variant::TUSB320I) return 0x9B;
  if (variant == t::Variant::TUSB322I) return 0x7F;
  if (variant == t::Variant::TUSB320LI || variant == t::Variant::TUSB320HI ||
      variant == t::Variant::TUSB320LAI || variant == t::Variant::TUSB320HAI) return 0xFF;
  return 0;
}

const char* modeName(t::PortMode value) {
  switch (value) {
    case t::PortMode::FOLLOW_PORT_PIN: return "port";
    case t::PortMode::DFP: return "dfp";
    case t::PortMode::UFP: return "ufp";
    case t::PortMode::DRP: return "drp";
  }
  return UNKNOWN;
}

const char* currentName(t::SourceCurrent value) {
  switch (value) {
    case t::SourceCurrent::DEFAULT: return "default";
    case t::SourceCurrent::MA_1500: return "1a5";
    case t::SourceCurrent::MA_3000: return "3a0";
  }
  return UNKNOWN;
}

const char* preferenceName(t::SourcePreference value) {
  switch (value) {
    case t::SourcePreference::NORMAL: return "normal";
    case t::SourcePreference::TRY_SNK: return "try-sink";
    case t::SourcePreference::TRY_SRC: return "try-source";
  }
  return UNKNOWN;
}

const char* dutyName(t::DrpDutyCycle value) {
  switch (value) {
    case t::DrpDutyCycle::PERCENT_30: return "30";
    case t::DrpDutyCycle::PERCENT_40: return "40";
    case t::DrpDutyCycle::PERCENT_50: return "50";
    case t::DrpDutyCycle::PERCENT_60: return "60";
  }
  return UNKNOWN;
}

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

const char* roleName(t::PortRole value) {
  switch (value) {
    case t::PortRole::UNKNOWN: return "UNKNOWN";
    case t::PortRole::NOT_APPLICABLE: return "NOT_APPLICABLE";
    case t::PortRole::SOURCE: return "SOURCE";
    case t::PortRole::SINK: return "SINK";
  }
  return "UNKNOWN";
}

const char* orientationName(t::Orientation value) {
  switch (value) {
    case t::Orientation::UNKNOWN: return "UNKNOWN";
    case t::Orientation::NOT_APPLICABLE: return "NOT_APPLICABLE";
    case t::Orientation::CC1: return "CC1";
    case t::Orientation::CC2: return "CC2";
  }
  return "UNKNOWN";
}

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

const char* coherenceName(t::SnapshotCoherence value) {
  switch (value) {
    case t::SnapshotCoherence::SEQUENTIAL: return "SEQUENTIAL";
    case t::SnapshotCoherence::BOOKENDS_MATCH: return "BOOKENDS_MATCH";
    case t::SnapshotCoherence::CHANGED_DURING_READ: return "CHANGED_DURING_READ";
  }
  return "UNKNOWN";
}

const char* sectionName(const char* topic) {
  if (std::strcmp(topic, "common") == 0) return "Common";
  if (std::strcmp(topic, "connection") == 0) return "Connection";
  if (std::strcmp(topic, "port") == 0) return "Port";
  if (std::strcmp(topic, "profile") == 0) return "Profile";
  if (std::strcmp(topic, "lifecycle") == 0) return "Lifecycle";
  return topic;
}

const char* rawByte(uint8_t byte, bool valid, char (&buffer)[4]) {
  if (!valid) return UNKNOWN;
  std::snprintf(buffer, sizeof(buffer), "%u", static_cast<unsigned>(byte));
  return buffer;
}

const char* optionalCount(uint32_t value, bool valid, char (&buffer)[11]) {
  if (!valid) return UNKNOWN;
  std::snprintf(buffer, sizeof(buffer), "%" PRIu32, value);
  return buffer;
}

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

void Cli::human(const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  const int length = std::vsnprintf(_record, sizeof(_record), format, arguments);
  va_end(arguments);
  if (length < 0) _console.enqueue(nullptr, sizeof(_record));
  else _console.enqueue(_record, static_cast<std::size_t>(length));
}

void Cli::acknowledge(const Request& request, const Response& response) {
  const auto config = _driver.configurationSnapshot();
  record(request, "ACK", "command=%s code=%s detail=%" PRId32
      " action=%s staged_generation=%" PRIu32 " desired_generation=%" PRIu32
      " applied_generation=%" PRIu32 " configuration=%s", request.command,
      t::errorName(response.status.code), response.status.detail,
      response.status.ok() ? response.action : "UNCHANGED",
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

void Cli::close(const Request& request, const t::Status& status) {
  record(request, "END", "command=%s outcome=%s code=%s detail=%" PRId32 " callbacks=0",
      request.command, status.ok() ? "SUCCEEDED" : "FAILED", t::errorName(status.code),
      status.detail);
}

void Cli::payload(const Request& request, const Response& response) {
  const auto diagnostics = _driver.diagnostics();
  switch (response.payload) {
    case Payload::NONE: break;
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

void Cli::profileOutput(const Request& request) {
  const auto snapshot = _driver.configurationSnapshot();
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
