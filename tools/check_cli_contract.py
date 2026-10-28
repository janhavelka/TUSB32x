"""Compare fixed CLI metadata with execution of the explicitly supplied C++ fixture.

Use --host with tusb32x_cli_host. Each route runs in an isolated scripted-device
process; no physical bus, serial port or firmware is used. Help, actual dispatch,
typed records, terminal statuses and callback effects must all agree. All process
reads/exits are bounded by PersistentHost. --self-test-negative requires damaged
alias and record contracts to fail against actual fixture output.
Read-observation routes independently require zero adapter write callbacks; their
reported hardware-effect value alone is insufficient evidence of bus behavior.
"""

from __future__ import annotations

import argparse
import re
from collections.abc import Mapping, Sequence

from tusb32x_cli_contract import ROWS, SCHEMAS


def compare_help(
    records: Sequence[Mapping[str, str]], expected_rows: Sequence[Mapping[str, str]],
    human_lines: Sequence[str] | None = None,
) -> None:
    """Require ordered real HELP payloads to match every supplied metadata field.

    Optional human lines check the displayed syntax after removing ANSI color.
    Machine fields are shared by this checker and the persistent transcript.
    Raise AssertionError for missing, extra, reordered or changed rows.
    """
    actual = [record for record in records if record["type"] == "HELP"]
    if len(actual) != len(expected_rows):
        raise AssertionError("executed help and expected row counts differ")
    for observed, expected in zip(actual, expected_rows):
        if observed["canonical"] != expected["command"]:
            raise AssertionError("executed help canonical row differs")
        for key, value in expected.items():
            if key not in {"command", "syntax"} and observed[key] != value:
                raise AssertionError(f"executed help metadata differs: {key}")
        if human_lines is not None and not any(
            expected["syntax"] in re.sub(r"\x1b\[[0-9;]*m", "", line) for line in human_lines
        ):
            raise AssertionError("executed help syntax differs from expected row")


def _help(host, text: str, rows: Sequence[Mapping[str, str]]):
    """Execute one help route and compare its machine payload and human syntax lines."""
    start = len(host.human_lines)
    records = host.command(text, "help")
    compare_help(records, rows, host.human_lines[start:])
    return records


def _schemas(
    records: Sequence[Mapping[str, str]], schemas: Mapping[str, Sequence[str]],
) -> None:
    """Check supplied schema dependencies against records parsed from the actual process."""
    for record in records:
        required = schemas.get(record["type"])
        if required is None or set(required) - record.keys():
            raise AssertionError(f"executed {record['type']} violates expected record contract")


def _stop(host) -> None:
    """Kill/reap a still-running failed fixture; successful callers use its checked close."""
    if host.process.poll() is None:
        host.process.kill()
        host.process.wait(timeout=3)


def _aliases(row: Mapping[str, str]) -> list[str]:
    """Return executable alias words, converting fixed dotted subcommand IDs to spaces."""
    return [] if row["aliases"] == "none" else [
        alias.replace(".", " ") for alias in row["aliases"].split(",")
    ]


def _route(host, row: Mapping[str, str], spelling: str):
    """Prepare one legal route, execute its spelling and check records and callback effects.

    Setup uses only real CLI operations and explicitly takes each setup result.
    The tested command owns its own request; older cancellation completion remains
    separate. Return its parsed response for injected-schema validation.
    """
    from test_cli_transcript import operation, payload

    command = row["command"]
    text = spelling
    arguments = {
        "color": " off", "verbose": " 1", "hilmark": " parity_marker",
        "mode": " dfp", "current": " 1a5", "duty": " 30", "debounce": " 118",
        "preference": " try-sink", "ufpaccessory": " off", "termination": " off",
        "rd-rp": " off", "reg": " 0x08", "decode": " 0x09 0x14",
        "wreg": " 0x08 0x80", "invalidate": " external-reset",
    }
    text += arguments.get(command, "")
    need_ready = row["execution"] == "CORE_JOB" or command in {
        "profile.discard", "wreg", "result", "result.take", "cancel", "end", "xfer_reset",
    }
    if command == "recover-dead-battery":
        host.command("variant tusb320 0x60", "variant")
    if need_ready and command != "bind":
        host.command("bind", "bind")
        if command not in {"init", "result", "result.take", "cancel", "job.step"}:
            operation(host, "init", "init")
            if command in {"apply", "verify", "profile.discard"}:
                operation(host, "apply", "apply")
    if command == "reg":
        host.command("bind", "bind")
    if command in {"cancel", "job.step"}:
        host.command("job auto 0", "job.auto")
        # Fixture turns advance time while output drains. Keep this ready first
        # read alive through the counter query so step proves work, not expiry.
        host.command("init 2147483647", "init", wait=False)
    if command in {"result", "result.take"}:
        started = host.command("init", "init")
        token = payload(started, "RESULT")["token"]
        if command == "result.take":
            text += " " + token
    before = payload(host.command("bus", "bus"), "BUS")
    if command == "xfer_assert":
        text += " " + str(int(before["adapter_writes"]) + int(before["adapter_write_reads"]))
    nonce = "parity_marker" if command == "hilmark" else None
    records = host.command(text, command, nonce=nonce,
                           code="UNSUPPORTED_FEATURE" if command == "intpin" else "OK")
    after = payload(host.command("bus", "bus"), "BUS")
    delta = sum(int(after[key]) - int(before[key]) for key in (
        "adapter_writes", "adapter_write_reads",
    ))
    if row["effect"] == "READ_OBSERVATION" and (
        after["adapter_writes"] != before["adapter_writes"]
    ):
        raise AssertionError(f"{text}: read observation invoked a device write callback")
    if row["execution"] in {"PURE", "CACHE_ONLY", "PLATFORM_ACTION"} and delta != 0:
        raise AssertionError(f"{text}: bus-silent handler invoked transport")
    if row["execution"] == "RAW_I2C" and delta != 1:
        raise AssertionError(f"{text}: raw diagnostic did not invoke exactly one callback")
    if row["execution"] == "CORE_JOB":
        if command == "job.step":
            if delta != 1 or delta > int(row["max_callbacks"]) or delta != int(
                payload(records, "JOB")["last_poll_callbacks"]
            ):
                raise AssertionError("explicit step violated its one-poll callback bound")
        else:
            result = payload(records, "RESULT")
            if delta != int(result["callbacks"]):
                raise AssertionError("operation callbacks differ from actual fixture attempts")
            if row["effect"] == "READ_OBSERVATION" and result["effect"] != "NONE":
                raise AssertionError("read-only operation reported a device write effect")
    expected_types = set(row["records"].split(","))
    if command == "intpin":
        expected_types = {"ACK", "PIN", "END"}
    if {record["type"] for record in records} != expected_types:
        raise AssertionError(f"{text}: executed record types differ from row contract")
    if command == "hilmark" and payload(records, "MARK")["nonce"] != "parity_marker":
        raise AssertionError("marker did not preserve its input")
    if command == "decode" and payload(records, "DECODE")["value"] != "20":
        raise AssertionError("pure decode lost the supplied raw byte")
    if command == "reg" and payload(records, "REG")["value"] != "128":
        raise AssertionError("raw read lost the fixture byte")
    if command == "wreg" and payload(records, "REG")["source"] != "write":
        raise AssertionError("raw write did not identify its source")
    if command == "invalidate" and payload(records, "INVALIDATE")["reason"] != "external-reset":
        raise AssertionError("invalidation lost its explicit reason")
    if command == "xfer_assert" and payload(records, "XFER")["assertion"] != "PASS":
        raise AssertionError("exact transfer assertion did not pass")
    if command == "job.step":
        # Only this fixture-owned job remains active. Cleanup follows measured
        # route assertions, and its original request must close before clean EOF.
        token = payload(records, "JOB")["token"]
        host.command("cancel " + token, "cancel")
        host.completion(token, code="CANCELLED")
    return records


def check_contract(
    executable: str, *, expected_rows: Sequence[Mapping[str, str]] = ROWS,
    expected_schemas: Mapping[str, Sequence[str]] = SCHEMAS,
) -> int:
    """Execute all canonical/alias/help routes; return their count or raise on disagreement.

    expected_rows/schemas allow negative dependency injection without changing the
    firmware, production contract module or parser. Temporary subprocesses are
    closed on success and killed/reaped on every exception.
    """
    from test_cli_transcript import PersistentHost

    count = 0
    host = PersistentHost(executable)
    try:
        _schemas(_help(host, "help all", expected_rows), expected_schemas)
        for topic in sorted({row["topic"] for row in expected_rows}):
            records = _help(host, "help " + topic,
                            [row for row in expected_rows if row["topic"] == topic])
            _schemas(records, expected_schemas)
            count += 1
        for row in expected_rows:
            aliases = [] if row["aliases"] == "none" else row["aliases"].split(",")
            for name in [row["command"], *aliases]:
                records = _help(host, "help " + name, [row])
                _schemas(records, expected_schemas)
                count += 1
        host.close()
    except BaseException:
        _stop(host)
        raise
    for row in expected_rows:
        canonical = row["command"].replace(".", " ")
        for spelling in [canonical, *_aliases(row)]:
            host = PersistentHost(executable)
            try:
                records = _route(host, row, spelling)
                _schemas(records, expected_schemas)
                host.close()
            except BaseException:
                _stop(host)
                raise
            count += 1
    return count + 1


def negative_checks(executable: str) -> None:
    """Require broken alias and required-field dependencies to fail on real fixture output."""
    rows = [dict(row) for row in ROWS]
    rows[0]["aliases"] = "broken_alias"
    schemas = dict(SCHEMAS)
    schemas["VERSION"] = (*SCHEMAS["VERSION"], "missing_required_test_field")
    for options in ({"expected_rows": rows}, {"expected_schemas": schemas}):
        try:
            check_contract(executable, **options)
        except (AssertionError, ValueError):
            continue
        raise AssertionError("deliberately damaged executable contract was accepted")


def main() -> int:
    """Run the selected finite fixture check; return zero only after every assertion passes."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", required=True, help="Explicit compiled C++ fixture path")
    parser.add_argument("--self-test-negative", action="store_true")
    args = parser.parse_args()
    if args.self_test_negative:
        negative_checks(args.host)
        print("CLI contract: broken alias and record dependencies rejected")
    else:
        count = check_contract(args.host)
        print(f"CLI contract: {len(ROWS)} rows and {count} executed routes passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
