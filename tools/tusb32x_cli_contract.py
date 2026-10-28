"""Frozen local CLI rows and schema-1 parsing; no I/O or device side effects.

C++ owns command dispatch. Transcript owners separately check request ordering,
nonce attribution and exactly one matching terminal closure. This module covers
the implemented prompt-20 subset only.
"""

from collections.abc import Mapping
import re

PREFIX = ("schema", "nonce", "request")
SCHEMAS = {
    "ACK": PREFIX + (
        "command", "code", "detail", "action", "staged_generation",
        "desired_generation", "applied_generation", "configuration",
    ),
    "END": PREFIX + ("command", "outcome", "code", "detail", "callbacks"),
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
         prerequisites="none", capability="all", records="ACK,END"):
    """Build fixed host metadata; no firmware code generation."""
    return {
        "command": command, "aliases": aliases, "topic": topic, "syntax": syntax,
        "execution": execution, "effect": effect, "prerequisites": prerequisites,
        "capability": capability, "records": records, "max_callbacks": "0",
        "tx_bytes": "0", "rx_bytes": "0", "fixture": "host", "test": "local_routes",
    }


PROFILE_RECORDS = "ACK,CONFIG,PROFILE,END"
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
_UINT = re.compile(r"(?:0|[1-9][0-9]*)\Z", re.ASCII)
_SINT = re.compile(r"-?(?:0|[1-9][0-9]*)\Z", re.ASCII)
_KEY = re.compile(r"[a-z][a-z0-9_]*\Z", re.ASCII)
_VALUE = re.compile(r"[!-<>-~]+\Z", re.ASCII)  # Visible ASCII except '='.
_NONCE = re.compile(r"[A-Za-z0-9_-]{1,32}\Z", re.ASCII)
_BOOL = frozenset((
    "bound", "adapter_available", "loss_pending", "color", "verbose", "identity_valid",
    "identity_matches", "revision_valid", "revision_matches", "advertisement_write",
    "duty_write", "accessory_write", "staged_reset", "has_desired", "has_observed",
    "has_applied", "available", "invalidated", "observed_time_valid",
))
_U32 = frozenset((
    "staged_generation", "desired_generation", "applied_generation", "generation",
    "config_generation", "dropped_records", "dropped_bytes", "bus_hz", "timeout_ms",
))
_U16 = frozenset(("profile_fields", "observed_valid_fields", "valid_fields"))
_U8 = frozenset(("address", "raw_valid_mask", "max_callbacks", "tx_bytes", "rx_bytes"))
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
        elif key in {"sequence", "observed_ms"}:
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
    if kind == "ACK" and record["action"] not in {"QUERY", "LOCAL", "UNCHANGED", "STAGED"}:
        raise ValueError("invalid local action")
    if kind == "ACK" and record["code"] != "OK" and record["action"] != "UNCHANGED":
        raise ValueError("failed local command must leave state unchanged")
    if kind == "END":
        if record["outcome"] not in {"SUCCEEDED", "FAILED"} or record["callbacks"] != "0":
            raise ValueError("invalid local terminal outcome/callback count")
        if (record["outcome"] == "SUCCEEDED") != (record["code"] == "OK"):
            raise ValueError("contradictory terminal status")
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
