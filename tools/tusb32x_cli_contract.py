"""Frozen executable CLI rows and schema-1 parsing; no I/O or device side effects.

C++ owns command dispatch. Transcript owners separately check request ordering,
nonce attribution and exactly one matching terminal closure. This module covers
the implemented local and cooperative-operation routes only.
"""

from collections.abc import Mapping
import re

PREFIX = ("schema", "nonce", "request")
SCHEMAS = {
    "ACK": PREFIX + (
        "command", "code", "detail", "action", "token", "staged_generation",
        "desired_generation", "applied_generation", "configuration",
    ),
    "END": PREFIX + ("command", "outcome", "code", "detail", "token", "callbacks"),
    "JOB": PREFIX + (
        "token", "kind", "state", "phase", "code", "detail", "callbacks", "last_poll_callbacks",
        "deadline_ms", "waiting", "ready_at_ms", "ready_at_valid", "result_pending", "auto",
    ),
    "RESULT": PREFIX + (
        "origin_request", "token", "kind", "state", "code", "detail", "effect", "callbacks",
        "desired_generation", "applied_generation", "configuration",
    ),
    "RESULT_DETAIL": PREFIX + (
        "origin_request", "token", "started_ms", "completed_ms", "completed_time_valid",
        "termination", "reset", "irq_observed", "irq_before", "ack_attempted",
        "ack_confirmed", "irq_after", "irq_after_valid",
    ),
    "MARK": PREFIX,
    "SESSION": ("schema", "code", "detail", "reason"),
    "LOSS": ("schema", "dropped_records", "dropped_bytes"),
    "VERSION": PREFIX + ("library",),
    "FIRMWARE": PREFIX + ("git", "tree", "build"),
    "FRAMEWORK": PREFIX + ("name", "version", "target"),
    "BUS": PREFIX + (
        "variant", "address", "bound", "bus", "framing", "bus_hz", "timeout_ms",
        "sda", "scl", "irq", "adapter_available", "adapter_writes", "adapter_write_reads",
    ),
    "CONSOLE": PREFIX + (
        "line_capacity", "input_budget", "output_capacity", "record_capacity",
        "drain_budget", "queued", "available", "dropped_records", "dropped_bytes",
        "loss_pending", "color", "verbose",
    ),
    "CAPABILITIES": PREFIX + (
        "configured_variant", "observed_variant", "identity_valid", "identity_matches",
        "revision_valid", "revision_matches", "profile_fields", "advertisement_write",
        "duty_write", "accessory_write",
    ),
    "TARGET": PREFIX + ("variant", "address", "bound", "staged_reset"),
    "CONFIG": PREFIX + (
        "raw_valid_mask", "observed_valid_fields", "has_desired", "has_observed",
        "has_applied", "desired_generation", "applied_generation", "configuration",
        "raw08", "raw09", "raw0a", "raw45",
    ),
    "PROFILE": PREFIX + (
        "source", "variant", "available", "generation", "valid_fields", "mode", "current",
        "duty", "debounce", "preference", "ufpaccessory", "termination", "rd_rp",
        "disable_ufp_accessory", "disable_term", "disable_rd_rp",
    ),
    "PORT": PREFIX + (
        "raw08", "raw09", "raw_valid_mask", "valid_fields", "invalidated", "attachment",
        "local_role", "peer_role", "orientation", "detected_current", "sequence",
        "observed_ms", "observed_time_valid", "age_ms",
    ),
    "PORT_DETAIL": PREFIX + (
        "advertised_current", "accessory", "active_cable", "interrupt_pending",
        "config_generation", "configuration", "coherence",
    ),
    "HELP": PREFIX + (
        "canonical", "aliases", "topic", "execution", "effect", "prerequisites",
        "capability", "records", "max_callbacks", "tx_bytes", "rx_bytes", "fixture", "test",
    ),
}


def _row(command, aliases, topic, syntax, execution="PURE", effect="LOCAL_STATE",
         prerequisites="none", capability="all", records="ACK,END", *, operation=False):
    """Build fixed host metadata; no firmware code generation."""
    return {
        "command": command, "aliases": aliases, "topic": topic, "syntax": syntax,
        "execution": execution, "effect": effect, "prerequisites": prerequisites,
        "capability": capability, "records": records,
        "max_callbacks": "1" if execution == "CORE_JOB" else "0",
        "tx_bytes": "2" if execution == "CORE_JOB" else "0",
        "rx_bytes": "8" if execution == "CORE_JOB" else "0",
        "fixture": "host", "test": "operation_routes" if operation else "local_routes",
    }


PROFILE_RECORDS = "ACK,CONFIG,PROFILE,END"
OPERATION_RECORDS = "ACK,JOB,RESULT,RESULT_DETAIL,END"
ROWS = (
    _row("help", "?", "common", "help [topic|command]", "CACHE_ONLY", "NONE",
         records="ACK,HELP,END"),
    _row("version", "ver", "common", "version", "CACHE_ONLY", "NONE",
         records="ACK,VERSION,FIRMWARE,FRAMEWORK,END"),
    _row("color", "none", "common", "color [on|off|0|1]", records="ACK,CONSOLE,END"),
    _row("verbose", "none", "common", "verbose [0|1]", records="ACK,CONSOLE,END"),
    _row("hilmark", "none", "common", "hilmark <nonce>", records="MARK,END"),
    _row("bus", "none", "connection", "bus", "CACHE_ONLY", "NONE", records="ACK,BUS,END"),
    _row("console", "none", "connection", "console", "CACHE_ONLY", "NONE",
         records="ACK,CONSOLE,END"),
    _row("capabilities", "none", "connection", "capabilities", "CACHE_ONLY", "NONE",
         records="ACK,CAPABILITIES,END"),
    _row("variant", "model", "connection", "variant [name [address]]",
         prerequisites="ended_idle_no_result", capability="valid_pair", records="ACK,TARGET,END"),
    _row("addr", "none", "connection", "addr [address]",
         prerequisites="ended_idle_no_result", capability="valid_pair", records="ACK,TARGET,END"),
    _row("bind", "none", "lifecycle", "bind", prerequisites="idle_no_result",
         capability="valid_pair", records="ACK,TARGET,END"),
    _row("end", "unbind", "lifecycle", "end", records="ACK,TARGET,END"),
    _row("forget-evidence", "none", "lifecycle", "forget-evidence",
         prerequisites="ended_idle_no_result", records="ACK,TARGET,END"),
    _row("sample", "snapshot", "port", "sample", "CACHE_ONLY", "NONE",
         records="ACK,PORT,PORT_DETAIL,END"),
    _row("freshness", "none", "port", "freshness", "CACHE_ONLY", "NONE",
         records="ACK,PORT,PORT_DETAIL,END"),
    _row("profile.show", "cfg,settings", "profile", "profile [show]", "CACHE_ONLY", "NONE",
         records=PROFILE_RECORDS),
    _row("profile.reset", "none", "profile", "profile reset", capability="variant_values",
         records=PROFILE_RECORDS),
    _row("profile.discard", "none", "profile", "profile discard",
         prerequisites="retained_desired", capability="variant_values", records=PROFILE_RECORDS),
    _row("profile.validate", "none", "profile", "profile validate", effect="NONE",
         capability="variant_values", records=PROFILE_RECORDS),
    _row("mode", "none", "profile", "mode [port|dfp|ufp|drp]",
         capability="variant_values", records=PROFILE_RECORDS),
    _row("current", "none", "profile", "current [default|1a5|3a0]",
         capability="variant_values", records=PROFILE_RECORDS),
    _row("duty", "none", "profile", "duty [30|40|50|60]",
         capability="variant_values", records=PROFILE_RECORDS),
    _row("debounce", "none", "profile", "debounce [milliseconds]",
         capability="variant_values", records=PROFILE_RECORDS),
    _row("preference", "none", "profile", "preference [normal|try-sink|try-source]",
         capability="extended", records=PROFILE_RECORDS),
    _row("ufpaccessory", "none", "profile", "ufpaccessory [on|off]",
         capability="extended", records=PROFILE_RECORDS),
    _row("termination", "none", "profile", "termination [on|off]",
         capability="extended", records=PROFILE_RECORDS),
    _row("rd-rp", "none", "profile", "rd-rp [on|off]", capability="has_45",
         records=PROFILE_RECORDS),
    _row("init", "initialize,probe", "connection", "init [timeout_ms]", "CORE_JOB",
         "READ_OBSERVATION", "bound", records=OPERATION_RECORDS, operation=True),
    _row("read", "status", "port", "read", "CORE_JOB", "READ_OBSERVATION", "bound",
         records=OPERATION_RECORDS, operation=True),
    _row("profile.read", "none", "profile", "profile read [timeout_ms]", "CORE_JOB",
         "READ_OBSERVATION", "bound", records=OPERATION_RECORDS, operation=True),
    _row("apply", "none", "profile", "apply [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
         "known_actual", records=OPERATION_RECORDS, operation=True),
    _row("verify", "selfcheck,selftest", "profile", "verify [timeout_ms]", "CORE_JOB",
         "READ_OBSERVATION", "retained_desired", records=OPERATION_RECORDS, operation=True),
    _row("reconcile", "none", "profile", "reconcile [timeout_ms]", "CORE_JOB",
         "READ_OBSERVATION", "bound", records=OPERATION_RECORDS, operation=True),
    _row("irq.read", "none", "interrupt", "irq read", "CORE_JOB", "READ_OBSERVATION",
         "bound", records=OPERATION_RECORDS, operation=True),
    _row("irq.ack", "none", "interrupt", "irq ack [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
         "bound", "irq_ack", OPERATION_RECORDS, operation=True),
    _row("irq.service", "none", "interrupt", "irq service [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
         "bound", "irq_ack", OPERATION_RECORDS, operation=True),
    _row("reset", "none", "lifecycle", "reset [timeout_ms]", "CORE_JOB", "DEVICE_WRITE",
         "bound", records=OPERATION_RECORDS, operation=True),
    _row("recover-dead-battery", "none", "lifecycle", "recover-dead-battery [timeout_ms]",
         "CORE_JOB", "DEVICE_WRITE", "base_stable_vdd", "base_only", OPERATION_RECORDS,
         operation=True),
    _row("job.status", "progress", "job", "job [status]", "CACHE_ONLY", "NONE",
         records="ACK,JOB,END", operation=True),
    _row("job.auto", "none", "job", "job auto [0|1]", records="ACK,JOB,END", operation=True),
    _row("job.step", "none", "job", "job step [0..255]", "CORE_JOB", "DEVICE_WRITE",
         "represented_operation", records="ACK,JOB,END", operation=True),
    _row("cancel", "stop,job.cancel", "job", "cancel [token]",
         prerequisites="represented_operation", operation=True),
    _row("result", "job.result", "job", "result [token]", "CACHE_ONLY", "NONE",
         "retained_result", records="ACK,RESULT,RESULT_DETAIL,END", operation=True),
    _row("result.take", "none", "job", "result take <token>", prerequisites="retained_result",
         records="ACK,RESULT,RESULT_DETAIL,END", operation=True),
)
COMMANDS = frozenset(row["command"] for row in ROWS)
ERRORS = frozenset((
    "OK", "NOT_BOUND", "NOT_INITIALIZED", "INVALID_CONFIG", "INVALID_PARAM",
    "UNSUPPORTED_FEATURE", "BUSY", "IN_PROGRESS", "RESULT_PENDING", "RESULT_NOT_AVAILABLE",
    "TOKEN_MISMATCH", "TOKEN_EXHAUSTED", "TIMEOUT", "CANCELLED", "CONFIGURATION_UNKNOWN",
    "CONFIGURATION_MISMATCH", "CHIP_ID_MISMATCH", "I2C_ERROR", "I2C_NACK_ADDR",
    "I2C_NACK_DATA", "I2C_TIMEOUT", "I2C_BUS", "I2C_BUSY",
))
VARIANTS = frozenset((
    "tusb320", "tusb320i", "tusb320li", "tusb320hi", "tusb320lai", "tusb320hai", "tusb322i",
))
CONFIGURATIONS = frozenset(("UNCONFIGURED", "APPLYING", "KNOWN", "UNKNOWN"))
OPERATION_KINDS = frozenset((
    "NONE", "INITIALIZE", "READ_PROFILE", "APPLY_PROFILE", "VERIFY_PROFILE", "RECONCILE",
    "REFRESH_PORT", "ACKNOWLEDGE_INTERRUPT", "SERVICE_INTERRUPT", "RESET", "RECOVER_DEAD_BATTERY",
))
TERMINAL_STATES = frozenset(("SUCCEEDED", "FAILED", "CANCELLED", "TIMED_OUT"))
OPERATION_PHASES = frozenset(("NONE", "READING", "WRITING", "WAITING", "FINISHING"))
EFFECTS = frozenset(("NONE", "CONFIRMED", "PARTIAL", "INDETERMINATE"))
_UINT = re.compile(r"(?:0|[1-9][0-9]*)\Z", re.ASCII)
_SINT = re.compile(r"-?(?:0|[1-9][0-9]*)\Z", re.ASCII)
_KEY = re.compile(r"[a-z][a-z0-9_]*\Z", re.ASCII)
_VALUE = re.compile(r"[!-<>-~]+\Z", re.ASCII)  # Visible ASCII except '='.
_NONCE = re.compile(r"[A-Za-z0-9_-]{1,32}\Z", re.ASCII)
_BOOL = frozenset((
    "bound", "adapter_available", "loss_pending", "color", "verbose", "identity_valid",
    "identity_matches", "revision_valid", "revision_matches", "advertisement_write",
    "duty_write", "accessory_write", "staged_reset", "has_desired", "has_observed",
    "has_applied", "available", "invalidated", "observed_time_valid", "waiting",
    "ready_at_valid", "result_pending", "auto", "completed_time_valid", "irq_observed",
    "irq_before", "ack_attempted", "ack_confirmed", "irq_after", "irq_after_valid",
))
_U32 = frozenset((
    "staged_generation", "desired_generation", "applied_generation", "generation",
    "config_generation", "dropped_records", "dropped_bytes", "bus_hz", "timeout_ms", "callbacks",
))
_U16 = frozenset(("profile_fields", "observed_valid_fields", "valid_fields"))
_U8 = frozenset(("address", "raw_valid_mask", "max_callbacks", "tx_bytes", "rx_bytes",
                 "last_poll_callbacks"))
_BOUNDS = {
    "line_capacity": 192, "input_budget": 64, "output_capacity": 8192,
    "record_capacity": 512, "drain_budget": 256, "queued": 8192, "available": 8192,
}
_PROFILE_ENUMS = {
    "source": {"staged", "desired", "observed", "applied"},
    "mode": {"port", "dfp", "ufp", "drp", "unknown"},
    "current": {"default", "1a5", "3a0", "unknown"},
    "duty": {"30", "40", "50", "60", "unknown"},
    "debounce": {"116", "118", "133", "134", "151", "152", "168", "unknown"},
    "preference": {"normal", "try-sink", "try-source", "unknown"},
    "ufpaccessory": {"on", "off", "unknown"}, "termination": {"on", "off", "unknown"},
    "rd_rp": {"on", "off", "unknown"}, "disable_ufp_accessory": {"0", "1", "unknown"},
    "disable_term": {"0", "1", "unknown"}, "disable_rd_rp": {"0", "1", "unknown"},
}
_PORT_ENUMS = {
    "attachment": {"UNKNOWN", "DETACHED", "LOCAL_SOURCE", "LOCAL_SINK", "ACCESSORY"},
    "local_role": {"UNKNOWN", "NOT_APPLICABLE", "SOURCE", "SINK"},
    "peer_role": {"UNKNOWN", "NOT_APPLICABLE", "SOURCE", "SINK"},
    "orientation": {"UNKNOWN", "NOT_APPLICABLE", "CC1", "CC2"},
    "detected_current": {
        "UNKNOWN", "NOT_APPLICABLE", "DEFAULT", "MA_1500", "MA_3000", "CHARGE_THROUGH_500_MA",
    },
}
_DETAIL_ENUMS = {
    "advertised_current": {"default", "1a5", "3a0", "unknown"},
    "accessory": {
        "UNKNOWN", "NOT_APPLICABLE", "NONE", "AUDIO", "CHARGE_THROUGH", "DEBUG",
        "DEBUG_LOCAL_DFP", "DEBUG_LOCAL_UFP",
    },
    "active_cable": {"0", "1", "unknown"}, "interrupt_pending": {"0", "1", "unknown"},
    "coherence": {"SEQUENTIAL", "BOOKENDS_MATCH", "CHANGED_DURING_READ", "UNKNOWN"},
}


def _unsigned(value: str, maximum: int, minimum: int = 0) -> None:
    """Reject signs, leading zeroes, junk and integers outside inclusive bounds."""
    if not _UINT.fullmatch(value) or not minimum <= int(value) <= maximum:
        raise ValueError(f"invalid unsigned value: {value!r}")


def _signed(value: str) -> None:
    """Require a complete signed decimal int32, preserving full backend detail."""
    if not _SINT.fullmatch(value) or not -(1 << 31) <= int(value) < (1 << 31):
        raise ValueError(f"invalid signed detail: {value!r}")


def validate_record(record: Mapping[str, str]) -> None:
    """Validate a parsed mapping containing 'type'; raise ValueError on failure.

    Required keys and enum spellings are strict. Additional ASCII token fields
    are retained for additive evolution. No transcript ordering is established.
    """
    kind = record.get("type", "")
    if kind not in SCHEMAS:
        raise ValueError(f"unknown record type: {kind!r}")
    missing = set(SCHEMAS[kind]) - record.keys()
    if missing:
        raise ValueError(f"missing {kind} keys: {sorted(missing)}")
    for key, value in record.items():
        if key != "type" and (not _KEY.fullmatch(key) or not _VALUE.fullmatch(value)):
            raise ValueError(f"invalid key/value token: {key!r}={value!r}")
    if record["schema"] != "1":
        raise ValueError("unsupported schema")
    if kind not in {"SESSION", "LOSS"}:
        _unsigned(record["request"], (1 << 64) - 1, 1)
        if not _NONCE.fullmatch(record["nonce"]):
            raise ValueError("invalid nonce")
        if kind == "MARK" and record["nonce"] == "none":
            raise ValueError("reserved marker nonce")
    elif "request" in record or "nonce" in record:
        raise ValueError("session/loss records have no request attribution")
    for key, value in record.items():
        if key in _BOOL and not (key == "available" and kind == "CONSOLE"):
            if value not in {"0", "1"}:
                raise ValueError(f"invalid boolean: {key}")
        elif key in _U32:
            _unsigned(value, (1 << 32) - 1)
        elif key in _U16:
            _unsigned(value, (1 << 16) - 1)
        elif key in _U8:
            _unsigned(value, 255)
        elif key in {"detail", "sda", "scl", "irq"}:
            _signed(value)
        elif key in {"sequence", "observed_ms", "token", "origin_request", "deadline_ms",
                     "ready_at_ms", "started_ms", "completed_ms"}:
            _unsigned(value, (1 << 64) - 1)
        elif key == "age_ms" and value != "unknown":
            _unsigned(value, (1 << 63) - 1)
        elif key in {"raw08", "raw09", "raw0a", "raw45"} and value != "unknown":
            _unsigned(value, 255)
        elif key in {"adapter_writes", "adapter_write_reads"} and value != "unknown":
            _unsigned(value, (1 << 32) - 1)
    if "configuration" in record and record["configuration"] not in CONFIGURATIONS:
        raise ValueError("invalid configuration state")
    if "code" in record and record["code"] not in ERRORS:
        raise ValueError("invalid Err name")
    if kind in {"ACK", "END"} and record["command"] not in COMMANDS | {"invalid"}:
        raise ValueError("noncanonical command")
    if kind == "ACK":
        if record["action"] not in {"QUERY", "LOCAL", "UNCHANGED", "STAGED", "ADMITTED"}:
            raise ValueError("invalid input action")
        admitted = record["action"] == "ADMITTED"
        if admitted != (record["code"] == "IN_PROGRESS"):
            raise ValueError("operation admission requires IN_PROGRESS")
        if admitted and record["token"] == "0":
            raise ValueError("admitted operation requires a nonzero token")
        if record["code"] not in {"OK", "IN_PROGRESS"} and record["action"] != "UNCHANGED":
            raise ValueError("failed command must leave state unchanged")
    if kind == "END":
        if record["outcome"] not in TERMINAL_STATES:
            raise ValueError("invalid terminal outcome")
        if (record["outcome"] == "SUCCEEDED") != (record["code"] == "OK"):
            raise ValueError("contradictory terminal status")
        if record["outcome"] == "CANCELLED" and record["code"] != "CANCELLED":
            raise ValueError("contradictory cancelled status")
        if record["outcome"] == "TIMED_OUT" and record["code"] != "TIMEOUT":
            raise ValueError("contradictory expired status")
    if kind in {"JOB", "RESULT"}:
        if record["kind"] not in OPERATION_KINDS:
            raise ValueError("invalid operation kind")
        states = TERMINAL_STATES | {"IDLE", "ACTIVE"} if kind == "JOB" else TERMINAL_STATES
        if record["state"] not in states:
            raise ValueError("invalid operation state")
        if kind == "JOB" and record["phase"] not in OPERATION_PHASES:
            raise ValueError("invalid operation phase")
        if kind == "RESULT":
            if record["token"] == "0" or record["effect"] not in EFFECTS:
                raise ValueError("invalid terminal operation evidence")
            if (record["state"] == "SUCCEEDED") != (record["code"] == "OK"):
                raise ValueError("contradictory operation status")
            if record["state"] == "CANCELLED" and record["code"] != "CANCELLED":
                raise ValueError("contradictory cancelled operation status")
            if record["state"] == "TIMED_OUT" and record["code"] != "TIMEOUT":
                raise ValueError("contradictory expired operation status")
    if kind == "RESULT_DETAIL" and (
        record["token"] == "0" or record["termination"] not in {"UNKNOWN", "ENABLED", "DISABLED"}
        or record["reset"] not in {"NONE", "MAY_BE_PENDING", "COMPLETED"}
    ):
        raise ValueError("invalid operation detail")
    if kind == "SESSION" and (record["code"], record["detail"], record["reason"]) != (
        "TOKEN_EXHAUSTED", "0", "request_exhausted"
    ):
        raise ValueError("invalid session exhaustion")
    if kind in {"BUS", "TARGET"}:
        _unsigned(record["address"], 127)
    if kind == "BUS":
        _unsigned(record["timeout_ms"], (1 << 32) - 1, 1)
    lengths = (
        {"git": 40, "tree": 8, "build": 64} if kind == "FIRMWARE" else
        {"name": 32, "version": 32, "target": 32} if kind == "FRAMEWORK" else
        {"bus": 32, "framing": 32} if kind == "BUS" else {}
    )
    for key, maximum in lengths.items():
        if len(record[key]) > maximum:
            raise ValueError(f"oversized identity token: {key}")
    if kind in {"BUS", "TARGET"} and record["variant"] not in VARIANTS:
        raise ValueError("invalid selected variant")
    if kind == "PROFILE" and record["variant"] not in VARIANTS | {"unspecified", "unknown"}:
        raise ValueError("invalid profile attribution")
    if kind == "CAPABILITIES" and (
        record["configured_variant"] not in VARIANTS or record["observed_variant"] not in (
            VARIANTS | {"unknown", "unspecified"}
        )
    ):
        raise ValueError("invalid capability variant")
    enums = _PROFILE_ENUMS if kind == "PROFILE" else _PORT_ENUMS if kind == "PORT" else (
        _DETAIL_ENUMS if kind == "PORT_DETAIL" else {}
    )
    for key, allowed in enums.items():
        if record[key] not in allowed:
            raise ValueError(f"invalid {kind} enum: {key}")
    if kind == "CONSOLE":
        for key, maximum in _BOUNDS.items():
            _unsigned(record[key], maximum)
        for key in ("line_capacity", "input_budget", "output_capacity", "record_capacity",
                    "drain_budget"):
            if int(record[key]) != _BOUNDS[key]:
                raise ValueError(f"unexpected console bound: {key}")
        if int(record["queued"]) + int(record["available"]) != 8192:
            raise ValueError("contradictory console capacity")
    if kind == "HELP":
        row = next((item for item in ROWS if item["command"] == record["canonical"]), None)
        if row is None:
            raise ValueError("unimplemented help row")
        for key in SCHEMAS["HELP"][3:]:
            if key != "canonical" and record[key] != row[key]:
                raise ValueError(f"help metadata mismatch: {key}")


def parse_record(line: str) -> dict[str, str]:
    """Parse a machine line to a string mapping including 'type', or raise ValueError.

    Accept at most511 ASCII bytes with one optional final newline/CRLF. Human
    text, ANSI, duplicate keys, bad tokens and invalid schemas/values reject.
    Additional fields survive. The caller owns byte assembly and correlation.
    """
    if not isinstance(line, str) or not line.isascii():
        raise ValueError("record must be bounded ASCII text")
    if line.endswith("\r\n"):
        line = line[:-2] + "\n"
    if len(line) + (0 if line.endswith("\n") else 1) > 511:
        raise ValueError("oversized record")
    if line.endswith("\n"):
        line = line[:-1]
    parts = line.split(" ")
    if len(parts) < 3 or parts[0] != "TUSB" or parts[1] not in SCHEMAS:
        raise ValueError("not a supported machine record")
    result = {"type": parts[1]}
    for field in parts[2:]:
        if field.count("=") != 1:
            raise ValueError("malformed key=value field")
        key, value = field.split("=", 1)
        if key in result:
            raise ValueError(f"duplicate key: {key}")
        result[key] = value
    validate_record(result)
    return result
