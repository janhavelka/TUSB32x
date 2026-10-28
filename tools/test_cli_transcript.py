"""Exercise persistent real C++ CLI processes and reject damaged correlated records.

Pass --host with the compiled tusb32x_cli_host executable. Commands share staging,
request IDs, operation ownership and nonce state. All reads have finite deadlines;
the host is terminated on failure, and no serial device or physical bus is opened.
"""

from __future__ import annotations

import argparse
import json
import queue
import subprocess
import threading
import time
from pathlib import Path
import tempfile
from collections.abc import Callable, Mapping, Sequence

from tusb32x_cli_contract import ROWS, SCHEMAS, parse_record, validate_record, validate_exchange
from tusb32x_hil_artifacts import Artifacts
from tusb32x_hil_runner import Session, SessionBlocked
from tusb32x_hil_plans import ObservationPlan, PlanFailure, run_plan
from tusb32x_hil_fixtures import run_fixture_plan
from tusb32x_hil_fixture_setup import FIXTURE_SUITES, load_setup


class PersistentHost:
    """Own one subprocess, asynchronous stdout reader and monotonically framed requests."""

    def __init__(self, executable: str, *options: str) -> None:
        """Start the fixture without startup commands or hardware actions."""
        self.process = subprocess.Popen(
            [executable, *options], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, bufsize=0,
        )
        self.lines: queue.Queue[bytes | None] = queue.Queue()
        self.request = 0
        self.nonce = "none"
        self.schema_kinds: set[str] = set()
        self.samples: dict[str, dict[str, str]] = {}
        self.human_lines: list[str] = []
        self.records: dict[int, list[dict[str, str]]] = {}
        self.operations: dict[str, tuple[int, str, str]] = {}
        self.completed: set[str] = set()
        self.closed_requests: set[int] = set()
        self.last_ack = 0
        self.reader = threading.Thread(target=self._read, daemon=True)
        self.reader.start()

    def _read(self) -> None:
        """Publish complete output lines; EOF is an explicit sentinel for finite waits."""
        assert self.process.stdout is not None
        for line in iter(self.process.stdout.readline, b""):
            self.lines.put(line)
        self.lines.put(None)

    def command(
        self, text: str, command: str, *, nonce: str | None = None,
        code: str = "OK", raw: bytes | None = None, wait: bool = True,
    ) -> list[dict[str, str]]:
        """Send one line, collect matching END within 3 s and validate all real records.

        raw permits binary poisoned input. Returned dictionaries preserve payload
        evidence; this method never changes driver state outside the actual CLI.
        """
        assert self.process.stdin is not None
        self.request += 1
        if nonce is not None:
            self.nonce = nonce
        self.process.stdin.write(raw if raw is not None else (text + "\n").encode("ascii"))
        self.process.stdin.flush()
        deadline = time.monotonic() + 3
        while True:
            record = self._next(deadline)
            assert record is not None  # EOF is allowed only while closing the session.
            if record.get("request") != str(self.request):
                continue
            if record["type"] == "ACK":
                if record["command"] != command or record["nonce"] != self.nonce:
                    raise AssertionError("fresh acknowledgement correlation changed")
                if record["action"] == "ADMITTED":
                    self.operations[record["token"]] = (self.request, command, self.nonce)
                    if not wait:
                        if record["code"] != "IN_PROGRESS":
                            raise AssertionError("admission lost IN_PROGRESS")
                        return self.records[self.request]
            if record["type"] == "END":
                break
        records = self.records[self.request]
        validate_exchange(records, self.request, command, self.nonce)
        if records[-1]["code"] != code:
            raise AssertionError(f"{text!r}: expected {code}, received {records[-1]}")
        return records

    def _next(self, deadline: float, *, allow_eof: bool = False) -> dict[str, str] | None:
        """Read one machine record within the finite deadline and validate session ordering."""
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise AssertionError("fixture response deadline expired")
            try:
                line = self.lines.get(timeout=remaining)
            except queue.Empty:
                raise AssertionError("fixture response deadline expired") from None
            if line is None:
                if allow_eof:
                    return None
                raise AssertionError("fixture ended before request closure")
            if not line.startswith(b"TUSB "):
                self.human_lines.append(line.decode("ascii"))
                continue
            record = parse_record(line.decode("ascii"))
            if record["type"] in {"LOSS", "SESSION"}:
                raise AssertionError("fixture session lost records or request capacity")
            self.samples[record["type"]] = record
            if record["type"] not in self.schema_kinds:
                self.schema_kinds.add(record["type"])
                for required in SCHEMAS[record["type"]]:
                    missing = dict(record)
                    del missing[required]
                    expect_rejection(lambda missing=missing: validate_record(missing))
            request = int(record["request"])
            if request in self.closed_requests:
                raise AssertionError("record followed an already closed request")
            if record["type"] in {"ACK", "MARK"}:
                if request != self.last_ack + 1 or request > self.request:
                    raise AssertionError("fresh request acknowledgement regressed or skipped")
                self.last_ack = request
            elif request > self.last_ack or request not in self.records:
                raise AssertionError("payload preceded its input acknowledgement")
            self.records.setdefault(request, []).append(record)
            if record["type"] == "END":
                own = self.records[request]
                ack = own[0]
                command = "hilmark" if ack["type"] == "MARK" else ack["command"]
                validate_exchange(own, request, command, ack["nonce"])
                if ack.get("action") == "ADMITTED":
                    token = ack["token"]
                    if token in self.completed or token not in self.operations:
                        raise AssertionError("duplicate or foreign spontaneous completion")
                    self.completed.add(token)
                self.closed_requests.add(request)
            return record

    def completion(self, token: str, *, code: str = "OK") -> list[dict[str, str]]:
        """Collect the sole spontaneous closure for a previously admitted token."""
        deadline = time.monotonic() + 3
        while token not in self.completed:
            self._next(deadline)
        records = self.records[self.operations[token][0]]
        if records[-1]["code"] != code:
            raise AssertionError(f"operation {token} expected {code}, received {records[-1]}")
        return records

    def close(self) -> None:
        """Require clean process exit and validate all trailing output through bounded EOF.

        Older open jobs/help may close while the fixture drains. Every sent request
        must then be acknowledged and closed; a zero exit code alone proves neither.
        """
        assert self.process.stdin is not None
        self.process.stdin.close()
        try:
            result = self.process.wait(timeout=3)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait(timeout=3)
            raise AssertionError("fixture failed to stop at EOF") from None
        assert self.process.stderr is not None
        error = self.process.stderr.read().decode("utf-8", errors="replace")
        if result != 0 or error:
            raise AssertionError(f"fixture exit={result}, stderr={error!r}")
        deadline = time.monotonic() + 3
        while self._next(deadline, allow_eof=True) is not None:
            pass
        if self.last_ack != self.request or set(self.records) != self.closed_requests:
            raise AssertionError("fixture ended with unclosed requests")


def expect_rejection(action: Callable[[], object]) -> None:
    """Require a zero-argument validation callable to raise ValueError."""
    try:
        action()
    except ValueError:
        return
    raise AssertionError("damaged record or closure was accepted")


def exercise_terminal_ordering(executable: str) -> None:
    """Reject late/foreign records injected after an actual fixture response.

    A closed local request has the same framing protection as a completed job.
    Open older operations may still finish after newer queries, as exercised by
    exercise_operations; closure is tracked per request, never by completion order.
    """
    cases = ((damage, at_close) for damage in (
        "duplicate_end", "contradictory_end", "late_payload", "future_end"
    ) for at_close in (False, True))
    for damage, at_close in cases:
        host = PersistentHost(executable)
        try:
            records = host.command("version", "version")
            kind = "VERSION" if damage == "late_payload" else "END"
            injected = dict(next(record for record in records if record["type"] == kind))
            if damage == "contradictory_end":
                injected.update(outcome="FAILED", code="I2C_TIMEOUT", detail="-7")
            elif damage == "future_end":
                injected["request"] = str(host.request + 1)
            # Injection retains real schema-valid bytes; only session ordering is damaged.
            line = "TUSB " + kind + " " + " ".join(
                key + "=" + value for key, value in injected.items() if key != "type"
            ) + "\n"
            parse_record(line)
            host.lines.put(line.encode("ascii"))
            try:
                if at_close:
                    host.close()
                else:
                    host._next(time.monotonic() + 3)
            except AssertionError as error:
                expected = ("payload preceded its input acknowledgement" if damage == "future_end"
                            else "record followed an already closed request")
                if str(error) != expected:
                    raise AssertionError(
                        "damaged ordering failed for an unrelated reason"
                    ) from error
            else:
                raise AssertionError(f"accepted {damage} after a completed request")
            if not at_close:
                host.close()
        except BaseException:
            if host.process.poll() is None:
                host.process.kill()
                host.process.wait(timeout=3)
            raise

    for automatic in (False, True):
        host = PersistentHost(executable)
        try:
            host.command("bind", "bind")
            if not automatic:
                host.command("job auto 0", "job.auto")
            started = host.command("init", "init", wait=False)
            token = payload(started, "ACK")["token"]
            try:
                host.close()
            except AssertionError as error:
                if automatic or str(error) != "fixture ended with unclosed requests":
                    raise
            else:
                if not automatic:
                    raise AssertionError("accepted EOF with an active unclosed request")
                if token not in host.completed:
                    raise AssertionError("EOF drain discarded a legitimate pending completion")
        except BaseException:
            if host.process.poll() is None:
                host.process.kill()
                host.process.wait(timeout=3)
            raise

    host = PersistentHost(executable)
    try:
        # Queue real incremental help without collecting it first. EOF draining
        # must validate its ACK, rows and closure rather than discard the output.
        assert host.process.stdin is not None
        host.request += 1
        host.process.stdin.write(b"help all\n")
        host.process.stdin.flush()
        host.close()
        help_rows = [record for record in host.records[1] if record["type"] == "HELP"]
        if len(help_rows) != len(ROWS):
            raise AssertionError("EOF drain discarded pending incremental help")
    except BaseException:
        if host.process.poll() is None:
            host.process.kill()
            host.process.wait(timeout=3)
        raise


def parser_negatives(records: list[dict[str, str]]) -> None:
    """Mutate actual positive responses and reject missing, duplicate and contradictory data."""
    ack = next(r for r in records if r["type"] == "ACK")
    line = "TUSB ACK " + " ".join(f"{key}={value}" for key, value in ack.items() if key != "type")
    validate_record(ack)
    assert parse_record(line + "\n")["request"] == ack["request"]
    for malformed in (
        line + " schema=1\n", line.replace("schema=1", "schema=2") + "\n",
        line.replace(" code=OK", "") + "\n", line.replace("request=1", "request=0") + "\n",
        line.replace("detail=0", "detail=2147483648") + "\n",
        line.replace("code=OK", "code=FAKE") + "\n", "TUSB UNKNOWN schema=1\n",
        line + "\x1b[0m\n", "TUSB ACK " + "x" * 512 + "\n",
        line.replace("code=OK", "code=CONFIGURATION_UNKNOWN").replace(
            "action=QUERY", "action=STAGED") + "\n",
    ):
        expect_rejection(lambda malformed=malformed: parse_record(malformed))
    request, nonce = int(ack["request"]), ack["nonce"]
    validate_exchange(records, request, "version", nonce)
    expect_rejection(lambda: validate_exchange(records[:-1], request, "version", nonce))
    expect_rejection(lambda: validate_exchange(records + [records[-1]], request, "version", nonce))
    expect_rejection(lambda: validate_exchange([ack] + records, request, "version", nonce))
    expect_rejection(lambda: validate_exchange(records, request + 1, "version", nonce))
    loss = {"type": "LOSS", "schema": "1", "dropped_records": "1", "dropped_bytes": "12"}
    expect_rejection(lambda: validate_exchange(records + [loss], request, "version", nonce))
    altered = [dict(r) for r in records]
    altered[-1]["detail"] = "-2147483648"
    expect_rejection(lambda: validate_exchange(altered, request, "version", nonce))
    altered[-1]["detail"] = "0"
    altered[-1]["nonce"] = "foreign"
    expect_rejection(lambda: validate_exchange(altered, request, "version", nonce))
    altered[-1]["nonce"] = nonce
    altered[-1]["outcome"] = "FAILED"
    expect_rejection(lambda: validate_exchange(altered, request, "version", nonce))


def maximum_width_records(samples: Mapping[str, Mapping[str, str]]) -> None:
    """Bound every schema and all help rows using full-width typed values and tokens.

    Synthetic combinations conservatively maximize each field independently; real
    C++ boundary fixtures separately execute formatting and queue storage. Records
    include the wire newline and final C terminator in the 512-byte assertion.
    """
    records = {kind: dict(record) for kind, record in samples.items()}
    records["LOSS"] = {
        "type": "LOSS", "schema": "1", "dropped_records": "4294967295",
        "dropped_bytes": "4294967295",
    }
    records["SESSION"] = {
        "type": "SESSION", "schema": "1", "code": "TOKEN_EXHAUSTED", "detail": "0",
        "reason": "request_exhausted",
    }
    if set(records) != set(SCHEMAS):
        raise AssertionError(f"missing executed schema samples: {set(SCHEMAS) - records.keys()}")
    maximums = {
        "nonce": "n" * 32, "request": "18446744073709551615",
        "staged_generation": "4294967295", "desired_generation": "4294967295",
        "applied_generation": "4294967295", "generation": "4294967295",
        "config_generation": "4294967295",
        "configuration": "UNCONFIGURED", "variant": "tusb320lai", "address": "103",
        "configured_variant": "tusb320lai", "observed_variant": "tusb320lai",
        "git": "g" * 40, "tree": "t" * 8, "build": "b" * 64,
        "name": "f" * 32, "version": "v" * 32, "target": "t" * 32,
        "bus": "b" * 32, "framing": "f" * 32,
        "bus_hz": "4294967295", "timeout_ms": "4294967295",
        "sda": "-2147483648", "scl": "-2147483648", "irq": "-2147483648",
        "adapter_writes": "4294967295", "adapter_write_reads": "4294967295",
        "dropped_records": "4294967295", "dropped_bytes": "4294967295",
        "raw08": "255", "raw09": "255", "raw0a": "255", "raw45": "255",
        "observed_valid_fields": "255", "profile_fields": "255",
        "attachment": "LOCAL_SOURCE", "local_role": "NOT_APPLICABLE", "peer_role": "NOT_APPLICABLE",
        "orientation": "NOT_APPLICABLE", "detected_current": "CHARGE_THROUGH_500_MA",
        "sequence": "18446744073709551615", "observed_ms": "18446744073709551615",
        "age_ms": "9223372036854775807", "advertised_current": "unknown",
        "accessory": "DEBUG_LOCAL_DFP", "active_cable": "unknown", "interrupt_pending": "unknown",
        "coherence": "CHANGED_DURING_READ",
        "token": "18446744073709551615", "origin_request": "18446744073709551615",
        "deadline_ms": "18446744073709551615", "ready_at_ms": "18446744073709551615",
        "started_ms": "18446744073709551615", "completed_ms": "18446744073709551615",
        "callbacks": "4294967295", "last_poll_callbacks": "255",
        "kind": "ACKNOWLEDGE_INTERRUPT", "state": "TIMED_OUT", "phase": "FINISHING",
        "effect": "INDETERMINATE", "termination": "DISABLED", "reset": "MAY_BE_PENDING",
        "profile_valid_fields": "65535", "status_valid_fields": "65535",
        "value": "255", "threshold": "4294967295", "managed_streak": "4294967295",
        "success": "4294967295", "failures": "4294967295", "streak": "4294967295",
        "attempts": "4294967295", "last_kind": "RECOVER_DEAD_BATTERY",
        "last_code": "CONFIGURATION_MISMATCH", "last_detail": "-2147483648",
        "last_success_ms": "18446744073709551615", "last_failure_ms": "18446744073709551615",
        "time_ms": "18446744073709551615", "expected_attempts": "4294967295",
        "expected_success": "4294967295", "expected_failures": "4294967295",
        "reason": "external-reset",
    }
    for kind, record in records.items():
        if kind in ("LOSS", "SESSION", "HELP"):
            continue
        for key, value in maximums.items():
            if key in record:
                record[key] = value
        if kind in ("ACK", "END"):
            record.update(
                command=max((row["command"] for row in ROWS), key=len),
                code="CONFIGURATION_MISMATCH", detail="-2147483648",
            )
        if kind == "ACK":
            record["action"] = "UNCHANGED"
        if kind == "END":
            record["outcome"] = "FAILED"
        if kind == "RESULT":
            record.update(state="FAILED", code="CONFIGURATION_MISMATCH", detail="-2147483648")
        if kind == "JOB":
            record.update(code="CONFIGURATION_MISMATCH", detail="-2147483648")
        if kind == "FAILURE":
            record.update(code="CONFIGURATION_MISMATCH", detail="-2147483648")
        if kind == "CONSOLE":
            record.update(queued="4096", available="4096")
        if kind in {"PROFILE", "DECODE_PROFILE"}:
            record.update(
                valid_fields="65535", mode="unknown",
                current="unknown", duty="unknown", debounce="unknown", preference="try-source",
                ufpaccessory="unknown", termination="unknown", rd_rp="unknown",
                disable_ufp_accessory="unknown", disable_term="unknown", disable_rd_rp="unknown",
            )
        if kind == "PROFILE":
            record.update(source="observed", available="0")
        if kind == "REG":
            record["source"] = "supplied"
        if kind == "DRV":
            record["state"] = "DEGRADED"
        if kind == "PIN":
            record.update(available="0", value="unknown")
        if kind == "IDENTITY":
            record.update(bytes="3032334253555400", identity_valid="1", revision="255",
                          revision_valid="1")
        if kind == "CONFIG_DETAIL":
            record.update(mismatch_valid="1", mismatch_register="255", mismatch_expected="255",
                          mismatch_observed="255", mismatch_mask="255")
        if kind == "XFER":
            record.update(saturated="1", assertion="SATURATED")
        if kind == "PORT":
            record.update(raw_valid_mask="3", valid_fields="511")
    help_base = records.pop("HELP")
    candidates = list(records.values())
    for row in ROWS:
        help_record = dict(help_base)
        help_record.update(
            nonce=maximums["nonce"], request=maximums["request"], canonical=row["command"],
        )
        help_record.update({key: row[key] for key in SCHEMAS["HELP"][4:]})
        candidates.append(help_record)
    for record in candidates:
        kind = record["type"]
        line = (
            "TUSB " + kind + " "
            + " ".join(f"{key}={record[key]}" for key in SCHEMAS[kind]) + "\n"
        )
        if len(line.encode("ascii")) + 1 > 512:
            raise AssertionError(f"maximum {kind} record exceeds fixed storage: {len(line) + 1}")
        parse_record(line)


def exercise(host: PersistentHost) -> int:
    """Execute real cache, stage, alias, marker and malformed-input routes in one session."""
    version = host.command("ver", "version")
    parser_negatives(version)
    firmware = next(r for r in version if r["type"] == "FIRMWARE")
    if (
        firmware["git"] != "unknown" or firmware["tree"] != "unknown"
        or firmware["build"] != "host-fixture"
    ):
        raise AssertionError("fixture must report its actual supplied identity")
    host.command("hilmark case_20_begin", "hilmark", nonce="case_20_begin")
    for text, canonical in (
        ("color off", "color"), ("verbose 1", "verbose"), ("bus", "bus"), ("console", "console"),
        ("capabilities", "capabilities"), ("model", "variant"), ("addr", "addr"),
        ("snapshot", "sample"), ("freshness", "freshness"), ("cfg", "profile.show"),
        ("settings", "profile.show"), ("profile", "profile.show"), ("profile show", "profile.show"),
        ("mode dfp", "mode"), ("current 3a0", "current"), ("duty 30", "duty"),
        ("debounce 118", "debounce"), ("preference try-sink", "preference"),
        ("ufpaccessory off", "ufpaccessory"), ("termination off", "termination"),
        ("rd-rp off", "rd-rp"),
        ("profile validate", "profile.validate"), ("profile reset", "profile.reset"),
        ("variant tusb320 0x60", "variant"), ("debounce 133", "debounce"), ("addr 97", "addr"),
        ("variant tusb320lai 0x47", "variant"), ("bind", "bind"), ("unbind", "end"),
        ("forget-evidence", "forget-evidence"),
    ):
        host.command(text, canonical)
    host.command("profile discard", "profile.discard", code="CONFIGURATION_UNKNOWN")
    from check_cli_contract import compare_help

    for topic in (
        "all", "common", "connection", "port", "profile", "lifecycle", "profile.reset", "ver",
    ):
        records = host.command("? " + topic, "help")
        if topic == "all":
            compare_help(records, ROWS)
    for text in (
        "unknown", "mode dfp extra", "addr -1", "addr 0x47g", "hilmark none", "help unknown",
    ):
        host.command(text, "invalid", code="INVALID_PARAM")
    host.command("binary poison", "invalid", code="INVALID_PARAM", raw=b"mode ufp\x00\x08\n")
    host.command("fragmented", "mode", raw=b"mo" + b"de dfx\x08p\r\n")
    host.command("hilmark case_20_end", "hilmark", nonce="case_20_end")
    host.command("version", "version")
    return host.request


def payload(records: Sequence[Mapping[str, str]], kind: str) -> Mapping[str, str]:
    """Require and return exactly one payload of the requested type."""
    matches = [record for record in records if record["type"] == kind]
    if len(matches) != 1:
        raise AssertionError(f"expected exactly one {kind}, received {len(matches)}")
    return matches[0]


def result_negatives(records: Sequence[Mapping[str, str]], command: str) -> None:
    """Reject incomplete, duplicated and misattributed successful result deliveries."""
    ack = payload(records, "ACK")
    request, nonce = int(ack["request"]), ack["nonce"]
    validate_exchange(records, request, command, nonce)
    for kind in ("RESULT", "RESULT_DETAIL"):
        expect_rejection(lambda kind=kind: validate_exchange(
            [r for r in records if r["type"] != kind], request, command, nonce,
        ))
        expect_rejection(lambda kind=kind: validate_exchange(
            list(records) + [dict(payload(records, kind))], request, command, nonce,
        ))
    for kind, key in (("RESULT", "token"), ("RESULT_DETAIL", "token"),
                      ("RESULT_DETAIL", "origin_request"), ("END", "token")):
        changed = [dict(record) for record in records]
        target = next(record for record in changed if record["type"] == kind)
        target[key] = str(int(target[key]) + 1)
        expect_rejection(
            lambda changed=changed: validate_exchange(changed, request, command, nonce),
        )
    changed = [dict(record) for record in records]
    result = next(record for record in changed if record["type"] == "RESULT")
    result.update(state="FAILED", code="I2C_TIMEOUT", detail="-2147483648")
    # Delivery succeeds independently of the result's original hardware failure.
    validate_exchange(changed, request, command, nonce)
    for state in ("CANCELLED", "TIMED_OUT"):
        contradictory = dict(result, state=state)
        expect_rejection(lambda contradictory=contradictory: validate_record(contradictory))


def operation(host: PersistentHost, text: str, canonical: str, *, code: str = "OK") -> str:
    """Execute one real managed operation and explicitly consume its exact retained result."""
    records = host.command(text, canonical, code=code)
    result = payload(records, "RESULT")
    token = result["token"]
    copied = host.command("result take " + token, "result.take")
    if payload(copied, "RESULT")["code"] != code:
        raise AssertionError("delivery OK changed the failed/successful operation payload")
    return token


def exercise_operations(host: PersistentHost) -> None:
    """Run aliases, manual budgets, retained fences, nonce changes and explicit mutations."""
    bus = payload(host.command("bus", "bus"), "BUS")
    if bus["adapter_writes"] != "0" or bus["adapter_write_reads"] != "0":
        raise AssertionError("original local transcript invoked transport")
    if payload(host.command("job auto", "job.auto"), "JOB")["auto"] != "1":
        raise AssertionError("automatic stepping must default on")
    host.command("init", "init", code="NOT_BOUND")
    host.command("bind", "bind")
    host.command("verify", "verify", code="CONFIGURATION_UNKNOWN")
    host.command("apply", "apply", code="NOT_INITIALIZED")
    for text in (
        "init 0", "probe -1", "initialize 2147483648", "read 20", "status 1",
        "profile read 1 extra", "apply +1", "verify 1ms", "reconcile 0x10",
        "irq read 10", "irq ack 0", "irq service 2147483648", "reset 0",
        "recover-dead-battery -1", "job auto on", "job step 256", "job step -1",
        "cancel 0", "stop 18446744073709551616", "result 0", "result take",
        "result take 0", "job result 1 extra", "job start init", "profile commit",
        "help nonexistent", "reg", "diag extra",
    ):
        host.command(text, "invalid", code="INVALID_PARAM")
    host.command("help job", "help")
    host.command("help interrupt", "help")
    host.command("profile reset", "profile.reset")
    host.command("job auto 0", "job.auto")
    initial = host.command("progress", "job.status")
    if payload(initial, "JOB")["auto"] != "0":
        raise AssertionError("manual mode was not retained")
    admitted = host.command("initialize 2147483647", "init", wait=False)
    token = payload(admitted, "ACK")["token"]
    origin_nonce = host.nonce
    host.command("hilmark active_new_marker", "hilmark", nonce="active_new_marker")
    host.command("probe", "init", code="BUSY")
    host.command("result " + token, "result", code="RESULT_NOT_AVAILABLE")
    status = payload(host.command("job status", "job.status"), "JOB")
    deadline = status["deadline_ms"]
    if status["callbacks"] != "0" or status["state"] != "ACTIVE":
        raise AssertionError("manual admission or cached handlers performed transport")
    zero = payload(host.command("job step 0", "job.step"), "JOB")
    if zero["callbacks"] != "0" or zero["last_poll_callbacks"] != "0":
        raise AssertionError("zero budget invoked transport")
    previous = 0
    for budget in (255, 1, 0, 1, 255, 1, 255, 1, 255, 1, 255, 1, 255, 1, 255, 1):
        job = payload(host.command("job step " + str(budget), "job.step"), "JOB")
        callbacks = int(job["callbacks"])
        if callbacks - previous not in ({0} if budget == 0 else {0, 1}):
            raise AssertionError("basic terminal callback exceeded one attempt per owner turn")
        if job["deadline_ms"] != deadline:
            raise AssertionError("manual stepping renewed operation deadline")
        previous = callbacks
        if job["state"] != "ACTIVE":
            break
    else:
        raise AssertionError("bounded initialization failed to terminate")
    completed = host.completion(token)
    if any(record["nonce"] != origin_nonce for record in completed):
        raise AssertionError("new marker changed original operation attribution")
    for text in ("result", "job result", "result " + token):
        query = host.command(text, "result")
        result_negatives(query, "result")
        copied = payload(query, "RESULT")
        if copied["origin_request"] != str(host.operations[token][0]) or copied["token"] != token:
            raise AssertionError("repeated snapshot changed retained origin")
    host.command("read", "read", code="RESULT_PENDING")
    host.command("result take " + str(int(token) + 1), "result.take", code="TOKEN_MISMATCH")
    result_negatives(host.command("result take " + token, "result.take"), "result.take")
    host.command("result take " + token, "result.take", code="RESULT_NOT_AVAILABLE")
    host.command("job auto 1", "job.auto")
    for text, canonical in (
        ("probe", "init"), ("init", "init"), ("read", "read"), ("status", "read"),
        ("profile read 1000", "profile.read"), ("apply", "apply"),
        ("verify", "verify"), ("selfcheck", "verify"), ("selftest", "verify"),
        ("reconcile", "reconcile"), ("irq read", "irq.read"),
    ):
        operation(host, text, canonical)
    # A write unrelated to 09 preserves the pending latch; ACK then preserves applied duty.
    host.command("current 1a5", "current")
    operation(host, "apply", "apply")
    sample = host.command("sample", "sample")
    if payload(sample, "PORT_DETAIL")["interrupt_pending"] != "1":
        raise AssertionError("unrelated profile write consumed IRQ evidence")
    host.command("duty 30", "duty")
    operation(host, "irq ack", "irq.ack")
    operation(host, "profile read", "profile.read")
    profiles = host.command("profile", "profile.show")
    observed = next(r for r in profiles if r["type"] == "PROFILE" and r["source"] == "observed")
    staged = next(r for r in profiles if r["type"] == "PROFILE" and r["source"] == "staged")
    if observed["duty"] != "50" or staged["duty"] != "30":
        raise AssertionError("IRQ acknowledgement copied staged duty into hardware")
    operation(host, "irq service", "irq.service")
    host.command("recover-dead-battery", "recover-dead-battery", code="UNSUPPORTED_FEATURE")
    operation(host, "reset", "reset")
    # Stop aliases close their own local request; the cancelled job keeps its old origin.
    host.command("job auto 0", "job.auto")
    for cancel in ("cancel", "stop", "job cancel"):
        started = host.command("init 2147483647", "init", wait=False)
        token = payload(started, "ACK")["token"]
        host.command(cancel, "cancel")
        host.completion(token, code="CANCELLED")
        taken = host.command("result take " + token, "result.take")
        if payload(taken, "RESULT")["state"] != "CANCELLED":
            raise AssertionError("successful delivery erased cancellation")
    # Resume an older job under a newer status request. ACK order is independent of completion.
    host.command("job auto 1", "job.auto")
    operation(host, "init", "init")
    host.command("job auto 0", "job.auto")
    started = host.command("apply 2147483647", "apply", wait=False)
    token = payload(started, "ACK")["token"]
    query_request = host.request + 1
    host.command("job status", "job.status")
    host.command("job auto 1", "job.auto")
    host.completion(token)
    if host.operations[token][0] >= query_request:
        raise AssertionError("test failed to interleave old work and newer query")
    host.command("result take " + token, "result.take")
    operation(host, "init 1", "init", code="TIMEOUT")
    host.command("end", "end")
    historical = host.command("snapshot", "sample")
    if payload(historical, "PORT")["invalidated"] != "1":
        raise AssertionError("end left historical port evidence usable")
    host.command("forget-evidence", "forget-evidence")
    host.command("variant tusb320 0x60", "variant")
    host.command("bind", "bind")
    operation(host, "init", "init")
    operation(host, "recover-dead-battery", "recover-dead-battery")


def exercise_fault(executable: str, mode: str) -> None:
    """Preserve full signed transport failure and separate failed payload from delivery OK."""
    host = PersistentHost(executable, "--fault", mode)
    try:
        host.command("bind", "bind")
        if mode == "readback":
            operation(host, "init", "init")
            host.command("current 1a5", "current")
            records = host.command("apply", "apply", code="I2C_TIMEOUT")
        else:
            records = host.command("init", "init", code="I2C_TIMEOUT")
        result = payload(records, "RESULT")
        if result["detail"] != "-2147483648":
            raise AssertionError("backend detail was narrowed or replaced")
        if mode == "readback" and (
            result["effect"] not in {"PARTIAL", "INDETERMINATE"}
            or result["configuration"] != "UNKNOWN"
        ):
            raise AssertionError("failure after write lost uncertain hardware evidence")
        copied = host.command("result take " + result["token"], "result.take")
        if payload(copied, "RESULT")["code"] != "I2C_TIMEOUT":
            raise AssertionError("delivery success concealed the failed operation")
        if mode == "readback":
            operation(host, "reconcile", "reconcile")
            snapshot = host.command("profile", "profile.show")
            observed = next(
                r for r in snapshot if r["type"] == "PROFILE" and r["source"] == "observed"
            )
            if observed["current"] != "1a5":
                raise AssertionError("readback failure fixture lost the accepted hardware write")
        host.close()
    except BaseException:
        if host.process.poll() is None:
            host.process.kill()
            host.process.wait(timeout=3)
        raise


def exercise_diagnostics(host: PersistentHost) -> None:
    """Execute raw/pure/cache routes, exclusions, accounting and explicit local invalidation."""
    host.command("end", "end")
    host.command("forget-evidence", "forget-evidence")
    host.command("variant tusb320lai 0x47", "variant")
    host.command("bind", "bind")
    operation(host, "init", "init")
    before = host.command("diag", "diag")
    for kind, key, value in (
        ("DIAG", "last_kind", "UNKNOWN"), ("MANAGED", "last_detail", "2147483648"),
        ("IDENTITY", "bytes", "not_hex_bytes"), ("IRQ", "pending_before", "2"),
        ("XFER", "source", "managed_callbacks"), ("XFER", "saturated", "1"),
    ):
        damaged = dict(payload(before, kind))
        damaged[key] = value
        expect_rejection(lambda damaged=damaged: validate_record(damaged))
    managed = dict(payload(before, "MANAGED"))
    irq = dict(payload(before, "IRQ"))
    stats = payload(before, "XFER")
    attempts, success, failures = (int(stats[key]) for key in ("attempts", "success", "failures"))
    for text in ("reg 0x00", "rreg 0x07", "rawread 0x08", "reg 0x09", "reg 0xA0"):
        records = host.command(text, "reg")
        if payload(records, "END")["callbacks"] != "1":
            raise AssertionError("raw read lost its terminal callback count")
        attempts += 1
        success += 1
    for text in ("decode 0x08 0xff", "decode 0x09 0", "decode 0x0A 8"):
        records = host.command(text, "decode")
        damaged = dict(payload(records, "REG"), value="256")
        expect_rejection(lambda damaged=damaged: validate_record(damaged))
    for text, canonical in (("irq", "irq"), ("drv", "drv"), ("health", "drv"),
                            ("state", "drv"), ("xfer_stats", "xfer_stats"),
                            ("counters", "xfer_stats")):
        host.command(text, canonical)
    after = host.command("diag", "diag")
    for kind, expected in (("MANAGED", managed), ("IRQ", irq)):
        observed = dict(payload(after, kind))
        for key in ("request", "nonce"):
            observed.pop(key)
            expected.pop(key)
        if observed != expected:
            raise AssertionError("raw/pure/cached diagnostics changed managed evidence")
    host.command(f"xfer_assert {attempts} {success} {failures}", "xfer_assert")
    mismatch = host.command(f"xfer_assert {attempts + 1}", "xfer_assert", code="INVALID_PARAM")
    if payload(mismatch, "XFER")["assertion"] != "FAIL":
        raise AssertionError("counter mismatch did not emit failed source evidence")
    for text in ("reg -1", "rawread 0x100", "decode 8 -1", "decode 8 256", "wreg 8",
                 "rawwrite 8 0x100", "irq extra", "drv extra", "xfer_stats extra",
                 "xfer_assert -1", "xfer_assert 4294967296", "xfer_assert 1 2",
                 "xfer_assert 1 2 3 4", "xfer_assert 0x01", "invalidate", "invalidate reset"):
        host.command(text, "invalid", code="INVALID_PARAM")
    host.command("decode 0x0b 0", "decode", code="INVALID_PARAM")
    host.command("reg 0x0b", "reg", code="INVALID_PARAM")
    host.command("wreg 0x00 0", "wreg", code="UNSUPPORTED_FEATURE")
    host.command("wreg 0x0A 8", "wreg", code="UNSUPPORTED_FEATURE")
    host.command("wreg 0x08 0x80", "wreg")
    attempts += 1
    success += 1
    host.command(f"xfer_assert {attempts} {success} {failures}", "xfer_assert")
    uncertain = payload(host.command("diag", "diag"), "DIAG")
    host.command("xfer_reset", "xfer_reset")
    cleared = host.command("diag", "diag")
    if payload(cleared, "XFER")["attempts"] != "0":
        raise AssertionError("idle reset did not clear all-source totals")
    for key in ("configuration", "effect", "config_write_unresolved", "reset"):
        if payload(cleared, "DIAG")[key] != uncertain[key]:
            raise AssertionError("counter reset repaired configuration/effect evidence")
    for reason in ("external-reset", "power-loss", "target-change"):
        host.command("invalidate " + reason, "invalidate")
    host.command("job auto 0", "job.auto")
    started = host.command("init", "init", wait=False)
    token = payload(started, "ACK")["token"]
    host.command("reg 8", "reg", code="BUSY")
    host.command("wreg 8 128", "wreg", code="BUSY")
    host.command("xfer_reset", "xfer_reset", code="BUSY")
    host.command("diag", "diag")
    host.command("cancel", "cancel")
    host.completion(token, code="CANCELLED")
    host.command("xfer_reset", "xfer_reset", code="RESULT_PENDING")
    host.command("reg 8", "reg", code="RESULT_PENDING")
    host.command("result take " + token, "result.take")
    host.command("job auto 1", "job.auto")
    host.command("end", "end")
    host.command("forget-evidence", "forget-evidence")
    host.command("variant tusb320 0x60", "variant")
    host.command("decode 0xA0 0", "decode", code="UNSUPPORTED_FEATURE")
    host.command("bind", "bind")
    host.command("reg 0xA0", "reg", code="UNSUPPORTED_FEATURE")
    host.command("intpin", "intpin", code="UNSUPPORTED_FEATURE")


def exercise_diagnostic_fixtures(executable: str) -> None:
    """Execute optional input levels/failures and raw callback faults in isolated processes."""
    for value, code, available in (("0", "OK", "1"), ("1", "OK", "1"),
                                   ("unavailable", "UNSUPPORTED_FEATURE", "0"),
                                   ("failure", "INVALID_CONFIG", "0")):
        host = PersistentHost(executable, "--input", value)
        try:
            pin = payload(host.command("intpin", "intpin", code=code), "PIN")
            if pin["available"] != available or pin["value"] != (
                value if available == "1" else "unknown"
            ):
                raise AssertionError("application input validity or level changed")
            host.close()
        except BaseException:
            if host.process.poll() is None:
                host.process.kill()
                host.process.wait(timeout=3)
            raise
    for mode, text, canonical in (("transport", "reg 8", "reg"),
                                   ("write", "wreg 8 128", "wreg")):
        host = PersistentHost(executable, "--fault", mode)
        try:
            host.command("bind", "bind")
            if mode == "write":
                operation(host, "init", "init")
            before = payload(host.command("diag", "diag"), "MANAGED")
            records = host.command(text, canonical, code="I2C_TIMEOUT")
            ack, end = payload(records, "ACK"), payload(records, "END")
            if ack["action"] != "ATTEMPTED" or end["detail"] != "-2147483648":
                raise AssertionError("raw failure lost callback evidence or backend detail")
            after = host.command("diag", "diag")
            for key in SCHEMAS["MANAGED"][3:]:
                if payload(after, "MANAGED")[key] != before[key]:
                    raise AssertionError("raw failure contaminated managed health")
            if payload(after, "XFER")["failures"] != "1":
                raise AssertionError("raw failure missing from all-callback totals")
            if mode == "write" and payload(after, "DIAG")["configuration"] != "UNKNOWN":
                raise AssertionError("failed raw write retained configuration trust")
            host.close()
        except BaseException:
            if host.process.poll() is None:
                host.process.kill()
                host.process.wait(timeout=3)
            raise


class SessionFixtureIo:
    """Adapt the existing persistent C++ fixture to the borrowed session I/O contract.

    This test-only pipe uses finite queue waits and small command writes. It
    makes no serial backend timing or physical hardware claim.
    """

    def __init__(self, host):
        """Borrow the existing reader/process and retain at most one bounded line fragment."""
        self.host = host
        self.pending = b""
        self.writes = 0

    def read(self, size, timeout):
        """Return a fragment or finite timeout; EOF before framing is an error."""
        if not self.pending:
            try:
                self.pending = self.host.lines.get(timeout=timeout)
            except queue.Empty:
                return b""
            if self.pending is None:
                raise OSError("C++ fixture ended before session closure")
            if len(self.pending) > 1024:
                raise AssertionError("fixture line exceeds session bound")
        result, self.pending = self.pending[:size], self.pending[size:]
        return result

    def write(self, data, timeout):
        """Offer one bounded command; the Session checks elapsed time before/after it."""
        if timeout <= 0 or len(data) > 192:
            raise AssertionError("invalid fixture write budget/length")
        self.writes += 1
        return self.host.process.stdin.write(data)

    def close(self):
        """Close test input, require the real fixture's checks to pass, and bound teardown."""
        self.host.process.stdin.close()
        if self.host.process.wait(timeout=3) != 0:
            raise AssertionError("session fixture detected bad callback/frame/budget")
        self.host.reader.join(timeout=3)


def exercise_session(executable):
    """Run real marker/version/admit/wait/take/restore exchanges and failed payload delivery."""
    for fault in (False, True):
        host = PersistentHost(executable, *(('--fault', 'transport') if fault else ()))
        io = SessionFixtureIo(host)
        with tempfile.TemporaryDirectory(prefix="tusb32x-session-") as temporary:
            artifacts = Artifacts(Path(temporary), time.monotonic)
            try:
                session = Session(io, artifacts)
                identity = session.begin()
                if identity.build != "host-fixture" or identity.git != "unknown":
                    raise AssertionError("fixture provenance was replaced by host identity")
                begin_nonce = session.nonce
                before = io.writes
                try:
                    session.command("apply")
                except SessionBlocked:
                    pass
                else:
                    raise AssertionError("unknown fixture identity authorized a device mutation")
                if io.writes != before:
                    raise AssertionError("blocked identity guard sent bytes")
                for malformed in ("mode nonsense", "bus extra", "profile.show"):
                    rejected = session.command(malformed)
                    if (rejected.command != "invalid" or
                            rejected.end["code"] != "INVALID_PARAM" or
                            rejected.end["callbacks"] != "0" or not session.valid):
                        raise AssertionError("real parser rejection damaged verified session")
                session.command("color off")
                session.command("verbose 1")
                session.command("mode dfp")
                session.command("bind")
                admitted = session.command("init")
                if not admitted.admitted or admitted.ack["code"] != "IN_PROGRESS":
                    raise AssertionError("real operation admission lost IN_PROGRESS")
                result = session.wait_operation()
                expected = "I2C_TIMEOUT" if fault else "OK"
                if result["code"] != expected or (fault and result["detail"] != "-2147483648"):
                    raise AssertionError("real payload failed to preserve operation status/detail")
                copied = session.command("result " + admitted.ack["token"])
                if payload(copied.records, "RESULT")["code"] != expected:
                    raise AssertionError("query changed retained operation outcome")
                delivered = session.take_result()
                if delivered.end["code"] != "OK" or (
                        payload(delivered.records, "RESULT")["code"] != expected):
                    raise AssertionError("delivery and payload status were conflated")
                session.finish()
                if session.nonce == begin_nonce:
                    raise AssertionError("end marker reused begin nonce")
                artifacts.primary = "FAIL" if fault else "PASS"
                artifacts.check("operation", "FAIL" if fault else "PASS", expected)
                artifacts.checkpoint()
                if artifacts.exit_code() != (1 if fault else 0):
                    raise AssertionError("delivery/cleanup repaired a failed payload")
                io.close()
            except BaseException:
                if host.process.poll() is None:
                    host.process.kill()
                    host.process.wait(timeout=3)
                raise
            finally:
                artifacts.close()


def exercise_observation_plans(executable):
    """Run the complete production readonly plan against normal and failing real CLI buses."""
    for fault in (False, True):
        options = ("--observation", "1") + (("--fault", "transport") if fault else ())
        host = PersistentHost(executable, *options)
        io = SessionFixtureIo(host)
        with tempfile.TemporaryDirectory(prefix="tusb32x-plan-") as temporary:
            artifacts = Artifacts(Path(temporary), time.monotonic)
            try:
                session = Session(io, artifacts)
                try:
                    run_plan(session, ObservationPlan())
                except PlanFailure:
                    if not fault:
                        raise
                else:
                    if fault:
                        raise AssertionError("failed callback became a successful readonly plan")
                report = artifacts.snapshot()
                if report["primary"] != ("FAIL" if fault else "PASS"):
                    raise AssertionError("native plan lost its primary outcome")
                if session.operation is not None or session.valid:
                    raise AssertionError("native plan left its owned result/session open")
                operation = report["metadata"]["operation"]
                if not operation["taken"]:
                    raise AssertionError("native plan failed to consume its exact result")
                if fault:
                    if (operation["result"]["code"] != "I2C_TIMEOUT" or
                            operation["result"]["detail"] != "-2147483648"):
                        raise AssertionError("native plan erased original failed callback evidence")
                elif report["metadata"]["observations"]["count"] < 1:
                    raise AssertionError("readonly plan never established a fresh observation")
                elif (report["metadata"]["observations"]["detached"] !=
                      report["metadata"]["observations"]["count"]):
                    raise AssertionError("valid repeated detached observations were not counted")
                # Continue the same process after the production plan's end marker. These
                # cache queries independently prove actual restoration and zero device writes.
                host.request, host.nonce = session.last_request, session.nonce
                host.last_ack = session.last_request
                bus = payload(host.command("bus", "bus"), "BUS")
                if bus["adapter_writes"] != "0" or int(bus["adapter_write_reads"]) < 1:
                    raise AssertionError("readonly plan wrote hardware or performed no reads")
                before = report["metadata"]["local_before"]
                after_console = payload(host.command("console", "console"), "CONSOLE")
                after_job = payload(host.command("job status", "job.status"), "JOB")
                after_profiles = host.command("profile show", "profile.show")
                after_profile = next(r for r in after_profiles if r["type"] == "PROFILE"
                                     and r["source"] == "staged")
                if (after_job["auto"] != before["auto"] or
                        any(after_console[key] != before["console"][key]
                            for key in ("color", "verbose")) or
                        any(after_profile[key] != before["profile"][key]
                            for key in SCHEMAS["PROFILE"][3:] if key != "generation")):
                    raise AssertionError("native plan did not restore saved local values")
                host.close()
            finally:
                if host.process.poll() is None:
                    host.process.kill()
                    host.process.wait(timeout=3)
                artifacts.close()


def exercise_plan_denied_take(executable):
    """Keep a production plan failed and unrestored when real output reservation denies take."""
    host = PersistentHost(executable, "--observation", "1", "--fault", "take-full")
    io = SessionFixtureIo(host)
    with tempfile.TemporaryDirectory(prefix="tusb32x-denied-plan-") as temporary:
        artifacts = Artifacts(Path(temporary), time.monotonic)
        try:
            session = Session(io, artifacts)
            try:
                run_plan(session, ObservationPlan())
            except PlanFailure:
                pass
            else:
                raise AssertionError("denied take did not stop the production plan")
            report = artifacts.snapshot()
            operation = report["metadata"]["operation"]
            if (report["primary"] != "FAIL" or operation["taken"] or session.valid or
                    session.operation is None or not report["residual_uncertainty"] or
                    not report["metadata"].get("unrestored_local")):
                raise AssertionError("denied take lost owned/unrestored failed evidence")
            # Independent fixture inspection proves core retention. The failed production
            # Session remains invalid and never retries the take or replays local settings.
            host.request, host.nonce = session.last_request, session.nonce
            host.last_ack = session.last_request
            job = payload(host.command("job status", "job.status"), "JOB")
            copied = payload(host.command("result", "result"), "RESULT")
            if job["result_pending"] != "1" or any(
                    copied[key] != operation["result"][key] for key in SCHEMAS["RESULT"][3:]):
                raise AssertionError("failed plan consumed or changed the retained core result")
            host.close()
        finally:
            if host.process.poll() is None:
                host.process.kill()
                host.process.wait(timeout=3)
            artifacts.close()


def exercise_session_turns(executable):
    """Prove real older completion, one-turn cached progress and denied take retention."""
    for congested in (False, True):
        options = ("--observation", "1") + (("--fault", "take-full") if congested else ())
        host = PersistentHost(executable, *options)
        io = SessionFixtureIo(host)
        with tempfile.TemporaryDirectory(prefix="tusb32x-turns-") as temporary:
            artifacts = Artifacts(Path(temporary), time.monotonic)
            try:
                session = Session(io, artifacts)
                session.begin()
                session.command("bind")
                session.command("job auto 0")
                admitted = session.command("init")
                origin = admitted.ack["request"]
                zero = payload(session.command("job step 0").records, "JOB")
                if zero["callbacks"] != "0" or zero["last_poll_callbacks"] != "0":
                    raise AssertionError("real zero callback budget advanced work")
                session.command("job auto 1")
                before = payload(session.command("bus").records, "BUS")
                newer_request = session.last_request
                after = payload(session.command("bus").records, "BUS")
                if int(after["adapter_write_reads"]) - int(before["adapter_write_reads"]) != 1:
                    raise AssertionError("cached query turn did not advance exactly one callback")
                rejected = session.command("mode nonsense")
                if rejected.end["code"] != "INVALID_PARAM" or rejected.end["callbacks"] != "0":
                    raise AssertionError("malformed handler acquired callbacks from older work")
                for _ in range(8):
                    if session.operation.complete:
                        break
                    session.command("job status")
                result = session.wait_operation()
                if (result["origin_request"] != origin or int(origin) >= newer_request or
                        result["code"] != "OK" or result["callbacks"] != "6"):
                    raise AssertionError("older completion lost exact origin or callback evidence")
                if congested:
                    denied = session.take_result()
                    if denied.end["code"] != "BUSY" or session.operation is None:
                        raise AssertionError("output-full take consumed the retained core result")
                    copied = payload(session.command("result").records, "RESULT")
                    if any(copied[key] != result[key] for key in SCHEMAS["RESULT"][3:]):
                        raise AssertionError("denied take changed the original result")
                taken = session.take_result()
                if taken.end["code"] != "OK" or session.operation is not None:
                    raise AssertionError("exact successful take did not release ownership")
                session.finish()
                io.close()
            finally:
                if host.process.poll() is None:
                    host.process.kill()
                    host.process.wait(timeout=3)
                artifacts.close()


class ScriptedPhysicalFixture:
    """Supply independent CC/attachment/INT register inputs; never hardware evidence.

    The launch-only file changes RO physical inputs, not requested profile controls.
    Every version is applied once by the C++ fixture, allowing real W1C ACK/service
    writes to clear INT without the next ordinary CLI command reasserting it.
    """

    # TI SLLSEQ8D Tables 8/9, pp17-18: literal LAI RO/IRQ inputs, independent
    # of the plan's expected fields and the production decoder/composer.
    STATES = {
        "initial-detached": (0x00, 0x10),
        "cleanup-detached": (0x00, 0x10),
        "orientation-cc1": (0x00, 0x90),
        "orientation-cc2": (0x00, 0xB0),
        "orientation-detach": (0x00, 0x10),
        "accessory-audio": (0x08, 0xD0),
        "accessory-debug": (0x0C, 0xD0),
        "accessory-charge-through": (0x0A, 0xD0),
        "accessory-debug-local-dfp": (0x0C, 0xD0),
        "accessory-debug-local-ufp": (0x0E, 0xD0),
        "accessory-detach-audio": (0x00, 0x10),
        "accessory-detach-debug": (0x00, 0x10),
        "accessory-detach-charge-through": (0x00, 0x10),
        "accessory-detach-debug-local-dfp": (0x00, 0x10),
        "accessory-detach-debug-local-ufp": (0x00, 0x10),
        "sink-default": (0x00, 0x90),
        "sink-1a5": (0x10, 0x90),
        "sink-3a0": (0x30, 0x90),
        "sink-detach": (0x00, 0x10),
        "source-connect": (0x00, 0x50),
        "source-default": (0x00, 0x50),
        "source-1a5": (0x00, 0x50),
        "source-3a0": (0x00, 0x50),
        "source-detach": (0x00, 0x10),
        "drp-resolved": (0x00, 0x90),
        "drp-detach": (0x00, 0x10),
        "irq-assert": (0x00, 0x90),
        "irq-reassert": (0x00, 0x90),
        "irq-coalesced": (0x00, 0x90),
        "irq-detach": (0x00, 0x10),
    }

    def __init__(self, path):
        """Start detached with a pending modeled latch before the first session command."""
        self.path = path
        self.version = 0
        self.steps = []
        self.path.write_text("0 0 16\n", encoding="ascii")

    def __call__(self, step, deadline):
        """Publish literal LAI status bytes for the named fixture transition.

        Expected decoded values belong to the production plan, so they cannot
        determine these independent physical inputs. Writable profile bits are
        preserved by the C++ model and remain controlled by actual driver writes.
        """
        if time.monotonic() >= deadline:
            raise AssertionError("native scripted transition exceeded its finite deadline")
        raw08, raw09 = self.STATES[step["id"]]
        self.version += 1
        self.steps.append(step["id"])
        self.path.write_text(f"{self.version} {raw08} {raw09}\n", encoding="ascii")
        return True


def fixture_no_sleep(_duration):
    """Advance through real owner turns; the C++ fixture owns its simulated millisecond clock."""


def exercise_fixture_plans(executable):
    """Run four production plans and reject two wrong expectations through the real CLI/core.

    Only physical status inputs are scripted. Actual staging/apply/verify, 50% ACK
    preservation, unrelated duty-write latch preservation and service/recheck use
    the production operation engine and typed records. Saved reports explicitly
    identify all connector/CC/INT observations as modeled, with hardware NOT RUN.
    """
    scenarios = [(suite, None) for suite in FIXTURE_SUITES] + [
        ("orientation", ("orientation-cc1", "orientation", "CC2", "CC1")),
        ("role-current", ("sink-1a5", "detected_current", "MA_3000", "MA_1500")),
    ]
    for suite, damaged_expectation in scenarios:
        with tempfile.TemporaryDirectory(prefix="tusb32x-fixture-" + suite + "-") as temporary:
            root = Path(temporary)
            setup_path = root / "setup.json"
            setup_path.write_text(json.dumps({
                "board": "native simulated board; no physical hardware",
                "part": "modeled TUSB320LAI; no measured silicon",
                "adapter": "C++ recording callbacks",
                "wiring": "modeled CC1/CC2 and active-low INT; VBUS_DET modeled only",
                "addr_strap": "modeled 0x47", "enable": "modeled active-low EN_N asserted",
                "supply_mv": 5000, "partner": "modeled reversible source/sink/accessories",
                "operator": "deterministic native test; no physical attestation",
                "variant": "tusb320lai", "startup_ready": True,
                "capabilities": ["orientation", "audio", "debug", "charge-through",
                                 "debug-local-dfp", "debug-local-ufp", "role-current",
                                 "interrupt"],
            }), encoding="utf-8")
            setup = load_setup(setup_path, "native-model", "tusb320lai", (suite,))
            physical = ScriptedPhysicalFixture(root / "state.txt")
            host = PersistentHost(executable, "--fixture", "1", "--fixture-state",
                                  str(physical.path))
            io = SessionFixtureIo(host)
            artifacts = Artifacts(root / "evidence", time.monotonic)
            artifacts.metadata["native_simulation"] = {
                "physical_hardware": "NOT RUN",
                "evidence": "modeled physical inputs; real CLI and core operation engine",
            }
            try:
                session = Session(io, artifacts)
                expected = {"git": "0123456789abcdef0123456789abcdef01234567",
                            "framework": "host", "target": "native"}

                def transition(step, deadline):
                    """Damage only the plan's expectation; literal physical inputs stay fixed."""
                    if damaged_expectation and step["id"] == damaged_expectation[0]:
                        step["expect"][damaged_expectation[1]] = damaged_expectation[2]
                    return physical(step, deadline)

                try:
                    summary = run_fixture_plan(
                        session, ObservationPlan(suites=(suite,), run_timeout=20), setup, expected,
                        transition=transition, sleep=fixture_no_sleep,
                    )
                except PlanFailure as error:
                    if not damaged_expectation:
                        raise
                    _, field, wanted, observed = damaged_expectation
                    message = f"fixture {field}: expected {wanted}, observed {observed}"
                    if str(error) != message:
                        raise AssertionError("native oracle failed for unrelated reason") from error
                else:
                    if damaged_expectation:
                        raise AssertionError("native physical inputs followed damaged expectation")
                report = artifacts.snapshot()
                saved = json.loads((artifacts.directory / "report.json").read_text("utf-8"))
                if saved != report or saved["metadata"]["native_simulation"][
                        "physical_hardware"] != "NOT RUN":
                    raise AssertionError("native fixture report lost modeled/physical distinction")
                if damaged_expectation:
                    port = report["metadata"]["fixture_port"]["port"]
                    raw08, raw09 = ScriptedPhysicalFixture.STATES[damaged_expectation[0]]
                    if (report["primary"] != "FAIL" or artifacts.exit_code() != 1 or
                            report["metadata"]["hardware_cleanup"] != "BLOCKED" or
                            report["cleanup"] != "BLOCKED" or
                            not report["residual_uncertainty"] or session.operation is not None or
                            session.valid or int(port["raw08"]) & 0x3E != raw08 or
                            int(port["raw09"]) & 0xF0 != raw09 or
                            port[damaged_expectation[1]] != damaged_expectation[3] or
                            physical.steps[-1] != damaged_expectation[0] or
                            physical.steps.count(damaged_expectation[0]) != 1 or
                            report["metadata"]["fixture_progress"]["mutation_requests"] != 1 or
                            report["checks"][suite]["status"] != "FAIL"):
                        raise AssertionError("native mismatch lost failed independent raw evidence")
                    io.close()
                    continue
                if (report["primary"] != "PASS" or report["cleanup"] != "PASS" or
                        artifacts.exit_code() != 0 or report["residual_uncertainty"] or
                        report["metadata"]["hardware_cleanup"] != "PASS" or
                        session.operation is not None or session.valid):
                    raise AssertionError("native fixture plan lost successful owned cleanup proof")
                if (summary["mutation_requests"] < 1 or summary["operations"] < 1 or
                        summary["transitions"] != len(physical.steps) or
                        physical.steps[-1] != "cleanup-detached"):
                    raise AssertionError("native fixture plan skipped declared transitions/work")
                if suite == "interrupt":
                    detail = report["metadata"]["irq_service"]["detail"]
                    if detail["irq_after"] != "0" or detail["ack_confirmed"] != "1":
                        raise AssertionError("real bounded IRQ service failed to clear modeled INT")
                io.close()
            finally:
                if host.process.poll() is None:
                    host.process.kill()
                    host.process.wait(timeout=3)
                host.reader.join(timeout=3)
                artifacts.close()


def main() -> int:
    """Run the bounded transcript; return zero only after all records and fixture exit pass."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", required=True, help="Compiled persistent C++ fixture")
    args = parser.parse_args()
    host = PersistentHost(args.host)
    try:
        local_requests = exercise(host)
        if local_requests != 51:
            raise AssertionError("original local transcript sequence changed")
        exercise_operations(host)
        exercise_diagnostics(host)
        maximum_width_records(host.samples)
        requests = host.request
        host.close()
        for mode in ("transport", "readback"):
            exercise_fault(args.host, mode)
        exercise_diagnostic_fixtures(args.host)
        exercise_terminal_ordering(args.host)
        exercise_session(args.host)
        exercise_observation_plans(args.host)
        exercise_plan_denied_take(args.host)
        exercise_session_turns(args.host)
        exercise_fixture_plans(args.host)
    except BaseException:
        if host.process.poll() is None:
            host.process.kill()
            host.process.wait(timeout=3)
        raise
    print(f"CLI transcript: {requests} persistent requests and damaged-record checks passed; "
          "2 strict sessions, 3 native readonly plans, 2 owner-turn sessions and "
          "4 modeled physical-fixture plans and 2 independent mismatch rejections passed; "
          "physical hardware NOT RUN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
