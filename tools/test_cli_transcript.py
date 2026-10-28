"""Exercise one persistent real C++ CLI process and reject damaged local records.

Pass --host with the compiled tusb32x_cli_host executable. Commands share staging,
request IDs and nonce state. All reads have finite deadlines; the host is terminated
on failure, and no serial device or physical bus is opened.
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
    """Check one local ACK/MARK and exactly one matching END, raising ValueError.

    Inputs are already schema-checked machine records for one command. Closure
    requires matching request, nonce, command and status; markers use MARK alone
    for acknowledgement. No operation admission is implemented by this stage.
    """
    acknowledgements = [r for r in records if r["type"] in ("ACK", "MARK")]
    ends = [r for r in records if r["type"] == "END"]
    if len(acknowledgements) != 1 or len(ends) != 1:
        raise ValueError("local request needs one acknowledgement and one END")
    for record in records:
        if record["type"] in ("LOSS", "SESSION"):
            raise ValueError("session lost framing or request capacity")
        if record.get("request") != str(request) or record.get("nonce") != nonce:
            raise ValueError("record correlation changed within local request")
    ack, end = acknowledgements[0], ends[0]
    if end["command"] != command or end["callbacks"] != "0":
        raise ValueError("wrong local command or callback count")
    if command == "hilmark":
        if ack["type"] != "MARK" or end["code"] != "OK":
            raise ValueError("marker must acknowledge with MARK")
    elif ack["type"] != "ACK" or ack["command"] != command:
        raise ValueError("acknowledgement command mismatch")
    elif ack["code"] != end["code"] or ack["detail"] != end["detail"]:
        raise ValueError("contradictory local terminal status")
    if (end["outcome"] == "SUCCEEDED") != (end["code"] == "OK"):
        raise ValueError("contradictory local terminal outcome")


class PersistentHost:
    """Own one subprocess, asynchronous stdout reader and monotonically framed requests."""

    def __init__(self, executable: str) -> None:
        """Start the fixture without startup commands or hardware actions."""
        self.process = subprocess.Popen(
            [executable], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, bufsize=0,
        )
        self.lines: queue.Queue[bytes | None] = queue.Queue()
        self.request = 0
        self.nonce = "none"
        self.schema_kinds: set[str] = set()
        self.samples: dict[str, dict[str, str]] = {}
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
        code: str = "OK", raw: bytes | None = None,
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
        records: list[dict[str, str]] = []
        deadline = time.monotonic() + 3
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise AssertionError(f"missing END for request {self.request}: {text!r}")
            try:
                line = self.lines.get(timeout=remaining)
            except queue.Empty:
                raise AssertionError(f"missing END for request {self.request}: {text!r}") from None
            if line is None:
                raise AssertionError("fixture ended before request closure")
            if line.startswith(b"TUSB "):
                record = parse_record(line.decode("ascii"))
                self.samples[record["type"]] = record
                if record["type"] not in self.schema_kinds:
                    self.schema_kinds.add(record["type"])
                    for required in SCHEMAS[record["type"]]:
                        missing = dict(record)
                        del missing[required]
                        expect_rejection(lambda missing=missing: validate_record(missing))
                records.append(record)
                if record["type"] == "END":
                    break
        validate_exchange(records, self.request, command, self.nonce)
        if records[-1]["code"] != code:
            raise AssertionError(f"{text!r}: expected {code}, received {records[-1]}")
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
    }
    for kind, record in records.items():
        if kind in ("LOSS", "SESSION", "HELP"):
            continue
        for key, value in maximums.items():
            if key in record:
                record[key] = value
        if kind in ("ACK", "END"):
            record.update(
                command="profile.validate", code="CONFIGURATION_UNKNOWN", detail="-2147483648",
            )
        if kind == "ACK":
            record["action"] = "UNCHANGED"
        if kind == "END":
            record["outcome"] = "FAILED"
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
    for text in ("unknown", "mode dfp extra", "addr -1", "addr 0x47g", "hilmark none", "help job"):
        host.command(text, "invalid", code="INVALID_PARAM")
    host.command("binary poison", "invalid", code="INVALID_PARAM", raw=b"mode ufp\x00\x08\n")
    host.command("fragmented", "mode", raw=b"mo" + b"de dfx\x08p\r\n")
    host.command("hilmark case_20_end", "hilmark", nonce="case_20_end")
    host.command("version", "version")
    maximum_width_records(host.samples)
    return host.request


def main() -> int:
    """Run the bounded transcript; return zero only after all records and fixture exit pass."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", required=True, help="Compiled persistent C++ fixture")
    args = parser.parse_args()
    host = PersistentHost(args.host)
    try:
        requests = exercise(host)
        host.close()
    except BaseException:
        if host.process.poll() is None:
            host.process.kill()
            host.process.wait(timeout=3)
        raise
    print(f"CLI transcript: {requests} persistent requests and damaged-record checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
