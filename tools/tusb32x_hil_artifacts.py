"""Stream bounded serial evidence and save explicit, independent run outcomes.

The caller supplies host and firmware metadata separately; this module never
infers device identity or hardware success. Reports use PASS/FAIL/NOT RUN/BLOCKED.
Each checkpoint replaces report.json and summary.md atomically, individually.
report.json is authoritative if replacing the later summary fails. No serial
port, subprocess or hardware operation is performed here.
"""

from __future__ import annotations

import html
import json
import math
import os
from pathlib import Path
import tempfile
from collections.abc import Callable
from dataclasses import dataclass


STATUSES = frozenset({"PASS", "FAIL", "NOT RUN", "BLOCKED"})
MAX_REPORT_BYTES = 512 * 1024
MAX_CHECKS = 1024
MAX_RESIDUALS = 64
MAX_REPORT_TAIL_BYTES = 64 * 1024


class EvidenceError(RuntimeError):
    """Evidence is invalid, incomplete, or could not be saved; never implies success."""


@dataclass(frozen=True)
class CaptureLimits:
    """Maximum encoded transcript and retained tail bytes, validated before opening.

    Transcript sizes are 1..128 MiB and tail sizes are 1..1 MiB, with tail no
    larger than transcript. Booleans, floats and nonfinite values are rejected.
    Limits count the encoded timestamp/direction/escaped-data representation.
    """

    transcript_bytes: int = 32 * 1024 * 1024
    tail_bytes: int = 64 * 1024

    def __post_init__(self) -> None:
        """Reject invalid sizes rather than silently coercing or clipping them."""
        for name, value, ceiling in (
            ("transcript_bytes", self.transcript_bytes, 128 * 1024 * 1024),
            ("tail_bytes", self.tail_bytes, 1024 * 1024),
        ):
            if type(value) is not int or not 1 <= value <= ceiling:
                raise EvidenceError(f"{name} must be an integer in 1..{ceiling}")
        if self.tail_bytes > self.transcript_bytes:
            raise EvidenceError("tail_bytes must not exceed transcript_bytes")


def _text(value: object, name: str, maximum: int, allow_empty: bool = True) -> str:
    """Validate bounded UTF-8 text used in a report, rejecting surrogate code points."""
    if not isinstance(value, str) or len(value) > maximum or (not allow_empty and not value):
        raise EvidenceError(f"{name} must be text of at most {maximum} characters")
    try:
        value.encode("utf-8")
    except UnicodeError as error:
        raise EvidenceError(f"{name} must be valid UTF-8 text") from error
    return value


def _status(value: str) -> None:
    """Require one exact report outcome; unknown strings must not imply PASS."""
    if not isinstance(value, str) or value not in STATUSES:
        raise EvidenceError("status must be PASS, FAIL, NOT RUN or BLOCKED")


def _copy_metadata(value: object, depth: int = 0, budget: list[int] | None = None) -> object:
    """Copy only bounded JSON values, without invoking arbitrary object conversion."""
    if budget is None:
        budget = [4096]
    budget[0] -= 1
    if budget[0] < 0 or depth > 8:
        raise EvidenceError("metadata exceeds its node or nesting limit")
    if value is None or type(value) is bool:
        return value
    if isinstance(value, str):
        return _text(value, "metadata value", 8192)
    if type(value) is int:
        if value.bit_length() > 128:
            raise EvidenceError("metadata integer exceeds 128 bits")
        return value
    if type(value) is float and math.isfinite(value):
        return value
    if type(value) is list:
        if len(value) > 4096:
            raise EvidenceError("metadata list exceeds its node limit")
        return [_copy_metadata(item, depth + 1, budget) for item in value]
    if type(value) is dict:
        if len(value) > 4096:
            raise EvidenceError("metadata mapping exceeds its node limit")
        return {_text(key, "metadata key", 128, False):
                _copy_metadata(item, depth + 1, budget) for key, item in value.items()}
    raise EvidenceError("metadata must contain finite JSON values only")


def _escaped(data: bytes) -> bytes:
    """Encode a small byte chunk as unambiguous printable ASCII without line breaks."""
    return "".join(
        chr(byte) if 32 <= byte <= 126 and byte != 92 else f"\\x{byte:02x}"
        for byte in data
    ).encode("ascii")


def _markdown(value: str) -> str:
    """Escape report text for a plain Markdown table cell or paragraph."""
    value = html.escape(value, quote=False)
    value = value.translate({ord(character): f"&#{ord(character)};"
                             for character in "\\|`[]()*_#!{}"})
    return value.replace("\r", "&#13;").replace("\n", "<br>")


class Artifacts:
    """Own one exclusive transcript and bounded report state for a serialized run.

    ``metadata`` is a small JSON dictionary; use separate host and firmware keys.
    ``primary`` and ``cleanup`` default to NOT RUN. ``residual_uncertainty`` holds
    at most 64 bounded strings and ``interrupted`` selects exit 130. A check is
    required proof: register NOT RUN before attempting it. No checks means missing
    proof. Direct field changes are validated before reports are replaced.

    Capture loss is sticky. After a cap, clock or transcript I/O failure, stop
    recording and checkpoint the failure; the last valid report remains intact
    if validation or temporary-file writing fails. Checkpoints remain possible
    after capture loss and after close. This class is not thread-safe.
    """

    def __init__(self, directory: Path, clock: Callable[[], float],
                 limits: CaptureLimits | None = None) -> None:
        """Create directory and exclusive transcript.log; never overwrite evidence.

        The injected clock returns finite, nonnegative monotonic seconds. Invalid
        limits/clock fail before opening files. Existing transcript.log is an error.
        No report is created until checkpoint is explicitly requested.
        """
        self.directory = Path(directory)
        self.limits = limits if limits is not None else CaptureLimits()
        if not isinstance(self.limits, CaptureLimits) or not callable(clock):
            raise EvidenceError("valid capture limits and clock are required")
        self._clock = clock
        self._last_time = 0.0
        self._capture_lost = False
        self._capture_error = ""
        self._timestamp()
        self.metadata: dict = {}
        self.primary = "NOT RUN"
        self.cleanup = "NOT RUN"
        self.residual_uncertainty: list[str] = []
        self.interrupted = False
        self._checks: dict[str, dict[str, str]] = {}
        self._tail = bytearray()
        self._transcript_bytes = 0
        self._tx_bytes = 0
        self._rx_bytes = 0
        self._records = 0
        self._closed = False
        try:
            self.directory.mkdir(parents=True, exist_ok=True)
            self._stream = (self.directory / "transcript.log").open("xb", buffering=0)
        except OSError as error:
            raise EvidenceError(f"cannot create exclusive transcript: {error}") from error

    def _lost(self, message: str) -> EvidenceError:
        """Latch the first capture failure so later cleanup cannot restore evidence."""
        if not self._capture_lost:
            self._capture_error = message[:512].encode("utf-8", "replace").decode("utf-8")
        self._capture_lost = True
        return EvidenceError(self._capture_error)

    def _timestamp(self) -> float:
        """Read and validate monotonic seconds; checkpoint never calls this clock."""
        try:
            value = self._clock()
            if type(value) not in (int, float):
                raise ValueError("clock must return a number")
            value = float(value)
            if not math.isfinite(value) or value < self._last_time:
                raise ValueError("clock must be finite, nonnegative and monotonic")
        except Exception as error:
            raise self._lost(f"capture clock failed: {error}") from error
        self._last_time = value
        return value

    def _write(self, data: bytes) -> None:
        """Write one bounded chunk, retaining only actually accepted bytes."""
        written = self._stream.write(data)
        if type(written) is not int or not 0 <= written <= len(data):
            raise OSError("transcript returned an invalid write count")
        self._transcript_bytes += written
        self._tail.extend(data[:written])
        if len(self._tail) > self.limits.tail_bytes:
            del self._tail[:-self.limits.tail_bytes]
        if written != len(data):
            raise OSError("short transcript write")

    def record(self, direction: str, data: bytes) -> None:
        """Stream one TX/RX fragment with absolute monotonic seconds and escaped bytes.

        Cap checks include framing overhead and reject the entire new fragment.
        Chunked encoding keeps extra memory bounded independently of transcript
        size. Cap, clock and I/O failures latch evidence loss and raise EvidenceError.
        Direction and payload validation errors raise without changing capture.
        """
        if direction not in ("TX", "RX") or type(data) is not bytes:
            raise EvidenceError("record requires TX or RX and a bytes payload")
        if self._capture_lost:
            raise EvidenceError(self._capture_error)
        if self._closed:
            raise self._lost("record attempted after transcript close")
        timestamp = self._timestamp()
        header = f"{timestamp:.9f} {direction} bytes={len(data)} ".encode("ascii")
        remaining = self.limits.transcript_bytes - self._transcript_bytes
        if len(data) + len(header) + 1 > remaining:
            raise self._lost("transcript byte cap exceeded; capture is incomplete")
        encoded_size = sum(1 if 32 <= byte <= 126 and byte != 92 else 4 for byte in data)
        if encoded_size + len(header) + 1 > remaining:
            raise self._lost("transcript byte cap exceeded; capture is incomplete")
        if direction == "TX":
            self._tx_bytes += len(data)
        else:
            self._rx_bytes += len(data)
        try:
            self._write(header)
            for offset in range(0, len(data), 1024):
                self._write(_escaped(data[offset:offset + 1024]))
            self._write(b"\n")
            self._stream.flush()
        except (OSError, ValueError) as error:
            raise self._lost(f"transcript I/O failed: {error}") from error
        self._records += 1

    def check(self, name: str, status: str, detail: str = "") -> None:
        """Insert/update required proof with an exact status and bounded explanation.

        Up to 1024 names of 1..128 characters and details of at most 4096 characters
        are accepted. Invalid updates preserve the previous proof. A FAIL is sticky
        for that named check, so a later passing cleanup cannot erase a failure.
        """
        _text(name, "check name", 128, False)
        _status(status)
        _text(detail, "check detail", 4096)
        if name not in self._checks and len(self._checks) >= MAX_CHECKS:
            raise EvidenceError("check count limit exceeded")
        if self._checks.get(name, {}).get("status") == "FAIL" and status != "FAIL":
            raise EvidenceError("a failed check cannot be replaced with successful evidence")
        self._checks[name] = {"status": status, "detail": detail}

    def _outcome(self) -> int:
        """Apply verdict precedence after public report fields have been validated."""
        if self.interrupted:
            return 130
        statuses = [row["status"] for row in self._checks.values()]
        if self.primary == "FAIL" or "FAIL" in statuses:
            return 1
        if (self.primary != "PASS" or self.cleanup in ("FAIL", "BLOCKED")
                or not statuses or any(status != "PASS" for status in statuses)
                or self._capture_lost or self.residual_uncertainty):
            return 2
        return 0

    def snapshot(self) -> dict:
        """Return a detached, bounded JSON report; invalid state raises EvidenceError.

        No clock, transcript or report file is touched. JSON is limited to 512 KiB,
        including the latest at most 64 KiB of retained tail (omission is counted).
        Metadata supports depth 8, 4096 nodes, 8192-character strings and finite
        JSON numbers. Residual strings are at most 4096 chars.
        """
        _status(self.primary)
        _status(self.cleanup)
        if type(self.interrupted) is not bool or type(self.metadata) is not dict:
            raise EvidenceError("interrupted must be bool and metadata must be a dictionary")
        if (type(self.residual_uncertainty) is not list
                or len(self.residual_uncertainty) > MAX_RESIDUALS):
            raise EvidenceError("residual uncertainty exceeds its 64-entry limit")
        residual = [_text(item, "residual uncertainty", 4096)
                    for item in self.residual_uncertainty]
        report = {
            "schema": 1, "primary": self.primary, "cleanup": self.cleanup,
            "interrupted": self.interrupted, "metadata": _copy_metadata(self.metadata),
            "checks": {name: dict(row) for name, row in self._checks.items()},
            "residual_uncertainty": residual,
            "capture": {"lost": self._capture_lost, "error": self._capture_error,
                        "transcript_bytes": self._transcript_bytes,
                        "tx_bytes": self._tx_bytes, "rx_bytes": self._rx_bytes,
                        "records": self._records, "retained_tail_bytes": len(self._tail),
                        "report_tail_omitted_bytes": max(
                            0, len(self._tail) - MAX_REPORT_TAIL_BYTES),
                        "tail": self._tail[-MAX_REPORT_TAIL_BYTES:].decode("ascii")},
            "exit_code": self._outcome(),
        }
        self._json(report)
        return report

    @staticmethod
    def _json(report: dict) -> bytes:
        """Encode a report within its total byte limit without nonfinite JSON values."""
        encoded = bytearray()
        encoder = json.JSONEncoder(ensure_ascii=False, allow_nan=False, sort_keys=True, indent=2)
        for part in encoder.iterencode(report):
            chunk = part.encode("utf-8")
            if len(encoded) + len(chunk) + 1 > MAX_REPORT_BYTES:
                raise EvidenceError("report exceeds 512 KiB limit")
            encoded.extend(chunk)
        encoded.extend(b"\n")
        return bytes(encoded)

    @staticmethod
    def _summary(report: dict) -> bytes:
        """Render bounded machine state with metadata preserved as labelled JSON."""
        lines = ["# TUSB32x session evidence", "",
                 f"Primary: {report['primary']}. Cleanup: {report['cleanup']}.", "",
                 f"Exit code: {report['exit_code']}. Interrupted: {report['interrupted']}.", "",
                 "| Required check | Status | Detail |", "| --- | --- | --- |"]
        for name, row in report["checks"].items():
            lines.append(f"| {_markdown(name)} | {row['status']} | {_markdown(row['detail'])} |")
        if not report["checks"]:
            lines.extend(["", "No required proof was recorded."])
        lines.extend(["", "Residual uncertainty:", ""])
        lines.extend(f"- {_markdown(item)}" for item in report["residual_uncertainty"])
        capture = report["capture"]
        lines.extend(["", f"Capture lost: {capture['lost']}. {_markdown(capture['error'])}", "",
                      f"Transcript bytes: {capture['transcript_bytes']}; "
                      f"TX payload bytes: {capture['tx_bytes']}; "
                      f"RX payload bytes: {capture['rx_bytes']}; records: {capture['records']}.",
                      f"Retained tail bytes: {capture['retained_tail_bytes']}; "
                      f"tail bytes omitted from report: {capture['report_tail_omitted_bytes']}.",
                      "", "Metadata (host and firmware remain separate):", "",
                      "    " + _markdown(json.dumps(report["metadata"], ensure_ascii=False)),
                      "", "Retained transcript tail:", ""])
        lines.extend("    " + line for line in capture["tail"].splitlines())
        lines.extend(["", "report.json is authoritative if this summary is stale.",
                      "Serial records alone do not establish physical hardware qualification.", ""])
        return "\n".join(lines).encode("utf-8")

    def _replace(self, name: str, data: bytes) -> None:
        """Flush one same-directory temporary file before atomic replacement; always unlink it."""
        temporary: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(dir=self.directory, prefix=f".{name}.",
                                             suffix=".tmp", delete=False) as stream:
                temporary = Path(stream.name)
                if stream.write(data) != len(data):
                    raise OSError("short report write")
                stream.flush()
                os.fsync(stream.fileno())
            os.replace(temporary, self.directory / name)
        finally:
            if temporary is not None:
                temporary.unlink(missing_ok=True)

    def checkpoint(self) -> None:
        """Save JSON then Markdown individually with os.replace; raise on save failure.

        Validate and render both before replacing either. A failed replacement
        preserves that file's previous bytes and removes its temporary file.
        JSON may be newer than Markdown if the second replacement fails.
        """
        report = self.snapshot()
        encoded = self._json(report)
        summary = self._summary(report)
        try:
            self._replace("report.json", encoded)
            self._replace("summary.md", summary)
        except OSError as error:
            raise EvidenceError(f"cannot save evidence checkpoint: {error}") from error

    def exit_code(self) -> int:
        """Return 0 complete PASS, 1 failed proof, 2 incomplete, or 130 interrupted.

        Cleanup PASS never repairs primary/check FAIL. All declared checks must
        PASS and at least one must exist. Cleanup NOT RUN means none was needed.
        Capture loss, missing proof or residual uncertainty prevents exit zero.
        Invalid public report state raises EvidenceError instead of returning PASS.
        """
        return self.snapshot()["exit_code"]

    def close(self) -> None:
        """Flush and close the transcript once; latch I/O failure and raise EvidenceError.

        Does not checkpoint implicitly. The caller can checkpoint after close,
        including a failed close, so the report can preserve evidence loss.
        """
        if self._closed:
            return
        self._closed = True
        try:
            try:
                self._stream.flush()
            finally:
                self._stream.close()
        except (OSError, ValueError) as error:
            raise self._lost(f"transcript close failed: {error}") from error
