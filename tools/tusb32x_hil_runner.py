"""Bounded serial sessions and saved evidence, without hardware test plans.

I/O is injected as read(size, timeout)/write(bytes, timeout); time is injected in
monotonic seconds. Only this run's admitted token may be controlled or consumed.
The command-line entry performs a read-only marker/identity session, not HIL
qualification. Optional pyserial is imported only when an explicit port is opened.
"""

import argparse
from dataclasses import asdict, dataclass, field
import math
from pathlib import Path
import re
import secrets
import subprocess
import time

from tusb32x_cli_contract import ROWS, SCHEMAS, parse_record, validate_exchange
from tusb32x_hil_artifacts import Artifacts, CaptureLimits, EvidenceError

MAX_REQUEST = (1 << 64) - 1
LOCAL_PROFILE = {
    "mode": "mode", "current": "current", "duty": "duty", "debounce": "debounce",
    "preference": "preference", "ufpaccessory": "ufpaccessory", "termination": "termination",
    "rd-rp": "rd_rp",
}
KINDS = {
    "init": "INITIALIZE", "read": "REFRESH_PORT", "profile.read": "READ_PROFILE",
    "apply": "APPLY_PROFILE", "verify": "VERIFY_PROFILE", "reconcile": "RECONCILE",
    "irq.read": "REFRESH_PORT", "irq.ack": "ACKNOWLEDGE_INTERRUPT",
    "irq.service": "SERVICE_INTERRUPT", "reset": "RESET",
    "recover-dead-battery": "RECOVER_DEAD_BATTERY",
}
ROW_BY_COMMAND = {row["command"]: row for row in ROWS}
ALIASES = {alias: row["command"] for row in ROWS for alias in row["aliases"].split(",")
           if alias != "none" and "." not in alias}
ANSI = re.compile(rb"\x1b\[[0-9;]*m")
BOOT = re.compile(rb"(?:ESP-ROM|ets |rst:|boot:|reboot|TUSB_BOOT)", re.IGNORECASE)


class SessionError(RuntimeError):
    """Base error for a saved session failure; exit_code describes its CLI verdict."""

    exit_code = 1


class InvocationError(SessionError):
    """Invalid invocation or unavailable tool; no complete test proof, exit 2."""

    exit_code = 2


class ProtocolError(SessionError):
    """Malformed, contradictory or lost evidence; exit 1 and invalidate the session."""


class SessionBlocked(SessionError):
    """An unmet identity/ownership prerequisite prevents work; exit 2."""

    exit_code = 2


class SessionTimeout(ProtocolError):
    """The single monotonic exchange deadline expired, including a late success."""


def finite_seconds(value, name):
    """Return finite positive seconds at most 3600, rejecting booleans and NaN."""
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise InvocationError(f"{name} must be finite seconds")
    if not math.isfinite(value) or not 0 < value <= 3600:
        raise InvocationError(f"{name} must be in (0,3600] seconds")
    return float(value)


@dataclass(frozen=True)
class Timeouts:
    """Finite command and per-write ceilings in seconds, validated before opening."""

    command: float = 5.0
    write: float = 1.0

    def __post_init__(self):
        """Reject unsupported timeout values before any backend work."""
        finite_seconds(self.command, "command timeout")
        finite_seconds(self.write, "write timeout")


@dataclass(frozen=True)
class Limits:
    """Per-exchange limits; captured transcript/tail limits belong to CaptureLimits."""

    response_bytes: int = 256 * 1024
    response_records: int = 4096
    noise_bytes: int = 8192
    line_bytes: int = 1024
    io_reads: int = 16384

    def __post_init__(self):
        """Require positive integers with finite ceilings independent of the peer."""
        maxima = (4 * 1024 * 1024, 65536, 65536, 4096, 262144)
        for (name, value), maximum in zip(asdict(self).items(), maxima):
            if type(value) is not int or not 1 <= value <= maximum:
                raise InvocationError(f"{name} must be an integer in [1,{maximum}]")


@dataclass(frozen=True)
class FirmwareIdentity:
    """Firmware-reported provenance; host checkout metadata is a separate fact."""

    library: str
    git: str
    tree: str
    build: str
    framework: str
    framework_version: str
    target: str

    @property
    def known(self):
        """Whether every firmware field is known enough for mutation prerequisites."""
        return (bool(re.fullmatch(r"[0-9a-fA-F]{40}", self.git)) and
                self.tree in {"clean", "dirty"} and
                all(value not in {"", "unknown", "none"} for value in (
                    self.library, self.build, self.framework, self.framework_version, self.target)))

    def require(self, expected):
        """Require each explicitly expected field, rejecting unknown identity or typos."""
        if expected and not self.known:
            raise SessionBlocked("unknown firmware identity cannot satisfy an expectation")
        actual = asdict(self)
        for name, value in expected.items():
            if name not in actual or not isinstance(value, str) or value in {"", "unknown"}:
                raise InvocationError(f"invalid expected identity field: {name}")
            if actual[name] in {"unknown", "none", ""} or actual[name] != value:
                raise SessionBlocked(f"firmware identity mismatch/unknown: {name}")


@dataclass
class Exchange:
    """One bounded command response; admitted work closes later in its original frame."""

    command: str
    nonce: str
    ack: dict = field(default_factory=dict)
    records: list = field(default_factory=list)
    end: dict | None = None

    @property
    def admitted(self):
        """IN_PROGRESS/ADMITTED is accepted work, not successful operation completion."""
        return self.ack.get("action") == "ADMITTED"


@dataclass
class OwnedOperation:
    """One admitted token, its original request/nonce, and fixed-size terminal evidence."""

    token: int
    request: int
    command: str
    nonce: str
    ack: dict
    deadline: float
    records: dict = field(default_factory=dict)
    response_bytes: int = 0
    response_records: int = 0

    @property
    def complete(self):
        """True only after the original RESULT, RESULT_DETAIL and END were verified."""
        return "END" in self.records


def canonical_command(text):
    """Normalize contract aliases without duplicating device-side argument validation."""
    if (not isinstance(text, str) or not text.isascii() or not text or
            len(text) > 191 or any(ord(char) < 32 or ord(char) > 126 for char in text)):
        raise InvocationError("command must be 1..191 printable ASCII bytes")
    words = text.split()
    if not words or len(words) > 12 or any(len(word) > 32 for word in words):
        raise InvocationError("command exceeds token limits")
    first = words[0]
    if first == "profile":
        name = "profile." + (words[1] if len(words) > 1 else "show")
    elif first == "job":
        second = words[1] if len(words) > 1 else "status"
        name = {"cancel": "cancel", "result": "result"}.get(second, "job." + second)
    elif first == "result" and len(words) > 1 and words[1] == "take":
        name = "result.take"
    elif first == "irq" and len(words) > 1:
        name = "irq." + words[1]
    else:
        name = ALIASES.get(first, first)
    return name if name in ROW_BY_COMMAND else "invalid"


class Session:
    """Serialized session over borrowed terminal I/O; no retries, scans or resets.

    State retains only the pending response, one operation and high-water IDs.
    Each call uses one absolute monotonic deadline across all its writes/reads.
    Reconnect notification requires invalidate and a new Session/handshake.
    """

    def __init__(self, io, artifacts, clock=time.monotonic, timeouts=None, limits=None,
                 nonce_factory=None):
        """Borrow I/O/artifacts/clock; construction performs no I/O or implicit opening."""
        self.io = io
        self.artifacts = artifacts
        self.clock = clock
        self.timeouts = timeouts or Timeouts()
        self.limits = limits or Limits()
        self.nonce_factory = nonce_factory or (lambda: secrets.token_hex(16))
        self.valid = False
        self.identity = None
        self.nonce = "none"
        self.last_request = 0
        self.last_token = 0
        self.operation = None
        self._pending = None
        self._buffer = bytearray()
        self._last_clock = None
        self._synchronizing = False
        self._started = False
        self._local = {}
        self._changed = set()
        self._restoring = False
        self._begin_nonce = None
        self._exchange_deadline = None
        self._payload_failed = False
        self._record_bytes = 0

    def _now(self):
        """Sample finite non-regressing monotonic seconds; ambiguity invalidates evidence."""
        value = self.clock()
        if (isinstance(value, bool) or not isinstance(value, (float, int)) or
                not math.isfinite(value) or (self._last_clock is not None and
                                            value < self._last_clock)):
            raise ProtocolError("invalid/regressed monotonic clock")
        self._last_clock = value
        return value

    def _deadline(self, deadline):
        """Validate or establish one finite absolute deadline, never renewed per fragment."""
        now = self._now()
        value = now + self.timeouts.command if deadline is None else deadline
        if not isinstance(value, (int, float)) or not math.isfinite(value):
            raise InvocationError("deadline must be finite monotonic seconds")
        if value <= now:
            raise SessionTimeout("deadline expired")
        if value - now > 3600:
            raise InvocationError("deadline interval exceeds 3600 seconds")
        return value

    def _remaining(self, deadline):
        """Check before and after every I/O; a late success is still a timeout."""
        remaining = deadline - self._now()
        if remaining <= 0:
            raise SessionTimeout("session deadline expired")
        return remaining

    def invalidate(self, reason):
        """Locally mark reboot/reconnect/loss without automatic cancellation or restoration."""
        self.valid = False
        self._synchronizing = False
        self.artifacts.metadata["session_invalid"] = str(reason)[:1024]
        if self._changed:
            self.artifacts.metadata["unrestored_local"] = sorted(self._changed)
            self._uncertain("Local settings were not restored: " + ",".join(sorted(self._changed)))
        if self.operation:
            self.artifacts.metadata["unconsumed_operation"] = {
                "token": self.operation.token, "origin_request": self.operation.request}
            self._uncertain(f"Owned token {self.operation.token} was not consumed")
        if self.artifacts.metadata.get("device_intent"):
            self._uncertain("Session lost after device intent: " +
                            self.artifacts.metadata["device_intent"])

    def _uncertain(self, fact):
        """Retain a bounded residual fact independently of primary and cleanup outcomes."""
        if fact not in self.artifacts.residual_uncertainty:
            self.artifacts.residual_uncertainty.append(fact[:4096])

    def _abort(self, error):
        """Save failure/interruption without allowing cleanup to repair its primary outcome."""
        self.invalidate(str(error) or type(error).__name__)
        self.artifacts.interrupted |= isinstance(error, KeyboardInterrupt)
        status = "BLOCKED" if isinstance(error, SessionBlocked) else "FAIL"
        self.artifacts.primary = status if not self.artifacts.interrupted else "NOT RUN"
        self.artifacts.check("session", status, str(error) or type(error).__name__)
        self.artifacts.checkpoint()

    def _line(self, deadline, counters):
        """Assemble a bounded line while streaming every received fragment to disk."""
        while True:
            self._remaining(deadline)
            newline = self._buffer.find(b"\n")
            if newline >= 0:
                line = bytes(self._buffer[:newline + 1])
                del self._buffer[:newline + 1]
                if len(line) > self.limits.line_bytes:
                    raise ProtocolError("line byte cap reached")
                return line
            if len(self._buffer) >= self.limits.line_bytes:
                raise ProtocolError("truncated/oversized line")
            counters["reads"] += 1
            if counters["reads"] > self.limits.io_reads:
                raise ProtocolError("read call cap reached")
            remaining = self._remaining(deadline)
            size = min(256, self.limits.line_bytes - len(self._buffer))
            data = self.io.read(size, remaining)
            if not isinstance(data, bytes) or len(data) > size:
                raise ProtocolError("invalid transport receive")
            self.artifacts.record("RX", data)
            self._remaining(deadline)
            counters["bytes"] += len(data)
            if counters["bytes"] > self.limits.response_bytes:
                raise ProtocolError("response byte cap reached")
            self._buffer.extend(data)

    def _parse_line(self, line, counters):
        """Decode one bounded line; only exact machine records establish evidence."""
        if line.startswith(b"TUSB "):
            self._record_bytes = len(line)
            try:
                record = parse_record(line.decode("ascii"))
            except (ValueError, UnicodeError) as error:
                raise ProtocolError(str(error)) from error
            counters["records"] += 1
            if counters["records"] > self.limits.response_records:
                raise ProtocolError("response record cap reached")
            if record["type"] in {"LOSS", "SESSION"}:
                raise ProtocolError("output loss or request exhaustion")
            return record
        counters["noise"] += len(line)
        if counters["noise"] > self.limits.noise_bytes:
            raise ProtocolError("boot/ANSI noise cap reached")
        if not self._synchronizing and BOOT.search(line):
            raise ProtocolError("new boot banner invalidates session")
        human = ANSI.sub(b"", line).rstrip(b"\r\n")
        if b"TUSB " in human or any(byte < 32 or byte > 126 for byte in human):
            raise ProtocolError("machine record contaminated by ANSI/control bytes")
        return None

    def _record(self, deadline, counters):
        """Reject damaged machine records; bounded human/ANSI/boot noise is never evidence."""
        while True:
            record = self._parse_line(self._line(deadline, counters), counters)
            if record is not None:
                return record

    def _buffered(self, deadline):
        """Reject already received duplicate/lost framing before another write or final PASS."""
        counters = {"bytes": len(self._buffer), "records": 0, "noise": 0, "reads": 0}
        while b"\n" in self._buffer:
            record = self._parse_line(self._line(deadline, counters), counters)
            if record is not None:
                self._ingest(record)
        if self._buffer and self.operation is None:
            raise ProtocolError("truncated unsolicited output after request closure")

    def _identity_record(self, record):
        """Compare observed identity fields with the verified baseline whenever repeated."""
        if self.identity is None:
            return
        expected = {
            "VERSION": {"library": self.identity.library},
            "FIRMWARE": {key: getattr(self.identity, key) for key in ("git", "tree", "build")},
            "FRAMEWORK": {"name": self.identity.framework,
                          "version": self.identity.framework_version,
                          "target": self.identity.target},
        }.get(record["type"], {})
        if any(record[key] != value for key, value in expected.items()):
            raise ProtocolError("firmware identity changed")

    def _operation_record(self, record):
        """Validate original older-job traffic independently of newer query ACK IDs."""
        op = self.operation
        self._remaining(op.deadline)
        op.response_bytes += self._record_bytes
        op.response_records += 1
        if (op.response_bytes > self.limits.response_bytes or
                op.response_records > self.limits.response_records):
            raise ProtocolError("original operation response cap reached")
        kind = record["type"]
        if (record["nonce"] != op.nonce or record.get("token") != str(op.token) or
                kind not in {"JOB", "RESULT", "RESULT_DETAIL", "END"}):
            raise ProtocolError("original operation correlation changed")
        if op.complete:
            raise ProtocolError("duplicate original terminal completion")
        if kind in {"RESULT", "RESULT_DETAIL"}:
            if record["origin_request"] != str(op.request) or kind in op.records:
                raise ProtocolError("duplicate/wrong-origin operation result")
        if kind in {"JOB", "RESULT"} and record["kind"] != KINDS[op.command]:
            raise ProtocolError("operation kind changed")
        if kind == "JOB" and "RESULT" in op.records:
            raise ProtocolError("progress followed terminal result")
        if kind == "RESULT_DETAIL" and "RESULT" not in op.records:
            raise ProtocolError("result detail preceded payload")
        op.records[kind] = record
        if kind == "END":
            try:
                validate_exchange([op.ack, *op.records.values()], op.request, op.command, op.nonce)
            except ValueError as error:
                raise ProtocolError(str(error)) from error
            self.artifacts.metadata["operation"] = {
                "ack": op.ack, "result": op.records["RESULT"],
                "detail": op.records["RESULT_DETAIL"], "end": record, "taken": False}
            result = op.records["RESULT"]
            if result["code"] != "OK":
                self._payload_failed = True
                self.artifacts.check("operation_payloads", "FAIL",
                                     f"token={op.token} {result['code']} detail={result['detail']}")
            elif not self._payload_failed:
                self.artifacts.check("operation_payloads", "PASS", "owned payloads succeeded")
            self.artifacts.checkpoint()

    def _ingest(self, record):
        """Route one record to the pending input or the single owned original operation."""
        pending = self._pending
        kind = record["type"]
        if self._synchronizing and not (kind == "MARK" and record["nonce"] == pending.nonce):
            return  # Valid stale pre-marker output cannot establish this run's frame.
        if kind in {"ACK", "MARK"}:
            if pending is None or pending.ack or int(record["request"]) <= self.last_request:
                raise ProtocolError("duplicate/regressed/unexpected input acknowledgement")
            if record["nonce"] != pending.nonce or (
                    kind == "MARK" and pending.command != "hilmark") or (
                    kind == "ACK" and record["command"] != pending.command):
                raise ProtocolError("input acknowledgement changed command/nonce")
            if (pending.command == "hilmark") != (kind == "MARK"):
                raise ProtocolError("marker requires exact MARK acknowledgement")
            if (kind == "ACK" and self.operation and record["code"] == "OK" and
                    pending.command in {"cancel", "result", "result.take", "job.status",
                                        "job.auto", "job.step"} and
                    record["token"] != str(self.operation.token)):
                raise ProtocolError("acknowledgement changed owned token")
            self.last_request = int(record["request"])
            self._synchronizing = False
            pending.ack = record
            pending.records.append(record)
            if pending.admitted:
                token = int(record["token"])
                if self.operation or token <= self.last_token or pending.command not in KINDS:
                    raise ProtocolError("foreign/reused operation token")
                self.last_token = token
                self.operation = OwnedOperation(
                    token, self.last_request, pending.command, pending.nonce, record,
                    self._exchange_deadline, response_bytes=self._record_bytes,
                    response_records=1)
            return
        request = int(record["request"])
        if self.operation and request == self.operation.request:
            self._operation_record(record)
            return
        if (pending is None or not pending.ack or request != int(pending.ack["request"]) or
                record["nonce"] != pending.nonce):
            raise ProtocolError("unexpected/closed request or changed nonce")
        self._identity_record(record)
        if kind in {"JOB", "RESULT", "RESULT_DETAIL"}:
            token = int(record["token"])
            foreign = kind == "JOB" and (record["state"] == "ACTIVE" or
                                         record["result_pending"] == "1")
            if (token and (self.operation is None or token != self.operation.token)) or (
                    foreign and self.operation is None):
                raise SessionBlocked("foreign active or retained operation")
            if kind in {"RESULT", "RESULT_DETAIL"}:
                if (self.operation is None or
                        record["origin_request"] != str(self.operation.request)):
                    raise ProtocolError("result copy changed origin/token")
        pending.records.append(record)
        if kind == "END":
            self._validate_pending(pending)
            pending.end = record

    def _validate_pending(self, pending):
        """Check exact closure, required records and immutable copies of terminal payloads."""
        try:
            validate_exchange(pending.records, int(pending.ack["request"]),
                              pending.command, pending.nonce)
        except ValueError as error:
            raise ProtocolError(str(error)) from error
        end = pending.records[-1]
        if end["code"] == "OK" and pending.command in ROW_BY_COMMAND:
            required = set(ROW_BY_COMMAND[pending.command]["records"].split(","))
            if not required <= {record["type"] for record in pending.records}:
                raise ProtocolError("missing required response records")
            if "PROFILE" in required and {record["source"] for record in pending.records
                                          if record["type"] == "PROFILE"} != {
                    "staged", "desired", "observed", "applied"}:
                raise ProtocolError("profile response requires all four explicit sources")
        unique = {}
        for record in pending.records:
            kind = record["type"]
            discriminator = (record["source"] if kind == "PROFILE" else
                             record["canonical"] if kind == "HELP" else "")
            key = (kind, discriminator)
            if key in unique:
                raise ProtocolError("duplicate response record")
            unique[key] = record
            if kind in {"RESULT", "RESULT_DETAIL"}:
                if not self.operation or not self.operation.complete:
                    raise ProtocolError("copied result lacks original terminal closure")
                original = self.operation.records[kind]
                for field_name in SCHEMAS[kind][3:]:
                    if record[field_name] != original[field_name]:
                        raise ProtocolError("result query contradicts original payload")

    def _exchange(self, text, name, deadline):
        """Write once, then collect one ACK/MARK plus closure or accepted admission."""
        if self._pending is not None or self.last_request == MAX_REQUEST:
            error = ProtocolError("pending request or exhausted request counter")
            self._abort(error)
            raise error
        pending = Exchange(name, text.split()[1] if name == "hilmark" else self.nonce)
        if self.operation and not self.operation.complete:
            deadline = min(deadline, self.operation.deadline)
        self._exchange_deadline = deadline
        counters = {"bytes": len(self._buffer), "records": 0, "noise": 0, "reads": 0}
        try:
            self._buffered(deadline)
            self._pending = pending
            data = (text + "\n").encode("ascii")
            self._remaining(deadline)
            self.artifacts.record("TX", data)
            write_deadline = min(deadline, self._now() + self.timeouts.write)
            remaining = self._remaining(write_deadline)
            written = self.io.write(data, remaining)
            self._remaining(write_deadline)
            if type(written) is not int or written != len(data):
                raise ProtocolError("short serial write; command acceptance is uncertain")
            while pending.end is None and not pending.admitted:
                self._ingest(self._record(deadline, counters))
            self._remaining(deadline)
            return pending
        except BaseException as error:
            self._abort(error)
            raise
        finally:
            self._pending = None

    def mark(self, deadline=None):
        """Require a fresh random MARK and matching END under one supplied deadline."""
        if not self.valid and not self._synchronizing:
            raise SessionBlocked("marker requires an establishing or verified session")
        deadline = self._deadline(deadline)
        nonce = self.nonce_factory()
        if (not isinstance(nonce, str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,32}", nonce) or
                nonce in {"none", self.nonce, self._begin_nonce}):
            raise InvocationError("marker nonce must be fresh, bounded ASCII")
        response = self._exchange("hilmark " + nonce, "hilmark", deadline)
        if response.end["code"] != "OK":
            raise ProtocolError("marker rejected")
        self.nonce = nonce
        return nonce

    def begin(self, expected=None, deadline=None):
        """Fresh marker, identity and idle prerequisite plus local snapshots; no device writes."""
        if self._started:
            raise SessionBlocked("create a fresh Session after loss/reconnect")
        self._started = True
        deadline = self._deadline(deadline)
        try:
            return self._begin(expected, deadline)
        except BaseException as error:
            self._abort(error)
            raise

    def _begin(self, expected, deadline):
        """Establish provisional framing; any failed prerequisite invalidates the handshake."""
        self._synchronizing = True
        self.mark(deadline)
        self._begin_nonce = self.nonce
        self.valid = True
        response = self.command("version", deadline)
        values = {record["type"]: record for record in response.records}
        self.identity = FirmwareIdentity(
            values["VERSION"]["library"], values["FIRMWARE"]["git"],
            values["FIRMWARE"]["tree"], values["FIRMWARE"]["build"],
            values["FRAMEWORK"]["name"], values["FRAMEWORK"]["version"],
            values["FRAMEWORK"]["target"])
        self.artifacts.metadata["firmware"] = asdict(self.identity)
        self.identity.require(expected or {})
        bus = self.command("bus", deadline)
        self.artifacts.metadata["target"] = next(r for r in bus.records if r["type"] == "BUS")
        jobs = self.command("job status", deadline)
        self._local["auto"] = next(r["auto"] for r in jobs.records if r["type"] == "JOB")
        console = self.command("console", deadline)
        self._local["console"] = next(r for r in console.records if r["type"] == "CONSOLE")
        profiles = self.command("profile show", deadline)
        self._local["profile"] = next(r for r in profiles.records if
                                      r["type"] == "PROFILE" and r["source"] == "staged")
        self.artifacts.metadata["local_before"] = self._local
        self.artifacts.check("session", "PASS", "fresh marker/identity and idle prerequisite")
        self.artifacts.checkpoint()
        return self.identity

    def _guard(self, text, name):
        """Fence foreign work and checkpoint local intent separately from device effects."""
        if not self.valid:
            raise SessionBlocked("session invalid; re-establish evidence with a fresh Session")
        words = text.split()
        if name in {"cancel", "result", "result.take", "job.step"}:
            if self.operation is None:
                raise SessionBlocked("no token admitted by this run")
            token_index = 2 if name == "result.take" or words[0] == "job" else 1
            if name in {"cancel", "result", "result.take"} and len(words) > token_index:
                if words[token_index] != str(self.operation.token):
                    raise SessionBlocked("cannot control/take a foreign token")
        if name in KINDS and self.operation:
            raise SessionBlocked("owned result must be consumed before replacement work")
        if name in {"variant", "addr"} and len(words) > 1:
            raise SessionBlocked("target edits require a separately established session")
        mutation = (ROW_BY_COMMAND.get(name, {}).get("effect") == "DEVICE_WRITE" and
                    name != "job.step")
        if mutation and (self.identity is None or not self.identity.known):
            raise SessionBlocked("known firmware identity required for device mutation")
        changed = set()
        if not self._restoring:
            if name in LOCAL_PROFILE and len(words) > 1:
                changed.add("profile")
            if name in {"profile.reset", "profile.discard"}:
                changed.add("profile")
            if name in {"color", "verbose"} and len(words) > 1:
                changed.add(name)
            if name == "job.auto" and len(words) > 2:
                changed.add("auto")
            self._changed.update(changed)
        if changed or mutation:
            self.artifacts.metadata["local_changed"] = sorted(self._changed)
            if mutation:
                self.artifacts.metadata["device_intent"] = text
            self.artifacts.checkpoint()  # Intent must survive interruption inside write().

    def command(self, text, deadline=None):
        """Execute one explicit contract route; no implicit apply/cancel/result consumption."""
        name = canonical_command(text)
        if name == "hilmark":
            raise InvocationError("use mark() for fresh session markers")
        deadline = self._deadline(deadline)
        # These local commands can disturb someone else's active work. Refresh the
        # cached prerequisite using the SAME deadline, without consuming a result.
        if self.valid and self.operation is None and (
                name in {"end", "bind", "forget-evidence", "invalidate", "xfer_reset"} or
                (name == "job.auto" and len(text.split()) > 2)):
            self.command("job status", deadline)
        self._guard(text, name)
        response = self._exchange(text, name, deadline)
        if name == "result.take" and response.end and response.end["code"] == "OK":
            self.artifacts.metadata["operation"]["taken"] = True
            self.operation = None
            self.artifacts.checkpoint()
        return response

    def wait_operation(self, deadline=None):
        """Wait for the one original terminal pair without consuming its retained result."""
        if not self.valid or self.operation is None:
            raise SessionBlocked("no verified owned operation")
        deadline = self._deadline(deadline)
        if not self.operation.complete:
            deadline = min(deadline, self.operation.deadline)
        counters = {"bytes": len(self._buffer), "records": 0, "noise": 0, "reads": 0}
        try:
            while not self.operation.complete:
                self._ingest(self._record(deadline, counters))
            self._remaining(deadline)
            return dict(self.operation.records["RESULT"])
        except BaseException as error:
            self._abort(error)
            raise

    def take_result(self, deadline=None):
        """Deliver the exact owned token; delivery OK does not repair a failed payload."""
        if self.operation is None:
            raise SessionBlocked("no retained result owned by this run")
        return self.command(f"result take {self.operation.token}", deadline)

    def restore_local(self, deadline=None):
        """Restore only saved staging/auto/output values in the still verified session; no apply."""
        if not self._changed:
            return
        if not self.valid or self.operation:
            self.artifacts.metadata["unrestored_local"] = sorted(self._changed)
            raise SessionBlocked("local restoration requires verified idle session")
        deadline = self._deadline(deadline)
        self.command("version", deadline)  # Identity change invalidates instead of replaying.
        self._restoring = True
        try:
            commands = []
            if "profile" in self._changed:
                for command, key in LOCAL_PROFILE.items():
                    value = self._local["profile"][key]
                    if value != "unknown":
                        commands.append(f"{command} {value}")
            if "auto" in self._changed:
                commands.append("job auto " + self._local["auto"])
            for key in ("verbose", "color"):
                if key in self._changed:
                    commands.append(key + " " + self._local["console"][key])
            for text in commands:
                response = self.command(text, deadline)
                if response.end["code"] != "OK":
                    raise SessionBlocked("local restoration rejected: " + text)
                words = text.split()
                if words[0] in LOCAL_PROFILE:
                    payload = next(r for r in response.records if
                                   r["type"] == "PROFILE" and r["source"] == "staged")
                    actual = payload[LOCAL_PROFILE[words[0]]]
                elif words[0] == "job":
                    actual = next(r["auto"] for r in response.records if r["type"] == "JOB")
                else:
                    actual = next(r[words[0]] for r in response.records if r["type"] == "CONSOLE")
                if actual != words[-1]:
                    raise ProtocolError("local restoration payload mismatch: " + text)
            self._changed.clear()
            self.artifacts.metadata["local_changed"] = []
            self.artifacts.cleanup = "PASS"
            self.artifacts.checkpoint()
        except BaseException:
            self.artifacts.cleanup = "FAIL"
            self.artifacts.metadata["unrestored_local"] = sorted(self._changed)
            self.artifacts.checkpoint()
            raise
        finally:
            self._restoring = False

    def finish(self, deadline=None):
        """Restore local values and require a fresh end marker; leave borrowed I/O to its owner."""
        if not self.valid or self.operation:
            raise SessionBlocked("cannot close invalid session or abandon an owned result")
        deadline = self._deadline(deadline)
        self.restore_local(deadline)
        self.mark(deadline)
        try:
            self._buffered(deadline)
            self._remaining(deadline)
        except BaseException as error:
            self._abort(error)
            raise
        self.artifacts.check("end_marker", "PASS", "fresh end marker and closure")
        self.artifacts.checkpoint()
        self.valid = False


class SerialTransport:
    """Terminal pyserial bridge with finite read/write timeouts and reported pre-open settings."""

    def __init__(self, serial, settings):
        """Borrow an already opened serial object and retain requested/applied line settings."""
        self.serial = serial
        self.settings = settings

    def read(self, size, timeout):
        """Read at most size bytes under the session's remaining finite seconds."""
        self.serial.timeout = min(timeout, 0.05)
        return self.serial.read(size)

    def write(self, data, timeout):
        """Write once under the remaining budget; Session rejects short writes without retry."""
        self.serial.write_timeout = timeout
        return self.serial.write(data)

    def close(self):
        """Close the owned serial handle without reset, retry or a blocking flush."""
        self.serial.close()


def open_serial(port, *, timeouts=None, dtr="unchanged", rts="unchanged", baud=115200,
                serial_factory=None):
    """Validate before constructing/opening a port; configure DTR/RTS before open.

    An OS/driver can pulse lines or ignore their setting, and open itself has no
    portable deadline. An explicit physical port is mandatory; URLs/scans are
    unsupported. Opening may reset a board even with unchanged line requests.
    """
    timeouts = timeouts or Timeouts()
    if not isinstance(port, str) or not port.strip() or "://" in port:
        raise InvocationError("an explicit physical serial port is required")
    if dtr not in {"on", "off", "unchanged"} or rts not in {"on", "off", "unchanged"}:
        raise InvocationError("DTR/RTS must be on/off/unchanged")
    if type(baud) is not int or not 1 <= baud <= 4_000_000:
        raise InvocationError("baud must be in [1,4000000]")
    if serial_factory is None:
        try:
            import serial
        except ImportError as error:
            raise InvocationError(
                "install tools/requirements-hil.txt for hardware serial") from error
        serial_factory = serial.Serial
    serial = serial_factory()
    settings = {"port": port, "baud": baud, "dtr": dtr, "rts": rts,
                "open_may_reset": True, "open_deadline_supported": False}
    try:
        if serial.is_open:
            raise InvocationError("serial factory must return a closed handle")
        serial.port = port
        serial.baudrate = baud
        serial.timeout = timeouts.command
        serial.write_timeout = timeouts.write
        serial.rtscts = False
        serial.dsrdtr = False
        for name, requested in (("dtr", dtr), ("rts", rts)):
            if requested != "unchanged":
                setattr(serial, name, requested == "on")
            settings[name + "_preopen"] = getattr(serial, name, "unsupported")
        serial.open()
        return SerialTransport(serial, settings)
    except BaseException:
        serial.close()
        raise


def host_identity(root):
    """Read only this checkout's Git identity, with finite argument-list subprocess calls."""
    root = Path(root).resolve()
    try:
        top = subprocess.run(["git", "-C", str(root), "rev-parse", "--show-toplevel"],
                             capture_output=True, text=True, check=True, timeout=3)
        if Path(top.stdout.strip()).resolve() != root:
            return {"git": "unknown", "tree": "unknown"}
        commit = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                                capture_output=True, text=True, check=True, timeout=3)
        dirty = subprocess.run(["git", "-C", str(root), "status", "--porcelain"],
                               capture_output=True, text=True, check=True, timeout=3)
        return {"git": commit.stdout.strip(), "tree": "dirty" if dirty.stdout else "clean"}
    except (OSError, subprocess.SubprocessError):
        return {"git": "unknown", "tree": "unknown"}


def main(argv=None):
    """Run only a read-only serial session and save outcomes; return 0/1/2/130.

    Opening is explicit and may reset the board. Observation/fixture suites are
    not implemented here. Invocation failures before capture creation return 2;
    later exceptions/interruption checkpoint evidence before closing the port.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--timeout", type=float, default=5.0)
    parser.add_argument("--write-timeout", type=float, default=1.0)
    parser.add_argument("--dtr", choices=("on", "off", "unchanged"), default="unchanged")
    parser.add_argument("--rts", choices=("on", "off", "unchanged"), default="unchanged")
    parser.add_argument("--response-bytes", type=int, default=256 * 1024)
    parser.add_argument("--transcript-bytes", type=int, default=32 * 1024 * 1024)
    parser.add_argument("--tail-bytes", type=int, default=64 * 1024)
    for name in ("library", "git", "tree", "build", "framework", "framework-version", "target"):
        parser.add_argument("--expect-" + name)
    artifacts = None
    io = None
    session = None
    try:
        args = parser.parse_args(argv)
        timeouts = Timeouts(args.timeout, args.write_timeout)
        limits = Limits(response_bytes=args.response_bytes)
        capture_limits = CaptureLimits(args.transcript_bytes, args.tail_bytes)
        artifacts = Artifacts(args.output, time.monotonic, capture_limits)
        artifacts.metadata["host"] = host_identity(Path(__file__).resolve().parents[1])
        artifacts.metadata["serial_requested"] = {
            "port": args.port, "baud": args.baud, "dtr": args.dtr, "rts": args.rts,
            "open_may_reset": True, "open_deadline_supported": False}
        artifacts.check("session", "NOT RUN", "read-only framing/identity exchange")
        artifacts.check("end_marker", "NOT RUN", "end marker not yet observed")
        artifacts.checkpoint()
        io = open_serial(args.port, timeouts=timeouts, dtr=args.dtr, rts=args.rts, baud=args.baud)
        artifacts.metadata["serial"] = io.settings
        session = Session(io, artifacts, timeouts=timeouts, limits=limits)
        expected = {name: getattr(args, "expect_" + name) for name in (
            "library", "git", "tree", "build", "framework", "framework_version", "target")
                    if getattr(args, "expect_" + name) is not None}
        session.begin(expected)
        session.finish()
        artifacts.primary = "PASS"
    except KeyboardInterrupt:
        if artifacts:
            artifacts.interrupted = True
            artifacts.primary = "NOT RUN"
        else:
            return 130
    except SystemExit as error:
        if artifacts is None:
            return int(error.code)
        artifacts.primary = "FAIL"
        artifacts.check("session", "FAIL", "backend exited before complete proof")
    except Exception as error:
        if artifacts is None:
            print(str(error))
            return 2
        failed = artifacts.primary == "FAIL" or getattr(error, "exit_code", 2) == 1
        artifacts.primary = "FAIL" if failed else "BLOCKED"
        artifacts.check("session", "FAIL" if failed else "BLOCKED", str(error))
    finally:
        if artifacts:
            if session and session._changed:
                artifacts.metadata["unrestored_local"] = sorted(session._changed)
            for name, closer in (("serial", io.close if io else None),
                                 ("transcript", artifacts.close)):
                if closer is not None:
                    try:
                        closer()
                    except BaseException as error:
                        if isinstance(error, KeyboardInterrupt):
                            artifacts.interrupted = True
                        artifacts.cleanup = "FAIL"
                        artifacts.residual_uncertainty.append(
                            f"{name} close failed: {error}"[:4096])
            try:
                artifacts.checkpoint()
            except (EvidenceError, OSError, ValueError) as error:
                print(f"Could not save final evidence: {error}")
                return 130 if artifacts.interrupted else 2
    return artifacts.exit_code()


if __name__ == "__main__":
    raise SystemExit(main())
