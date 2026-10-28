"""Exercise persistent real C++ CLI processes and reject damaged correlated records.

Pass --host with the compiled tusb32x_cli_host executable. Commands share staging,
request IDs, operation ownership and nonce state. All reads have finite deadlines;
the host is terminated on failure, and no serial device or physical bus is opened.
"""

from __future__ import annotations

import argparse
import queue
import subprocess
import threading
import time
from collections.abc import Callable, Mapping, Sequence

from tusb32x_cli_contract import ROWS, SCHEMAS, parse_record, validate_record


def validate_exchange(
    records: Sequence[Mapping[str, str]], request: int, command: str, nonce: str
) -> None:
    """Check one ACK/MARK and exactly one matching END, raising ValueError.

    Inputs are already schema-checked machine records for one command. Closure
    requires matching request, nonce, command and status; markers use MARK alone.
    Admitted operations require one terminal result/detail, while result query
    delivery status remains independent of its terminal payload status.
    """
    acknowledgements = [r for r in records if r["type"] in ("ACK", "MARK")]
    ends = [r for r in records if r["type"] == "END"]
    if len(acknowledgements) != 1 or len(ends) != 1:
        raise ValueError("request needs one acknowledgement and one END")
    for record in records:
        if record["type"] in ("LOSS", "SESSION"):
            raise ValueError("session lost framing or request capacity")
        if record.get("request") != str(request) or record.get("nonce") != nonce:
            raise ValueError("record correlation changed within request")
    ack, end = acknowledgements[0], ends[0]
    if end["command"] != command:
        raise ValueError("wrong terminal command")
    if command == "hilmark":
        if ack["type"] != "MARK" or end["code"] != "OK":
            raise ValueError("marker must acknowledge with MARK")
    elif ack["type"] != "ACK" or ack["command"] != command:
        raise ValueError("acknowledgement command mismatch")
    elif ack["action"] == "ADMITTED":
        results = [r for r in records if r["type"] == "RESULT"]
        details = [r for r in records if r["type"] == "RESULT_DETAIL"]
        if len(results) != 1 or len(details) != 1:
            raise ValueError("admitted work needs exactly one result and detail")
        result, detail = results[0], details[0]
        if ack["code"] != "IN_PROGRESS" or ack["token"] == "0":
            raise ValueError("invalid operation admission")
        for payload in (result, detail):
            if payload["origin_request"] != str(request) or payload["token"] != ack["token"]:
                raise ValueError("operation result origin/token mismatch")
        for key, payload_key in (("code", "code"), ("detail", "detail"),
                                 ("callbacks", "callbacks"), ("outcome", "state")):
            if end[key] != result[payload_key]:
                raise ValueError("terminal closure contradicts operation result")
        if end["token"] != ack["token"]:
            raise ValueError("terminal closure changed operation token")
    elif ack["code"] != end["code"] or ack["detail"] != end["detail"]:
        jobs = [r for r in records if r["type"] == "JOB"]
        if (command != "job.step" or len(jobs) != 1 or jobs[0]["state"] != "ACTIVE"
                or jobs[0]["code"] != end["code"] or jobs[0]["detail"] != end["detail"]):
            raise ValueError("contradictory local terminal status")
    if command in {"result", "result.take"} and end["code"] == "OK":
        results = [r for r in records if r["type"] == "RESULT"]
        details = [r for r in records if r["type"] == "RESULT_DETAIL"]
        if len(results) != 1 or len(details) != 1:
            raise ValueError("successful result delivery requires one result and detail")
        result, detail = results[0], details[0]
        if (ack["token"] == "0" or result["token"] != ack["token"]
                or detail["token"] != ack["token"] or end["token"] != ack["token"]
                or detail["origin_request"] != result["origin_request"]):
            raise ValueError("result delivery changed its token or origin")
    if ack.get("action") != "ADMITTED" and end["callbacks"] != "0":
        raise ValueError("local handler performed callbacks")
    if (end["outcome"] == "SUCCEEDED") != (end["code"] == "OK"):
        raise ValueError("contradictory local terminal outcome")


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
        self.records: dict[int, list[dict[str, str]]] = {}
        self.operations: dict[str, tuple[int, str, str]] = {}
        self.completed: set[str] = set()
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

    def _next(self, deadline: float) -> dict[str, str]:
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
                raise AssertionError("fixture ended before request closure")
            if not line.startswith(b"TUSB "):
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
            if record["type"] in {"ACK", "MARK"}:
                if request != self.last_ack + 1 or request > self.request:
                    raise AssertionError("fresh request acknowledgement regressed or skipped")
                self.last_ack = request
            elif request > self.last_ack:
                raise AssertionError("payload preceded its input acknowledgement")
            self.records.setdefault(request, []).append(record)
            if record["type"] == "END":
                own = self.records[request]
                ack = own[0]
                if ack.get("action") == "ADMITTED":
                    token = ack["token"]
                    if token in self.completed or token not in self.operations:
                        raise AssertionError("duplicate or foreign spontaneous completion")
                    origin, command, nonce = self.operations[token]
                    validate_exchange(own, origin, command, nonce)
                    self.completed.add(token)
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
        """Close stdin and require clean exit within 3 s; always reap a failed fixture."""
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


def expect_rejection(action: Callable[[], object]) -> None:
    """Require a zero-argument validation callable to raise ValueError."""
    try:
        action()
    except ValueError:
        return
    raise AssertionError("damaged record or closure was accepted")


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
                code="CONFIGURATION_UNKNOWN", detail="-2147483648",
            )
        if kind == "ACK":
            record["action"] = "UNCHANGED"
        if kind == "END":
            record["outcome"] = "FAILED"
        if kind == "RESULT":
            record.update(state="FAILED", code="CONFIGURATION_UNKNOWN", detail="-2147483648")
        if kind == "JOB":
            record.update(code="CONFIGURATION_UNKNOWN", detail="-2147483648")
        if kind == "CONSOLE":
            record.update(queued="4096", available="4096")
        if kind == "PROFILE":
            record.update(
                source="observed", available="0", valid_fields="0", mode="unknown",
                current="unknown", duty="unknown", debounce="unknown", preference="try-source",
                ufpaccessory="unknown", termination="unknown", rd_rp="unknown",
                disable_ufp_accessory="unknown", disable_term="unknown", disable_rd_rp="unknown",
            )
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
    for topic in (
        "all", "common", "connection", "port", "profile", "lifecycle", "profile.reset", "ver",
    ):
        records = host.command("? " + topic, "help")
        if topic == "all":
            actual = [r for r in records if r["type"] == "HELP"]
            if len(actual) != len(ROWS):
                raise AssertionError("executed help and host row counts differ")
            for observed, expected in zip(actual, ROWS):
                if observed["canonical"] != expected["command"]:
                    raise AssertionError("executed help canonical row differs")
                for key, value in expected.items():
                    if key not in ("command", "syntax") and observed[key] != value:
                        raise AssertionError(f"executed help metadata differs: {key}")
    for text in (
        "unknown", "mode dfp extra", "addr -1", "addr 0x47g", "hilmark none", "help registers",
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
        "help diagnostics", "reg 8", "diag",
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
        maximum_width_records(host.samples)
        requests = host.request
        host.close()
        for mode in ("transport", "readback"):
            exercise_fault(args.host, mode)
    except BaseException:
        if host.process.poll() is None:
            host.process.kill()
            host.process.wait(timeout=3)
        raise
    print(f"CLI transcript: {requests} persistent requests and damaged-record checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
