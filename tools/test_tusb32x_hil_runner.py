"""Exercise serial session framing, ownership and deadlines without hardware or sleeps.

The reactive peer emits independently schema-validated records in response to
actual TX bytes. Faults alter that wire output; tests use only public session APIs.
The separate artifact suite is included by load_tests for one CTest registration.
"""

from __future__ import annotations

from collections import deque
from contextlib import ExitStack, redirect_stderr, redirect_stdout
import io
from pathlib import Path
import json
import tempfile
import unittest
from unittest import mock

from tusb32x_cli_contract import COMMANDS, SCHEMAS, parse_record
import tusb32x_hil_runner as runner
from tusb32x_hil_artifacts import Artifacts


GIT = "1234567890abcdef1234567890abcdef12345678"


def record(record_type: str, request: int, nonce: str, **fields: object) -> dict[str, str]:
    """Build an independently validated schema-1 fixture with explicit typed values."""
    values = {key: "0" for key in SCHEMAS[record_type]}
    values.update(type=record_type, schema="1")
    if "request" in values:
        values.update(request=str(request), nonce=nonce)
    defaults = {
        "ACK": dict(command="bus", code="OK", action="QUERY", configuration="KNOWN"),
        "END": dict(command="bus", outcome="SUCCEEDED", code="OK"),
        "JOB": dict(kind="NONE", state="IDLE", phase="NONE", code="OK", auto="1"),
        "RESULT": dict(kind="REFRESH_PORT", state="SUCCEEDED", code="OK", effect="NONE",
                       token="7", origin_request=str(request), configuration="KNOWN"),
        "RESULT_DETAIL": dict(token="7", origin_request=str(request),
                              termination="ENABLED", reset="NONE"),
        "VERSION": dict(library="0.1.0"),
        "FIRMWARE": dict(git=GIT, tree="clean", build="test_build"),
        "FRAMEWORK": dict(name="host", version="test_1", target="native"),
        "BUS": dict(variant="tusb320lai", address="71", bound="1", bus="test_i2c",
                    framing="combined", bus_hz="100000", timeout_ms="20",
                    adapter_available="1"),
        "CONSOLE": dict(line_capacity="192", input_budget="64", output_capacity="8192",
                        record_capacity="512", drain_budget="256", available="8192"),
        "CAPABILITIES": dict(configured_variant="tusb320lai", observed_variant="tusb320lai",
                             profile_fields="255", advertisement_write="1",
                             duty_write="1", accessory_write="1"),
        "CONFIG": dict(configuration="KNOWN", has_desired="1", has_observed="1",
                       has_applied="1", raw08="0", raw09="0", raw0a="0", raw45="unknown"),
        "CONFIG_DETAIL": dict(termination="ENABLED", reset="NONE", mismatch_register="unknown",
                              mismatch_expected="unknown", mismatch_observed="unknown",
                              mismatch_mask="unknown"),
        "PROFILE": dict(source="staged", variant="tusb320lai", available="1", mode="drp",
                        current="default", duty="30", debounce="116", preference="normal",
                        ufpaccessory="on", termination="on", rd_rp="on", valid_fields="255",
                        disable_ufp_accessory="0", disable_term="0", disable_rd_rp="0"),
        "SESSION": dict(code="TOKEN_EXHAUSTED", reason="request_exhausted"),
    }
    values.update(defaults.get(record_type, {}))
    values.update({key: str(value) for key, value in fields.items()})
    parse_record(encode_record(values).decode("ascii"))
    return values


def encode_record(value: dict[str, str]) -> bytes:
    """Encode one fixture without depending on the session's parser or serializer."""
    return ("TUSB " + value["type"] + " " + " ".join(
        f"{key}={field}" for key, field in value.items() if key != "type"
    ) + "\n").encode("ascii")


class FakeClock:
    """Explicit monotonic seconds; empty reads advance by their requested timeout."""

    def __init__(self) -> None:
        """Start virtual time at a nonzero monotonic value."""
        self.now = 100.0

    def __call__(self) -> float:
        """Return virtual seconds without advancing the clock."""
        return self.now


class Peer:
    """Small reactive wire peer with fragment, latency and record-fault injection.

    No private Session fields are read or written. An admitted operation only
    completes when the test calls complete(), so query interleavings are exact.
    """

    def __init__(self, clock: FakeClock) -> None:
        """Create an idle firmware peer with explicitly controllable wire faults."""
        self.clock = clock
        self.pending = bytearray()
        self.writes: list[str] = []
        self.timeouts: list[tuple[str, float]] = []
        self.request = 0
        self.nonce = "none"
        self.fragments: deque[int] = deque()
        self.read_delay = 0.0
        self.write_delay = 0.0
        self.short_write = False
        self.response_hook = None
        self.write_error: BaseException | None = None
        self.read_error: BaseException | None = None
        self.close_error: BaseException | None = None
        self.identity = dict(git=GIT, tree="clean", build="test_build")
        self.color = "0"
        self.verbose = "0"
        self.auto = "1"
        self.staged = dict(mode="drp", current="default", duty="30", debounce="116",
                           preference="normal", ufpaccessory="on", termination="on", rd_rp="on")
        self.foreign_active = False
        self.foreign_result = False
        self.operation: dict | None = None
        self.token = 7
        self.closed = False

    def queue(self, records: list[dict[str, str]]) -> None:
        """Append raw validated fixtures to the simulated serial receive buffer."""
        self.pending.extend(b"".join(encode_record(value) for value in records))

    def _job(self, request: int, code: str = "OK") -> dict[str, str]:
        """Report current ownership with the status supplied by the CLI handler."""
        active = self.foreign_active or self.operation is not None and not self.operation["done"]
        retained = self.foreign_result or self.operation is not None and self.operation["done"]
        return record("JOB", request, self.nonce, auto=self.auto,
                      state="ACTIVE" if active else "SUCCEEDED" if retained else "IDLE",
                      kind="REFRESH_PORT" if active or retained else "NONE",
                      phase="READING" if active else "NONE",
                      token=self.token if active or retained else 0,
                      code=code, result_pending=int(retained))

    def _profile(self, request: int) -> list[dict[str, str]]:
        """Return configuration evidence and all four independently labelled profiles."""
        return [record("CONFIG", request, self.nonce),
                record("CONFIG_DETAIL", request, self.nonce)] + [
            record("PROFILE", request, self.nonce, source=source, **self.staged,
                   disable_ufp_accessory=int(self.staged["ufpaccessory"] == "off"),
                   disable_term=int(self.staged["termination"] == "off"),
                   disable_rd_rp=int(self.staged["rd_rp"] == "off"))
            for source in ("staged", "desired", "observed", "applied")
        ]

    def _console(self, request: int) -> dict[str, str]:
        """Report fixed console bounds and current local output settings."""
        return record("CONSOLE", request, self.nonce, color=self.color, verbose=self.verbose)

    def write(self, data: bytes, timeout: float) -> int:
        """Record exactly one command write and append the corresponding wire reply."""
        self.timeouts.append(("write", timeout))
        self.clock.now += self.write_delay
        if self.write_error is not None:
            raise self.write_error
        text = data.decode("ascii").rstrip("\r\n")
        self.writes.append(text)
        if self.short_write:
            return max(0, len(data) - 1)
        self.request += 1
        request = self.request
        parts = text.split()
        command = parts[0]
        if command == "hilmark":
            self.nonce = parts[1]
            records = [record("MARK", request, self.nonce),
                       record("END", request, self.nonce, command="hilmark")]
        else:
            canonical = ("profile." + parts[1] if command == "profile" else
                         "job." + (parts[1] if len(parts) > 1 else "status")
                         if command == "job" else
                         "result.take" if parts[:2] == ["result", "take"] else command)
            if canonical not in COMMANDS:
                canonical = "invalid"
            ack = record("ACK", request, self.nonce, command=canonical)
            end = record("END", request, self.nonce, command=canonical)
            payload = []
            if canonical == "version":
                payload = [record("VERSION", request, self.nonce),
                           record("FIRMWARE", request, self.nonce, **self.identity),
                           record("FRAMEWORK", request, self.nonce)]
            elif canonical == "bus":
                payload = [record("BUS", request, self.nonce)]
            elif canonical == "capabilities":
                payload = [record("CAPABILITIES", request, self.nonce)]
            elif canonical in {"console", "color", "verbose"}:
                if canonical != "console" and len(parts) > 1:
                    setattr(self, canonical, "1" if parts[1] in {"1", "on"} else "0")
                    ack["action"] = "LOCAL"
                payload = [self._console(request)]
            elif canonical in {"job.status", "job.auto", "job.step"}:
                if canonical == "job.auto" and len(parts) == 3:
                    self.auto = parts[2]
                    ack["action"] = "LOCAL"
                payload = [self._job(request)]
                ack["token"] = end["token"] = payload[0]["token"]
                if canonical == "job.step":
                    ack["action"] = "LOCAL"
                    if payload[0]["state"] == "ACTIVE":
                        payload[0]["code"] = "IN_PROGRESS"
            elif canonical == "profile.show" or canonical.replace("-", "_") in self.staged:
                if canonical != "profile.show" and len(parts) > 1:
                    self.staged[canonical.replace("-", "_")] = parts[1]
                    ack["action"] = "STAGED"
                payload = self._profile(request)
            elif canonical in {"read", "init", "apply", "reset"}:
                ack.update(action="ADMITTED", code="IN_PROGRESS", token=str(self.token))
                self.operation = dict(request=request, nonce=self.nonce, command=canonical,
                                      token=self.token, done=False)
                records = [ack, self._job(request, code="IN_PROGRESS")]
                return self._respond(text, records, data)
            elif canonical == "cancel":
                ack.update(action="LOCAL", token=str(self.token))
                end["token"] = str(self.token)
            elif canonical in {"result", "result.take"}:
                if self.operation is None or not self.operation["done"]:
                    raise AssertionError("test peer received result without retained operation")
                ack["token"] = end["token"] = str(self.operation["token"])
                payload = [dict(value, request=str(request), nonce=self.nonce)
                           for value in self.operation["payload"]]
                if canonical == "result.take":
                    if int(parts[2]) != self.operation["token"]:
                        raise AssertionError("test peer received take for a foreign token")
                    self.operation = None
            records = [ack] + payload + [end]
        return self._respond(text, records, data)

    def _respond(self, text: str, records: list[dict[str, str]], data: bytes) -> int:
        """Append the normal or injected reply after one accepted command write."""
        if self.response_hook is None:
            self.queue(records)
        else:
            reply = self.response_hook(text, records)
            if isinstance(reply, bytes):
                self.pending.extend(reply)
            else:
                self.queue(reply)
        return len(data)

    def complete(self, *, code: str = "OK", detail: int = 0) -> list[dict[str, str]]:
        """Queue one original completion, preserving its older request and nonce."""
        if self.operation is None:
            raise AssertionError("no operation was admitted")
        op = self.operation
        outcome = "SUCCEEDED" if code == "OK" else "FAILED"
        payload = [record("RESULT", op["request"], op["nonce"], token=op["token"],
                          state=outcome, code=code, detail=detail),
                   record("RESULT_DETAIL", op["request"], op["nonce"], token=op["token"])]
        op.update(payload=payload, done=True)
        closure = payload + [record("END", op["request"], op["nonce"], command=op["command"],
                                    token=op["token"], outcome=outcome, code=code, detail=detail)]
        self.queue(closure)
        return closure

    def read(self, size: int, timeout: float) -> bytes:
        """Return a fragment; requested waits advance virtual time when no bytes exist."""
        self.timeouts.append(("read", timeout))
        if self.read_error is not None:
            raise self.read_error
        self.clock.now += self.read_delay
        if not self.pending:
            self.clock.now += timeout
            return b""
        count = min(size, self.fragments.popleft() if self.fragments else size, len(self.pending))
        chunk = bytes(self.pending[:count])
        del self.pending[:count]
        return chunk

    def close(self) -> None:
        """Expose caller ownership without altering the simulated firmware state."""
        self.closed = True
        if self.close_error is not None:
            raise self.close_error


class SessionTests(unittest.TestCase):
    """Drive public session APIs through valid and damaged serial transcripts."""

    def make_session(self, **kwargs):
        """Create isolated real on-disk evidence and a virtual-clock peer."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        clock = FakeClock()
        artifacts = Artifacts(Path(temporary.name), clock)
        self.addCleanup(artifacts.close)
        peer = Peer(clock)
        nonces = iter(f"fresh_{index}" for index in range(1000))
        session = runner.Session(peer, artifacts, clock=clock, nonce_factory=lambda: next(nonces),
                                 **kwargs)
        return session, peer, clock, artifacts

    def begun(self, **kwargs):
        """Complete the actual read-only handshake before a test mutates its wire."""
        result = self.make_session(**kwargs)
        result[0].begin()
        return result

    def test_fragmented_crlf_and_ansi_human_noise(self):
        """Reassemble fragmented CRLF records while ignoring bounded human color output."""
        session, peer, _, artifacts = self.make_session()
        peer.fragments.extend([1, 2, 3, 1, 5] * 600)
        peer.pending.extend(b"\x1b[32mordinary boot text\x1b[0m\r\nPASS\r\n")
        peer.response_hook = lambda _, rows: b"".join(encode_record(row) for row in rows).replace(
            b"\n", b"\r\n")
        identity = session.begin()
        self.assertTrue(identity.known)
        self.assertEqual(identity.git, GIT)
        self.assertTrue(session.valid)
        self.assertEqual(session.command("bus").end["code"], "OK")
        artifacts.checkpoint()
        saved = json.loads((artifacts.directory / "report.json").read_text())
        self.assertIn("capture", saved)
        self.assertFalse(peer.closed)

    def test_begin_captures_firmware_separately_and_only_queries(self):
        """Establish firmware provenance through the read-only handshake routes."""
        session, peer, _, artifacts = self.make_session()
        identity = session.begin(expected={"git": GIT, "target": "native"})
        self.assertEqual(identity.library, "0.1.0")
        self.assertEqual(identity.framework, "host")
        self.assertEqual(identity.framework_version, "test_1")
        self.assertEqual(peer.writes[0].split()[0], "hilmark")
        self.assertTrue({"version", "bus", "job status", "console", "profile show"}
                        .issubset(set(peer.writes)))
        self.assertFalse(any(command.split()[0] in {"apply", "reset", "cancel", "result"}
                             for command in peer.writes))
        artifacts.checkpoint()
        self.assertIn("firmware", json.loads((artifacts.directory / "report.json").read_text())
                      ["metadata"])

    def test_unknown_identity_blocks_expectation_and_device_mutation(self):
        """Allow unknown read-only identity but reject mutation without sending bytes."""
        session, peer, _, _ = self.make_session()
        peer.identity = dict(git="unknown", tree="unknown", build="unknown")
        identity = session.begin()
        self.assertFalse(identity.known)
        before = list(peer.writes)
        with self.assertRaises(runner.SessionBlocked):
            session.command("apply")
        self.assertEqual(peer.writes, before)
        with self.assertRaises(runner.SessionBlocked):
            identity.require({"git": GIT})

    def test_expected_identity_mismatch_blocks_begin(self):
        """Reject mismatched expected firmware and leave the session unverified."""
        session, _, _, _ = self.make_session()
        with self.assertRaises(runner.SessionBlocked):
            session.begin(expected={"git": "f" * 40})
        self.assertFalse(session.valid)

    def test_partial_identity_cannot_satisfy_even_a_matching_expectation(self):
        """A matching library version cannot prove an otherwise unknown firmware build."""
        session, peer, _, _ = self.make_session()
        peer.identity["git"] = "unknown"
        with self.assertRaises(runner.SessionBlocked):
            session.begin(expected={"library": "0.1.0"})
        self.assertFalse(session.valid)

    def test_foreign_active_and_retained_work_are_never_taken_or_cancelled(self):
        """Block foreign prerequisites without consuming or cancelling their operation."""
        for field in ("foreign_active", "foreign_result"):
            with self.subTest(field=field):
                session, peer, _, _ = self.make_session()
                setattr(peer, field, True)
                with self.assertRaises(runner.SessionBlocked):
                    session.begin()
                self.assertFalse(any(text.startswith(("cancel", "result take"))
                                     for text in peer.writes))

    def test_echo_and_human_pass_are_not_marker_evidence(self):
        """Echoed input and human PASS text cannot establish a correlated frame."""
        session, peer, _, _ = self.make_session()
        peer.response_hook = lambda text, _: (text + "\nOK\nPASS\n").encode("ascii")
        with self.assertRaises(runner.SessionTimeout):
            session.begin()
        self.assertFalse(session.valid)

    def test_failed_begin_cannot_reenter_marker_synchronization(self):
        """A failed initial marker closes synchronization; later mark calls cannot retry it."""
        session, peer, _, _ = self.make_session()
        peer.response_hook = lambda _, rows: b"PASS\n"
        with self.assertRaises(runner.ProtocolError):
            session.begin()
        peer.response_hook = None
        before = list(peer.writes)
        with self.assertRaises(runner.SessionBlocked):
            session.mark()
        self.assertEqual(peer.writes, before)

    def test_stale_marker_cannot_establish_session(self):
        """A marker for another nonce cannot establish this session."""
        session, peer, _, _ = self.make_session()
        peer.response_hook = lambda _, rows: [dict(row, nonce="stale") for row in rows]
        with self.assertRaises(runner.ProtocolError):
            session.begin()
        self.assertFalse(session.valid)

    def test_missing_and_duplicate_required_records(self):
        """Reject missing proof and repeated records, including a trailing duplicate END."""
        for missing in ("ACK", "BUS", "END"):
            with self.subTest(missing=missing):
                session, peer, _, _ = self.begun()
                peer.response_hook = lambda _, rows: [row for row in rows if row["type"] != missing]
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus")
                self.assertFalse(session.valid)
        for duplicate in ("ACK", "BUS", "END"):
            with self.subTest(duplicate=duplicate):
                session, peer, _, _ = self.begun()
                def duplicate_row(_, rows):
                    """Repeat one required record while retaining its original wire position."""
                    target = next(row for row in rows if row["type"] == duplicate)
                    position = rows.index(target)
                    return rows[:position] + [target, dict(target)] + rows[position + 1:]
                peer.response_hook = duplicate_row
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus")
                    # A duplicated END can arrive just after the first closure.
                    session.command("console")
                self.assertFalse(session.valid)

    def test_profile_response_requires_each_explicit_source(self):
        """Reject each missing profile source, including unavailable observed/applied values."""
        for source in ("staged", "desired", "observed", "applied"):
            with self.subTest(source=source):
                session, peer, _, _ = self.make_session()
                def missing_source(_, rows):
                    """Remove a whole source record without damaging any remaining schema."""
                    return [row for row in rows if not (
                        row["type"] == "PROFILE" and row["source"] == source)]
                peer.response_hook = missing_source
                with self.assertRaises(runner.ProtocolError):
                    session.begin()
                self.assertFalse(session.valid)

    def test_schema_damage_and_ansi_inside_records_reject(self):
        """Reject malformed machine records rather than treating them as human noise."""
        for damage in (lambda line: line.replace(b"schema=1", b"schema=2", 1),
                       lambda line: line.replace(b"schema=1", b"schema=1 schema=1", 1),
                       lambda line: line.replace(b" command=bus", b"", 1),
                       lambda line: line.replace(b"TUSB ACK", b"TUSB \x1b[31mACK", 1),
                       lambda line: line.replace(b"TUSB ACK", b"TUSB ACK\xff", 1)):
            with self.subTest(damage=damage):
                session, peer, _, _ = self.begun()
                peer.response_hook = lambda _, rows: damage(b"".join(map(encode_record, rows)))
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus")
                self.assertFalse(session.valid)

    def test_request_nonce_and_command_mismatch_reject(self):
        """Invalidate regressed requests and mismatched acknowledgement attribution."""
        for key, value in (("request", "1"), ("nonce", "other"), ("command", "console")):
            with self.subTest(key=key):
                session, peer, _, _ = self.begun()
                peer.response_hook = lambda _, rows: [dict(rows[0], **{key: value})] + rows[1:]
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus")
                self.assertFalse(session.valid)

    def test_invalid_command_receives_typed_failed_closure(self):
        """Preserve a correctly framed invalid-command rejection as a failed exchange."""
        session, peer, _, _ = self.begun()
        def rejected(_, rows):
            """Model the canonical invalid-command failure emitted by the real CLI."""
            return [record("ACK", peer.request, peer.nonce, command="invalid",
                           code="INVALID_PARAM", action="UNCHANGED"),
                    record("END", peer.request, peer.nonce, command="invalid",
                           code="INVALID_PARAM", outcome="FAILED")]
        peer.response_hook = rejected
        result = session.command("not-a-command")
        self.assertEqual(result.ack["command"], "invalid")
        self.assertEqual(result.end["code"], "INVALID_PARAM")

    def test_invalid_arguments_keep_verified_session_and_prior_local_changes(self):
        """Accept silent parser rejection of a known route without guessing its syntax."""
        for text in ("mode drp extra", "bus extra", "profile.show"):
            with self.subTest(text=text):
                session, peer, _, artifacts = self.begun()
                session.command("color on")
                before = dict(peer.staged)
                def rejected(command, rows):
                    """Reject the whole input before the fixture can change its staging."""
                    if command == text:
                        peer.staged = dict(before)
                        return [record("ACK", peer.request, peer.nonce, command="invalid",
                                       code="INVALID_PARAM", action="UNCHANGED"),
                                record("END", peer.request, peer.nonce, command="invalid",
                                       code="INVALID_PARAM", outcome="FAILED")]
                    return rows
                peer.response_hook = rejected
                response = session.command(text)
                self.assertEqual(response.command, "invalid")
                self.assertEqual(response.end["code"], "INVALID_PARAM")
                self.assertTrue(session.valid)
                self.assertEqual(artifacts.metadata["local_changed"], ["color"])
                session.finish()
                self.assertEqual(peer.color, "0")
                self.assertEqual(peer.staged, before)

    def test_invalid_fallback_cannot_hide_success_traffic_or_payload(self):
        """Only exact failed ACK/END parser closures can change the predicted canonical route."""
        for damage in ("success", "detail", "token", "action", "traffic", "payload"):
            with self.subTest(damage=damage):
                session, peer, _, _ = self.begun()
                def rejected(_, rows):
                    """Inject a contradictory rejection independently of session validation."""
                    ack = record("ACK", peer.request, peer.nonce, command="invalid",
                                 code="INVALID_PARAM", action="UNCHANGED")
                    end = record("END", peer.request, peer.nonce, command="invalid",
                                 code="INVALID_PARAM", outcome="FAILED")
                    if damage == "success":
                        ack["code"] = end["code"] = "OK"
                        end["outcome"] = "SUCCEEDED"
                    elif damage == "detail":
                        ack["detail"] = end["detail"] = "1"
                    elif damage == "token":
                        ack["token"] = end["token"] = "7"
                    elif damage == "action":
                        ack["action"] = "QUERY"
                    elif damage == "traffic":
                        end["callbacks"] = "1"
                    return [ack, *([rows[1]] if damage == "payload" else []), end]
                peer.response_hook = rejected
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus extra")
                self.assertFalse(session.valid)

    def test_extension_fields_are_preserved_without_substituting_required_fields(self):
        """Retain additive fields while still requiring the complete known schema."""
        session, peer, _, _ = self.begun()
        peer.response_hook = lambda _, rows: [dict(row, future_field="extension") for row in rows]
        result = session.command("bus")
        self.assertEqual(result.ack["future_field"], "extension")
        self.assertEqual(next(row for row in result.records if row["type"] == "BUS")
                         ["future_field"], "extension")

    def test_unknown_source_extension_cannot_hide_duplicate_singleton_bus(self):
        """An additive source field cannot turn BUS into a repeated-record schema."""
        session, peer, _, _ = self.begun()
        def duplicate_bus(_, rows):
            """Add a second singleton BUS with an otherwise legal extension field."""
            bus = next(row for row in rows if row["type"] == "BUS")
            return rows[:-1] + [dict(bus, source="unrecognized_extension")] + rows[-1:]
        peer.response_hook = duplicate_bus
        with self.assertRaises(runner.ProtocolError):
            session.command("bus")
        self.assertFalse(session.valid)

    def test_output_loss_exhaustion_and_reboot_invalidate(self):
        """Loss, request exhaustion and reboot prevent subsequent writes in that session."""
        for payload in (b"TUSB LOSS schema=1 dropped_records=1 dropped_bytes=10\n",
                        b"TUSB SESSION schema=1 code=TOKEN_EXHAUSTED detail=0 "
                        b"reason=request_exhausted\n",
                        b"ESP-ROM:esp32s3-20210327\n", b"rst:0x1 (POWERON),boot:0x8\n"):
            with self.subTest(payload=payload):
                session, peer, _, _ = self.begun()
                peer.response_hook = lambda _, rows: payload + b"".join(map(encode_record, rows))
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus")
                self.assertFalse(session.valid)
                count = len(peer.writes)
                with self.assertRaises(runner.SessionError):
                    session.command("bus")
                self.assertEqual(len(peer.writes), count)

    def test_changed_firmware_identity_invalidates(self):
        """A repeated identity query must match the established firmware build."""
        session, peer, _, _ = self.begun()
        peer.identity["build"] = "other_build"
        with self.assertRaises(runner.ProtocolError):
            session.command("version")
        self.assertFalse(session.valid)

    def test_read_and_write_share_one_absolute_deadline(self):
        """A slow write reduces the remaining read budget and late replies fail."""
        session, peer, clock, _ = self.begun(timeouts=runner.Timeouts(command=1.0, write=0.7))
        peer.write_delay = 0.6
        peer.read_delay = 0.5
        start = clock()
        peer.timeouts.clear()
        with self.assertRaises(runner.SessionTimeout):
            session.command("bus")
        self.assertAlmostEqual(peer.timeouts[0][1], 0.7)
        self.assertTrue(all(timeout <= 0.4 + 1e-9 for kind, timeout in peer.timeouts
                            if kind == "read"))
        self.assertGreater(clock() - start, 1.0)
        self.assertFalse(session.valid)

    def test_explicit_begin_deadline_is_shared_by_all_handshake_commands(self):
        """The handshake cannot renew its caller-supplied deadline between commands."""
        session, peer, clock, _ = self.make_session()
        peer.write_delay = 0.3
        peer.read_delay = 0.1
        deadline = clock() + 0.9
        with self.assertRaises(runner.SessionTimeout):
            session.begin(deadline=deadline)
        self.assertLessEqual(len(peer.writes), 3)
        self.assertFalse(session.valid)

    def test_slow_fragments_do_not_restart_command_timeout(self):
        """Repeated fragments consume one command timeout rather than renewing it."""
        session, peer, clock, _ = self.begun(timeouts=runner.Timeouts(command=1.0))
        peer.fragments.extend([1] * 100)
        peer.read_delay = 0.2
        start = clock()
        with self.assertRaises(runner.SessionTimeout):
            session.command("bus")
        self.assertLessEqual(clock() - start, 1.21)
        self.assertFalse(session.valid)

    def test_expired_deadline_does_not_write(self):
        """An already expired caller deadline produces no serial command write."""
        session, peer, clock, _ = self.begun()
        before = list(peer.writes)
        with self.assertRaises(runner.SessionTimeout):
            session.command("bus", deadline=clock())
        self.assertEqual(peer.writes, before)

    def test_expired_public_deadlines_save_owned_evidence_before_any_io(self):
        """Expiration before the I/O loop still invalidates and checkpoints the owned token."""
        for method in ("command", "mark", "wait_operation"):
            with self.subTest(method=method):
                session, peer, clock, artifacts = self.begun()
                session.command("read", deadline=clock() + 1)
                clock.now += 2
                before = list(peer.writes)
                with self.assertRaises(runner.SessionTimeout):
                    if method == "command":
                        session.command("bus", deadline=clock())
                    else:
                        getattr(session, method)(deadline=clock())
                self.assertEqual(before, peer.writes)
                self.assertFalse(session.valid)
                saved = json.loads((artifacts.directory / "report.json").read_text())
                self.assertEqual(saved["exit_code"], 1)
                self.assertEqual(saved["checks"]["session"]["status"], "FAIL")
                self.assertEqual(saved["metadata"]["unconsumed_operation"]["token"], peer.token)

    def test_invalid_deadline_is_saved_as_invocation_incomplete(self):
        """An invalid finite-deadline argument stays exit two rather than a protocol failure."""
        session, peer, _, artifacts = self.begun()
        before = list(peer.writes)
        with self.assertRaises(runner.InvocationError):
            session.command("bus", deadline=float("nan"))
        self.assertEqual(peer.writes, before)
        self.assertEqual(artifacts.exit_code(), 2)
        self.assertFalse(session.valid)

    def test_late_write_and_short_write_invalidate(self):
        """Reject late or partial writes because complete command acceptance is unproved."""
        for short in (False, True):
            with self.subTest(short=short):
                session, peer, _, _ = self.begun(timeouts=runner.Timeouts(command=1.0, write=0.2))
                peer.short_write = short
                peer.write_delay = 0.0 if short else 0.3
                with self.assertRaises(runner.ProtocolError):
                    session.command("bus")
                self.assertFalse(session.valid)

    def test_truncated_line_never_becomes_success(self):
        """An incomplete record times out instead of satisfying its response schema."""
        session, peer, _, _ = self.begun()
        peer.response_hook = lambda _, rows: encode_record(rows[0])[:-1]
        with self.assertRaises(runner.SessionTimeout):
            session.command("bus")
        self.assertFalse(session.valid)

    def test_transport_exception_preserves_failure_and_invalidates(self):
        """Preserve backend exception detail while invalidating further session work."""
        for direction in ("read", "write"):
            with self.subTest(direction=direction):
                session, peer, _, _ = self.begun()
                setattr(peer, direction + "_error", OSError("cable removed"))
                with self.assertRaisesRegex(OSError, "cable removed"):
                    session.command("bus")
                self.assertFalse(session.valid)

    def test_response_line_noise_record_and_io_caps_fail_closed(self):
        """Each configured response bound independently prevents unbounded evidence."""
        variants = (
            (runner.Limits(response_bytes=64), b""),
            (runner.Limits(response_records=1), b""),
            (runner.Limits(noise_bytes=5), b"human noise\n"),
            (runner.Limits(line_bytes=64), b"x" * 65),
            (runner.Limits(io_reads=2), b""),
        )
        for limits, prefix in variants:
            with self.subTest(limits=limits):
                session, peer, _, _ = self.make_session(limits=limits)
                peer.pending.extend(prefix)
                if limits.io_reads == 2:
                    peer.fragments.extend([1] * 20)
                with self.assertRaises(runner.ProtocolError):
                    session.begin()
                self.assertFalse(session.valid)

    def test_admission_and_failed_payload_remain_distinct_from_delivery_ok(self):
        """Successful result delivery preserves the failed operation and signed detail."""
        session, peer, _, _ = self.begun()
        admitted = session.command("read")
        self.assertEqual(admitted.ack["code"], "IN_PROGRESS")
        self.assertIsNone(admitted.end)
        self.assertTrue(admitted.admitted)
        self.assertIsNotNone(session.operation)
        peer.complete(code="I2C_TIMEOUT", detail=-2147483648)
        outcome = session.wait_operation()
        self.assertEqual(outcome["code"], "I2C_TIMEOUT")
        self.assertEqual(outcome["detail"], "-2147483648")
        taken = session.take_result()
        self.assertEqual(taken.end["code"], "OK")
        self.assertEqual(next(row for row in taken.records if row["type"] == "RESULT")
                         ["state"], "FAILED")
        self.assertIsNone(session.operation)
        self.assertEqual(peer.writes[-1], "result take 7")

    def test_owned_host_deadline_cannot_be_renewed_by_wait_or_query(self):
        """Later calls clip their requested deadlines to the original admission limit."""
        for action in ("wait", "query"):
            with self.subTest(action=action):
                session, peer, clock, _ = self.begun(
                    timeouts=runner.Timeouts(command=1.0, write=1.0))
                admitted_deadline = clock() + 1.0
                session.command("read", deadline=admitted_deadline)
                clock.now += 0.75
                peer.read_delay = 0.3
                peer.timeouts.clear()
                if action == "wait":
                    peer.complete()
                with self.assertRaises(runner.SessionTimeout):
                    if action == "wait":
                        session.wait_operation(deadline=clock() + 3.0)
                    else:
                        session.command("job status", deadline=clock() + 3.0)
                self.assertTrue(peer.timeouts)
                self.assertTrue(all(timeout <= 0.25 + 1e-9 for _, timeout in peer.timeouts))
                self.assertFalse(session.valid)

    def test_failed_payload_stays_failed_after_delivery_and_good_cleanup(self):
        """Delivery OK, primary PASS and cleanup PASS cannot erase failed device proof."""
        session, peer, _, artifacts = self.begun()
        session.command("read")
        peer.complete(code="I2C_TIMEOUT", detail=-7)
        session.wait_operation()
        delivered = session.take_result()
        self.assertEqual(delivered.end["code"], "OK")
        session.command("color on")
        session.finish()
        artifacts.primary = "PASS"
        self.assertEqual(artifacts.cleanup, "PASS")
        artifacts.checkpoint()
        saved = json.loads((artifacts.directory / "report.json").read_text())
        self.assertEqual(saved["checks"]["operation_payloads"]["status"], "FAIL")
        self.assertEqual(saved["exit_code"], 1)
        self.assertEqual(artifacts.exit_code(), 1)

    def test_older_operation_can_finish_after_query_and_fresh_marker(self):
        """Keep the original operation nonce after newer query and marker requests."""
        session, peer, _, _ = self.begun()
        original = session.command("read").ack
        session.command("job status")
        new_nonce = session.mark()
        self.assertNotEqual(original["nonce"], new_nonce)
        peer.complete()
        result = session.wait_operation()
        self.assertEqual(result["origin_request"], original["request"])
        self.assertEqual(result["nonce"], original["nonce"])
        session.take_result()

    def test_original_response_caps_persist_across_later_queries(self):
        """A newer query cannot restart the older operation's byte or record allowance."""
        for limits in (runner.Limits(response_records=12), runner.Limits(response_bytes=4096)):
            with self.subTest(limits=limits):
                session, peer, _, _ = self.begun(limits=limits)
                original = session.command("read").ack
                with self.assertRaises(runner.ProtocolError):
                    for _ in range(40):
                        peer.queue([record("JOB", int(original["request"]), original["nonce"],
                                           token=original["token"], kind="REFRESH_PORT",
                                           state="ACTIVE", phase="READING", code="IN_PROGRESS")])
                        session.command("bus")
                self.assertFalse(session.valid)

    def test_interleaved_original_completion_during_query_is_kept(self):
        """Route an older terminal frame without confusing it with a newer query."""
        session, peer, _, _ = self.begun()
        original = session.command("read").ack
        def complete_during_query(_, rows):
            """Queue an older owned completion before the newer query response."""
            peer.complete()
            return rows
        peer.response_hook = complete_during_query
        exchange = session.command("bus")
        peer.response_hook = None
        self.assertEqual(exchange.end["command"], "bus")
        self.assertEqual(session.wait_operation()["origin_request"], original["request"])
        session.take_result()

    def test_wrong_operation_token_origin_and_duplicate_completion_reject(self):
        """Reject altered operation attribution and repeated original completions."""
        for key, value in (("token", "8"), ("origin_request", "1"), ("nonce", "wrong")):
            with self.subTest(key=key):
                session, peer, _, _ = self.begun()
                session.command("read")
                completion = peer.complete()
                peer.pending.clear()
                peer.queue([dict(completion[0], **{key: value})] + completion[1:])
                with self.assertRaises(runner.ProtocolError):
                    session.wait_operation()
                self.assertFalse(session.valid)
        session, peer, _, _ = self.begun()
        session.command("read")
        completion = peer.complete()
        peer.pending.clear()
        peer.queue([completion[0], dict(completion[0])] + completion[1:])
        with self.assertRaises(runner.ProtocolError):
            session.wait_operation()

    def test_operation_requires_one_detail_and_consistent_terminal_closure(self):
        """Require complete terminal detail and matching operation END evidence."""
        for damage in ("missing_detail", "wrong_end_token", "wrong_end_detail", "wrong_end_code"):
            with self.subTest(damage=damage):
                session, peer, _, _ = self.begun()
                session.command("read")
                completion = peer.complete()
                peer.pending.clear()
                if damage == "missing_detail":
                    completion = [row for row in completion if row["type"] != "RESULT_DETAIL"]
                else:
                    changes = {"wrong_end_token": {"token": "8"},
                               "wrong_end_detail": {"detail": "-7"},
                               "wrong_end_code": {"code": "I2C_ERROR", "outcome": "FAILED"}}
                    completion[-1] = dict(completion[-1], **changes[damage])
                peer.queue(completion)
                with self.assertRaises(runner.ProtocolError):
                    session.wait_operation()
                self.assertFalse(session.valid)

    def test_original_operation_requires_its_admission_job_record(self):
        """A result and END cannot substitute for missing required original JOB evidence."""
        session, peer, _, artifacts = self.begun()
        peer.response_hook = lambda _, rows: [r for r in rows if r["type"] != "JOB"]
        session.command("read")
        peer.complete()
        with self.assertRaisesRegex(runner.ProtocolError, "missing required original"):
            session.wait_operation()
        self.assertFalse(session.valid)
        self.assertEqual(artifacts.exit_code(), 1)

    def test_orphan_terminal_record_and_zero_admission_token_reject(self):
        """Reject terminal records without ownership and admission without a token."""
        session, peer, _, _ = self.begun()
        peer.pending.extend(encode_record(record("RESULT", peer.request, peer.nonce)))
        with self.assertRaises(runner.ProtocolError):
            session.command("bus")
        session, peer, _, _ = self.begun()
        peer.response_hook = lambda _, rows: [dict(rows[0], token="0")] + rows[1:]
        with self.assertRaises(runner.ProtocolError):
            session.command("read")

    def test_result_query_repetition_is_not_duplicate_original_completion(self):
        """Repeated retained-result queries remain copies of one original completion."""
        session, peer, _, _ = self.begun()
        session.command("read")
        peer.complete()
        original = session.wait_operation()
        for _ in range(2):
            query = session.command("result 7")
            result = next(row for row in query.records if row["type"] == "RESULT")
            self.assertEqual(result["origin_request"], original["origin_request"])
        session.take_result()

    def test_result_delivery_must_match_original_payload(self):
        """A copied result must retain every original terminal payload field."""
        session, peer, _, _ = self.begun()
        session.command("read")
        peer.complete(code="I2C_ERROR", detail=-7)
        session.wait_operation()
        def change_result(_, rows):
            """Corrupt only the copied payload while leaving delivery status successful."""
            return [dict(row, detail="-8") if row["type"] == "RESULT" else row for row in rows]
        peer.response_hook = change_result
        with self.assertRaises(runner.ProtocolError):
            session.take_result()
        self.assertFalse(session.valid)

    def test_job_queries_preserve_owned_token_in_ack_payload_and_end(self):
        """Require represented ownership across query ACK, JOB and END records."""
        for text in ("job status", "job auto 0", "job step 0"):
            for damaged_type in ("ACK", "JOB", "END"):
                with self.subTest(text=text, damaged_type=damaged_type):
                    session, peer, _, _ = self.begun()
                    session.command("read")
                    def damage(_, rows):
                        """Alter one represented token within the same query frame."""
                        return [dict(row, token="8") if row["type"] == damaged_type else row
                                for row in rows]
                    peer.response_hook = damage
                    with self.assertRaises(runner.SessionError):
                        session.command(text)
                    self.assertFalse(session.valid)
            with self.subTest(text=text, damaged_type="ALL_ZERO"):
                session, peer, _, _ = self.begun()
                session.command("read")
                peer.response_hook = lambda _, rows: [dict(row, token="0") for row in rows]
                with self.assertRaises(runner.ProtocolError):
                    session.command(text)
                self.assertFalse(session.valid)

    def test_cancel_response_preserves_explicitly_owned_token(self):
        """Self-consistent cancel replies must still identify the token owned by this run."""
        for damaged_type in ("ACK", "END", "BOTH"):
            with self.subTest(damaged_type=damaged_type):
                session, peer, _, _ = self.begun()
                session.command("read")
                peer.response_hook = lambda _, rows: [
                    dict(row, token="8") if row["type"] == damaged_type or damaged_type == "BOTH"
                    else row for row in rows]
                with self.assertRaises(runner.ProtocolError):
                    session.command("cancel 7")
                self.assertFalse(session.valid)

    def test_tokens_are_never_reused_or_implicitly_taken(self):
        """Reject unowned takes and reuse of an already completed operation token."""
        session, peer, _, _ = self.begun()
        before = list(peer.writes)
        with self.assertRaises(runner.SessionBlocked):
            session.take_result()
        self.assertEqual(peer.writes, before)
        session.command("read")
        peer.complete()
        session.wait_operation()
        session.take_result()
        # Firmware is required to allocate instance-local tokens monotonically.
        with self.assertRaises(runner.ProtocolError):
            session.command("read")

    def test_implicit_foreign_token_control_is_blocked_without_tx(self):
        """Token controls cannot operate without a matching admission from this run."""
        session, peer, _, _ = self.begun()
        for command in ("cancel", "cancel 19", "result take 19", "job step"):
            with self.subTest(command=command):
                before = list(peer.writes)
                with self.assertRaises(runner.SessionBlocked):
                    session.command(command)
                self.assertEqual(peer.writes, before)

    def test_mutation_checkpoints_before_command_is_written(self):
        """Save local mutation intent before the serial backend receives the command."""
        session, peer, _, artifacts = self.begun()
        events = []
        original_checkpoint = artifacts.checkpoint
        def checkpoint():
            """Record checkpoint order while saving the actual on-disk report."""
            events.append("checkpoint")
            original_checkpoint()
        def respond(text, rows):
            """Observe the mutation only after its intent checkpoint has completed."""
            if text == "mode dfp":
                events.append("mutation")
                self.assertIn("checkpoint", events)
            return rows
        peer.response_hook = respond
        with mock.patch.object(artifacts, "checkpoint", side_effect=checkpoint):
            session.command("mode dfp")
        self.assertLess(events.index("checkpoint"), events.index("mutation"))

    def test_restore_after_session_loss_cannot_mutate_unverified_device(self):
        """Session loss prevents replaying even previously saved local settings."""
        session, peer, _, _ = self.begun()
        session.command("mode dfp")
        peer.response_hook = lambda _, rows: b"ESP-ROM:esp32s3-20210327\n"
        with self.assertRaises(runner.ProtocolError):
            session.command("bus")
        before = list(peer.writes)
        with self.assertRaises(runner.SessionError):
            session.restore_local()
        self.assertEqual(peer.writes, before)

    def test_missing_staging_proof_blocks_edits_without_traffic(self):
        """Unavailable, partial or misattributed baseline cannot support later restoration."""
        for fields in ({"available": "0"}, {"valid_fields": "0"}, {"valid_fields": "254"},
                       {"mode": "unknown"}, {"variant": "tusb320"},
                       {"disable_term": "1"}):
            for command in ("mode dfp", "profile reset", "profile discard"):
                with self.subTest(fields=fields, command=command):
                    session, peer, _, artifacts = self.make_session()
                    def missing(_, rows):
                        """Damage staged validity while leaving every required key present."""
                        return [dict(row, **fields) if row["type"] == "PROFILE" and
                                row["source"] == "staged" else row for row in rows]
                    peer.response_hook = missing
                    session.begin()
                    before = list(peer.writes)
                    with self.assertRaises(runner.SessionBlocked):
                        session.command(command)
                    self.assertEqual(peer.writes, before)
                    self.assertNotEqual(artifacts.cleanup, "PASS")

    def test_coherently_absent_field_cannot_escape_restoration(self):
        """A setter needs its own saved field even when all other capability bits are valid."""
        session, peer, _, _ = self.make_session()
        def absent_mode(_, rows):
            """Advertise a missing mode field consistently in capabilities and staging."""
            return [dict(row, profile_fields="254") if row["type"] == "CAPABILITIES" else
                    dict(row, valid_fields="254", mode="unknown") if row["type"] == "PROFILE"
                    else row for row in rows]
        peer.response_hook = absent_mode
        session.begin()
        before = list(peer.writes)
        with self.assertRaisesRegex(runner.SessionBlocked, "staged field unavailable"):
            session.command("mode dfp")
        self.assertEqual(peer.writes, before)
        self.assertEqual(peer.staged["mode"], "drp")

    def test_restoration_requires_available_valid_attributed_payload(self):
        """Matching field text alone is insufficient proof of restored local staging."""
        for fields in ({"available": "0"}, {"valid_fields": "0"}, {"variant": "unknown"}):
            with self.subTest(fields=fields):
                session, peer, _, artifacts = self.begun()
                session.command("mode dfp")
                def missing(text, rows):
                    """Remove restored staging proof despite its matching requested mode."""
                    return [dict(row, **fields) if text == "mode drp" and
                            row["type"] == "PROFILE" and row["source"] == "staged"
                            else row for row in rows]
                peer.response_hook = missing
                with self.assertRaises(runner.SessionBlocked):
                    session.restore_local()
                self.assertEqual(artifacts.cleanup, "FAIL")
                self.assertIn("profile", artifacts.metadata["unrestored_local"])

    def test_reusable_session_cannot_pass_without_finish(self):
        """The public API declares end-marker proof even outside the command-line entrypoint."""
        session, _, _, artifacts = self.begun()
        artifacts.primary = "PASS"
        artifacts.checkpoint()
        self.assertEqual(artifacts.exit_code(), 2)
        self.assertEqual(artifacts.snapshot()["checks"]["end_marker"]["status"], "NOT RUN")
        session.finish()
        self.assertEqual(artifacts.exit_code(), 0)

    def test_restoration_delivery_ok_requires_matching_local_payload(self):
        """An OK restoration acknowledgement still requires matching staging evidence."""
        session, peer, _, artifacts = self.begun()
        session.command("mode dfp")
        def false_restore(text, rows):
            """Claim successful delivery while leaving staging different from its baseline."""
            if text == "mode drp":
                peer.staged["mode"] = "dfp"
                return [dict(row, mode="dfp") if row["type"] == "PROFILE" and
                        row["source"] == "staged" else row for row in rows]
            return rows
        peer.response_hook = false_restore
        with self.assertRaises(runner.SessionError):
            session.restore_local()
        self.assertEqual(artifacts.cleanup, "FAIL")
        self.assertIn("profile", artifacts.metadata["unrestored_local"])
        self.assertFalse(session.valid)
        before = list(peer.writes)
        peer.response_hook = None
        with self.assertRaises(runner.SessionBlocked):
            session.restore_local()
        with self.assertRaises(runner.SessionBlocked):
            session.finish()
        self.assertEqual(peer.writes, before)
        artifacts.primary = "PASS"
        self.assertEqual(artifacts.exit_code(), 1)

    def test_restore_local_settings_without_hardware_apply(self):
        """Restore staging, scheduling and output settings without applying device changes."""
        session, peer, _, artifacts = self.begun()
        before = len(peer.writes)
        session.command("mode dfp")
        self.assertTrue((artifacts.directory / "report.json").exists())
        session.command("current 3a0")
        session.command("color on")
        session.command("verbose 1")
        session.command("job auto 0")
        session.finish()
        self.assertEqual(peer.staged["mode"], "drp")
        self.assertEqual(peer.staged["current"], "default")
        self.assertEqual((peer.color, peer.verbose, peer.auto), ("0", "0", "1"))
        self.assertFalse(any(text.split()[0] in {"apply", "reset", "wreg"}
                             for text in peer.writes[before:]))
        self.assertTrue(peer.writes[-1].startswith("hilmark "))

    def test_public_finish_checkpoint_interrupt_is_saved(self):
        """A signal after final MARK closure cannot leave the reusable API at PASS."""
        session, _, _, artifacts = self.begun()
        original = artifacts.checkpoint
        def interrupt_final():
            """Interrupt once when the final check first becomes PASS."""
            if (artifacts.snapshot()["checks"]["end_marker"]["status"] == "PASS" and
                    not artifacts.interrupted):
                raise KeyboardInterrupt()
            original()
        with mock.patch.object(artifacts, "checkpoint", side_effect=interrupt_final):
            with self.assertRaises(KeyboardInterrupt):
                session.finish()
        self.assertFalse(session.valid)
        saved = json.loads((artifacts.directory / "report.json").read_text())
        self.assertTrue(saved["interrupted"])
        self.assertEqual(saved["exit_code"], 130)

    def test_post_delivery_checkpoint_interrupt_preserves_taken_result_fact(self):
        """A late save interruption records successful take without inventing a retained token."""
        session, peer, _, artifacts = self.begun()
        session.command("read")
        peer.complete()
        session.wait_operation()
        original = artifacts.checkpoint
        def interrupt_take():
            """Interrupt the post-take save only, leaving the interruption save available."""
            if artifacts.metadata["operation"]["taken"] and not artifacts.interrupted:
                raise KeyboardInterrupt()
            original()
        with mock.patch.object(artifacts, "checkpoint", side_effect=interrupt_take):
            with self.assertRaises(KeyboardInterrupt):
                session.take_result()
        saved = json.loads((artifacts.directory / "report.json").read_text())
        self.assertFalse(session.valid)
        self.assertIsNone(peer.operation)
        self.assertTrue(saved["metadata"]["operation"]["taken"])
        self.assertNotIn("unconsumed_operation", saved["metadata"])
        self.assertEqual(saved["exit_code"], 130)

    def test_finish_requires_owned_retained_result_to_be_consumed(self):
        """Finishing cannot silently abandon or consume an owned retained result."""
        session, peer, _, _ = self.begun()
        session.command("read")
        peer.complete()
        session.wait_operation()
        with self.assertRaises(runner.SessionBlocked):
            session.finish()
        self.assertFalse(any(text.startswith("result take") for text in peer.writes))

    def test_finish_rejects_duplicate_end_and_truncated_buffer(self):
        """Buffered framing damage after the final marker prevents a passing report."""
        for damage in ("duplicate_end", "truncated"):
            with self.subTest(damage=damage):
                session, peer, _, artifacts = self.begun()
                def damaged_marker(_, rows):
                    """Append a duplicate closure or incomplete record to the final marker frame."""
                    encoded = b"".join(map(encode_record, rows))
                    if damage == "duplicate_end":
                        return encoded + encode_record(rows[-1])
                    return encoded + b"TUSB ACK schema=1 nonce=truncated"
                peer.response_hook = damaged_marker
                with self.assertRaises(runner.ProtocolError):
                    session.finish()
                self.assertFalse(session.valid)
                artifacts.checkpoint()
                saved = json.loads((artifacts.directory / "report.json").read_text())
                self.assertNotEqual(saved["exit_code"], 0)
                self.assertNotEqual(saved["checks"].get("end_marker", {}).get("status"), "PASS")


class ConfigurationTests(unittest.TestCase):
    """Reject invalid bounds and opening options before a serial backend is touched."""

    def test_timeout_validation(self):
        """Reject nonfinite, nonpositive and incorrectly typed timeout values."""
        for value in (0, -1, float("nan"), float("inf"), 3600.1, True, "1"):
            for name in ("command", "write"):
                with self.subTest(name=name, value=value):
                    with self.assertRaises(runner.InvocationError):
                        runner.Timeouts(**{name: value})

    def test_limit_validation(self):
        """Reject invalid response limits before any backend interaction."""
        for name in ("response_bytes", "response_records", "noise_bytes", "line_bytes", "io_reads"):
            for value in (0, -1, float("inf"), True, 1 << 50):
                with self.subTest(name=name, value=value):
                    with self.assertRaises(runner.InvocationError):
                        runner.Limits(**{name: value})

    def test_identity_known_requires_each_firmware_field(self):
        """Known identity requires every provenance field and a valid full Git revision."""
        values = dict(library="0.1.0", git=GIT, tree="clean", build="build", framework="host",
                      framework_version="1", target="native")
        self.assertTrue(runner.FirmwareIdentity(**values).known)
        for name in values:
            with self.subTest(name=name):
                self.assertFalse(runner.FirmwareIdentity(**dict(values, **{name: "unknown"})).known)
        for git in ("a" * 39, "z" * 40):
            self.assertFalse(runner.FirmwareIdentity(**dict(values, git=git)).known)
        self.assertFalse(runner.FirmwareIdentity(**dict(values, tree="other")).known)

    def test_open_requires_explicit_port_and_valid_control_lines(self):
        """Validate port and modem-line choices before constructing the serial backend."""
        backend = mock.Mock()
        for port, kwargs in ((None, {}), ("", {}), ("COM_TEST", {"dtr": "toggle"}),
                             ("COM_TEST", {"rts": "auto"}), ("COM_TEST", {"baud": 0})):
            with self.subTest(port=port, kwargs=kwargs):
                with self.assertRaises(runner.InvocationError):
                    runner.open_serial(port, serial_factory=backend, **kwargs)
        backend.assert_not_called()

    def test_dtr_rts_are_configured_before_open_and_reported(self):
        """Apply requested modem lines before opening and preserve their reported settings."""
        class Backend:
            """Record control-line state at the moment the closed fake port opens."""
            def __init__(self):
                """Initialize a closed port with known default modem-line values."""
                self.events = []
                self.is_open = False
                self.dtr = True
                self.rts = False

            def open(self):
                """Record preconfigured port and modem lines before marking it open."""
                self.events.append((self.port, self.dtr, self.rts))
                self.is_open = True

            def close(self):
                """Mark this fake port closed without a hardware action."""
                self.is_open = False

        backend = Backend()
        transport = runner.open_serial("COM_TEST", dtr="off", rts="on",
                                       serial_factory=lambda: backend)
        self.addCleanup(transport.close)
        self.assertEqual(backend.events, [("COM_TEST", False, True)])
        self.assertIsInstance(transport.settings, dict)
        self.assertIn("off", json.dumps(transport.settings))
        self.assertIn("on", json.dumps(transport.settings))


class MainTests(unittest.TestCase):
    """Run the actual CLI entrypoint with borrowed fake serial and real saved files."""

    def invoke(self, *, fault=None, extra=(), opening_error=None):
        """Return exit/peer/report after a hardware-free command-line invocation."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        directory = Path(temporary.name) / "evidence"
        clock = FakeClock()
        peer = Peer(clock)
        peer.settings = {"port": "COM_TEST", "dtr": "unchanged", "rts": "unchanged"}
        if fault == "interrupt":
            peer.write_error = KeyboardInterrupt()
        elif fault == "backend-exit":
            peer.write_error = SystemExit(0)
        elif fault == "disconnect":
            peer.read_error = OSError("device disconnected")
        elif fault == "close":
            peer.close_error = OSError("serial close failed")
        elif fault == "loss":
            peer.response_hook = lambda _, rows: (
                b"TUSB LOSS schema=1 dropped_records=1 dropped_bytes=1\n")
        elif fault == "timeout":
            peer.response_hook = lambda _, rows: b""
        original_session = runner.Session
        def make_session(transport, artifacts, **kwargs):
            """Inject the same virtual clock into the actual CLI-created Session."""
            return original_session(transport, artifacts, clock=clock, **kwargs)
        with ExitStack() as stack:
            stack.enter_context(redirect_stdout(io.StringIO()))
            stack.enter_context(redirect_stderr(io.StringIO()))
            stack.enter_context(mock.patch.object(runner, "host_identity", return_value={
                "git": "f" * 40, "tree": "dirty"}))
            stack.enter_context(mock.patch.object(runner.time, "monotonic", side_effect=clock))
            stack.enter_context(mock.patch.object(runner, "Session", side_effect=make_session))
            opened = stack.enter_context(mock.patch.object(
                runner, "open_serial", return_value=peer, side_effect=opening_error))
            code = runner.main(["--port", "COM_TEST", "--output", str(directory), *extra])
        report_path = directory / "report.json"
        report = json.loads(report_path.read_text()) if report_path.exists() else None
        return code, peer, report, directory, opened

    def test_main_saves_read_only_session_and_separate_host_identity(self):
        """Save a successful read-only session with distinct host and firmware revisions."""
        code, peer, report, directory, opened = self.invoke()
        self.assertEqual(code, 0)
        self.assertEqual(report["primary"], "PASS")
        self.assertEqual(report["metadata"]["host"]["git"], "f" * 40)
        self.assertEqual(report["metadata"]["firmware"]["git"], GIT)
        self.assertTrue((directory / "transcript.log").is_file())
        self.assertTrue((directory / "summary.md").is_file())
        self.assertTrue(peer.closed)
        opened.assert_called_once()
        self.assertFalse(any(text.split()[0] in {"apply", "reset", "wreg", "cancel"}
                             for text in peer.writes))

    def test_main_interruption_saves_exit_130_and_closes_port(self):
        """Save interruption state and close the borrowed port before returning exit 130."""
        code, peer, report, directory, _ = self.invoke(fault="interrupt")
        self.assertEqual(code, 130)
        self.assertTrue(report["interrupted"])
        self.assertEqual(report["exit_code"], 130)
        self.assertNotEqual(report["primary"], "PASS")
        self.assertTrue(peer.closed)
        self.assertTrue((directory / "summary.md").exists())

    def test_final_summary_save_failure_corrects_json_verdict(self):
        """The real entrypoint cannot publish PASS JSON while returning incomplete saving."""
        original = Artifacts._replace
        def fail_final_summary(artifacts, name, data):
            """Fail only the final summary after the final PASS JSON was replaced."""
            if artifacts.primary == "PASS" and name == "summary.md":
                raise OSError("final summary locked")
            return original(artifacts, name, data)
        with mock.patch.object(Artifacts, "_replace", new=fail_final_summary):
            code, peer, report, directory, _ = self.invoke()
        self.assertEqual(code, 2)
        self.assertEqual(report["exit_code"], 2)
        self.assertIn("final summary locked", report["save_error"])
        self.assertTrue(peer.closed)
        self.assertEqual(list(directory.glob("*.tmp")), [])

    def test_final_checkpoint_interrupt_is_saved_with_exit_130(self):
        """Ctrl-C during either final atomic replacement gets one bounded interruption save."""
        original = Artifacts._replace
        for interrupted_file in ("report.json", "summary.md"):
            with self.subTest(interrupted_file=interrupted_file):
                events = []
                def interrupt_final(artifacts, name, data):
                    """Interrupt final PASS once, leaving its interruption save available."""
                    events.append(name)
                    if artifacts.primary == "PASS" and name == interrupted_file:
                        raise KeyboardInterrupt()
                    return original(artifacts, name, data)
                with mock.patch.object(Artifacts, "_replace", new=interrupt_final):
                    code, peer, report, directory, _ = self.invoke()
                self.assertEqual(code, 130)
                self.assertEqual(report["exit_code"], 130)
                self.assertTrue(report["interrupted"])
                self.assertNotEqual(report["primary"], "PASS")
                self.assertEqual(events[-2:], ["report.json", "summary.md"])
                self.assertTrue(peer.closed)
                self.assertEqual(list(directory.glob("*.tmp")), [])

    def test_backend_exit_zero_cannot_replace_missing_proof_with_pass(self):
        """A backend SystemExit zero is saved as failure, unlike argparse help before I/O."""
        code, peer, report, _, _ = self.invoke(fault="backend-exit")
        self.assertEqual(code, 1)
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["primary"], "FAIL")
        self.assertTrue(peer.closed)

    def test_main_protocol_and_timeout_failure_saved_as_failed(self):
        """Save protocol and deadline failures with exit one and failed session proof."""
        for fault in ("loss", "timeout"):
            with self.subTest(fault=fault):
                code, peer, report, _, _ = self.invoke(fault=fault)
                self.assertEqual(code, 1)
                self.assertEqual(report["primary"], "FAIL")
                self.assertEqual(report["checks"]["session"]["status"], "FAIL")
                self.assertTrue(peer.closed)

    def test_main_missing_optional_tool_saves_blocked(self):
        """Missing optional serial support produces saved blocked evidence and exit two."""
        code, peer, report, _, _ = self.invoke(
            opening_error=runner.InvocationError("optional pyserial is unavailable"))
        self.assertEqual(code, 2)
        self.assertEqual(report["primary"], "BLOCKED")
        self.assertIn("pyserial", report["checks"]["session"]["detail"])
        self.assertFalse(peer.writes)

    def test_main_disconnect_saves_failure_and_closes_port(self):
        """A disconnected backend preserves failed proof and closes the serial handle."""
        code, peer, report, _, _ = self.invoke(fault="disconnect")
        self.assertIn(code, (1, 2))
        self.assertNotEqual(report["primary"], "PASS")
        self.assertEqual(report["checks"]["session"]["status"], "FAIL")
        self.assertTrue(peer.closed)

    def test_main_close_exception_saves_failed_cleanup_and_prevents_pass(self):
        """A close failure is saved separately and prevents an overall passing outcome."""
        code, peer, report, directory, _ = self.invoke(fault="close")
        self.assertEqual(code, 2)
        self.assertEqual(report["exit_code"], 2)
        self.assertEqual(report["cleanup"], "FAIL")
        self.assertTrue(any("serial close failed" in item
                            for item in report["residual_uncertainty"]))
        self.assertTrue(peer.closed)
        self.assertTrue((directory / "summary.md").exists())

    def test_main_capture_cap_prevents_pass_and_saves_incomplete_evidence(self):
        """Capture exhaustion remains sticky and is saved as incomplete evidence."""
        code, peer, report, _, _ = self.invoke(
            extra=("--transcript-bytes", "100", "--tail-bytes", "32"))
        self.assertIn(code, (1, 2))
        self.assertNotEqual(report["primary"], "PASS")
        self.assertTrue(report["capture"]["lost"])
        self.assertTrue(peer.closed)

    def test_main_validates_invocation_before_open(self):
        """Invalid CLI options and absent required arguments never open a serial backend."""
        for extra in (("--timeout", "nan"), ("--write-timeout", "0"),
                      ("--response-bytes", "0"), ("--dtr", "toggle")):
            with self.subTest(extra=extra):
                code, _, report, _, opened = self.invoke(extra=extra)
                self.assertEqual(code, 2)
                opened.assert_not_called()
                self.assertIsNone(report)
        with mock.patch.object(runner, "open_serial") as opened, redirect_stderr(io.StringIO()):
            self.assertEqual(runner.main([]), 2)
        opened.assert_not_called()


def load_tests(loader, tests, pattern):
    """Register artifact failure coverage with this single runner test entrypoint."""
    import test_tusb32x_hil_artifacts
    tests.addTests(loader.loadTestsFromModule(test_tusb32x_hil_artifacts))
    return tests


if __name__ == "__main__":
    unittest.main()
