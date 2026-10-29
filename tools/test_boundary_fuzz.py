#!/usr/bin/env python3
"""Run bounded seeded mutations against serial evidence and archive boundaries.

Only the standard library and existing test peers are used. Each generated fault
has a known invalid property; unexpected acceptance is a failure, not a discarded
input. Valid controls exercise fragmentation, ownership and safe extraction. This
is deterministic mutation testing, not coverage-guided or interpreter-sanitized
fuzzing. No hardware, network, compiler or archive-provided code is executed.

Replay with --seed and --cases; unittest failures identify the surface, case and
mutation. Counts are per surface and capped to keep every invocation finite.
"""

from __future__ import annotations

import argparse
from contextlib import contextmanager
import gzip
import io
import json
from pathlib import Path
import random
import string
import tarfile
import tempfile
import unittest

import check_package as package
from test_tusb32x_hil_runner import FakeClock, Peer, encode_record, record
from tusb32x_cli_contract import parse_record
from tusb32x_hil_artifacts import Artifacts, CaptureLimits, EvidenceError
import tusb32x_hil_runner as runner


DEFAULT_SEED = 0x320322
DEFAULT_CASES = 48


class BoundaryFuzz(unittest.TestCase):
    """Check independent rejection and retention invariants for generated inputs."""

    seed = DEFAULT_SEED
    cases = DEFAULT_CASES

    def random_for(self, surface: int, case: int) -> random.Random:
        """Give every case an independent deterministic stream for short replays."""
        return random.Random(self.seed + surface * 1000003 + case * 9176)

    @contextmanager
    def session(self):
        """Borrow the real Session and on-disk Artifacts with a virtual-clock peer."""
        with tempfile.TemporaryDirectory(prefix="tusb32x-fuzz-session-") as root:
            clock = FakeClock()
            artifacts = Artifacts(Path(root), clock)
            peer = Peer(clock)
            nonces = iter(f"fuzz_{index}" for index in range(100))
            session = runner.Session(peer, artifacts, clock=clock,
                                     nonce_factory=lambda: next(nonces))
            try:
                session.begin()
                yield session, peer, artifacts
            finally:
                artifacts.close()

    def test_record_mutations(self):
        """Reject guaranteed schema damage while preserving valid numeric endpoints."""
        modes = ("schema", "duplicate", "missing", "overflow", "negative",
                 "control", "nonascii", "oversized")
        for case in range(self.cases):
            rng = self.random_for(1, case)
            mode = modes[case % len(modes)]
            with self.subTest(seed=self.seed, case=case, mutation=mode):
                nonce = "n" + "".join(rng.choices(string.ascii_letters, k=16))
                request = rng.choice((1, (1 << 64) - 1, rng.randrange(1, 1 << 64)))
                value = record("MARK", request, nonce)
                line = encode_record(value).decode("ascii")
                self.assertEqual(parse_record(line), value)
                self.assertEqual(parse_record(line.replace("\n", "\r\n")), value)
                if mode == "schema":
                    damaged = line.replace("schema=1", f"schema={rng.randrange(2, 1000)}")
                elif mode == "duplicate":
                    key = rng.choice(("schema", "nonce", "request"))
                    damaged = line.rstrip() + f" {key}={value[key]}\n"
                elif mode == "missing":
                    key = rng.choice(("schema", "nonce", "request"))
                    damaged = line.replace(f" {key}={value[key]}", "")
                elif mode == "overflow":
                    damaged = line.replace(f"request={request}",
                                           f"request={(1 << 64) + rng.randrange(10000)}")
                elif mode == "negative":
                    damaged = line.replace(f"request={request}", f"request=-{request}")
                elif mode == "control":
                    damaged = line.replace("nonce=", "nonce=\x1b")
                elif mode == "nonascii":
                    damaged = line.replace("nonce=", "nonce=\u00e9")
                else:
                    damaged = line.rstrip() + " extra=" + "a" * rng.randrange(512, 1024)
                with self.assertRaises(ValueError):
                    parse_record(damaged)

    def test_fragmented_valid_operations(self):
        """Split noisy valid traffic and deliver exactly the admitted retained token."""
        for case in range(max(1, self.cases // 8)):
            rng = self.random_for(2, case)
            with self.subTest(seed=self.seed, case=case), self.session() as state:
                session, peer, artifacts = state
                peer.fragments.extend(rng.randrange(1, 33) for _ in range(2000))
                peer.pending.extend(b"\x1b[32mordinary status\x1b[0m\r\nPASS\n")
                peer.response_hook = lambda _, rows: b"".join(
                    map(encode_record, rows)).replace(b"\n", b"\r\n")
                admitted = session.command("read")
                self.assertTrue(admitted.admitted)
                self.assertIsNone(admitted.end)
                peer.complete()
                result = session.wait_operation()
                self.assertEqual(result["token"], admitted.ack["token"])
                self.assertEqual(result["code"], "OK")
                self.assertIsNotNone(session.operation)
                self.assertFalse(any(text.startswith("result take") for text in peer.writes))
                session.take_result()
                self.assertIsNone(session.operation)
                self.assertEqual(peer.writes[-1], "result take " + admitted.ack["token"])
                session.finish()
                artifacts.primary = "PASS"
                self.assertEqual(artifacts.exit_code(), 0)

    def test_session_fault_mutations(self):
        """Never accept damaged closure or consume a token after uncertain evidence."""
        modes = ("truncate", "duplicate", "stale", "nonce", "foreign_token",
                 "foreign_origin", "noise", "read_error", "short_write", "capture",
                 "missing", "schema")
        for case in range(self.cases):
            rng = self.random_for(3, case)
            mode = modes[case % len(modes)]
            with self.subTest(seed=self.seed, case=case, mutation=mode), self.session() as state:
                session, peer, artifacts = state
                session.command("read")
                owned = session.operation.token
                closure = peer.complete()
                peer.pending.clear()
                rows = [dict(row) for row in closure]
                index = rng.randrange(len(rows))
                if mode == "duplicate":
                    # Duplicate RESULT/detail before END, so rejection cannot
                    # depend on whether a trailing END arrives in a later read.
                    index = rng.randrange(2)
                    rows.insert(index, dict(rows[index]))
                elif mode in {"stale", "nonce", "foreign_token", "foreign_origin"}:
                    index = rng.randrange(2)
                    key, value = {
                        "stale": ("request", "1"),
                        "nonce": ("nonce", "foreign_" + str(rng.randrange(100000))),
                        "foreign_token": ("token", str(owned + rng.randrange(1, 100000))),
                        "foreign_origin": ("origin_request", "1"),
                    }[mode]
                    rows[index][key] = value
                elif mode == "missing":
                    del rows[index]
                wire = b"".join(map(encode_record, rows))
                if mode == "truncate":
                    wire = wire[:rng.randrange(1, len(wire))]
                elif mode == "noise":
                    wire = b"PASS\n" * rng.randrange(2050, 2100) + wire
                elif mode == "schema":
                    wire = wire.replace(b"schema=1", b"schema=9", 1)
                peer.pending.extend(wire)
                peer.fragments.extend(rng.randrange(1, 65) for _ in range(2000))
                expected = runner.ProtocolError
                action = session.wait_operation
                if mode == "read_error":
                    peer.read_error = OSError("seeded receive failure")
                    expected = OSError
                elif mode == "short_write":
                    peer.short_write = True
                    action = lambda: session.command("bus")
                elif mode == "capture":
                    used = (artifacts.directory / "transcript.log").stat().st_size
                    artifacts.limits = CaptureLimits(used + 1, 1)
                    expected = EvidenceError
                before = list(peer.writes)
                with self.assertRaises(expected):
                    action()
                self.assertFalse(session.valid)
                self.assertIsNotNone(session.operation)
                self.assertEqual(session.operation.token, owned)
                self.assertEqual(artifacts.primary, "FAIL")
                self.assertNotEqual(artifacts.exit_code(), 0)
                self.assertFalse(any(text.startswith(("result take", "cancel"))
                                     for text in peer.writes[len(before):]))
                writes = list(peer.writes)
                with self.assertRaises(runner.SessionBlocked):
                    session.take_result()
                self.assertEqual(peer.writes, writes)
                report = json.loads((artifacts.directory / "report.json").read_text())
                self.assertEqual(report["primary"], "FAIL")
                self.assertNotEqual(report["exit_code"], 0)
                if mode == "capture":
                    self.assertTrue(report["capture"]["lost"])

    @staticmethod
    def tar_bytes(entries):
        """Serialize tiny fixture members without extraction or executable content."""
        stream = io.BytesIO()
        with tarfile.open(fileobj=stream, mode="w", format=tarfile.PAX_FORMAT) as archive:
            for member, payload in entries:
                archive.addfile(member, io.BytesIO(payload) if member.isreg() else None)
        return stream.getvalue()

    @staticmethod
    def member(name, payload):
        """Create an ordinary fixture file with the exact payload byte count."""
        member = tarfile.TarInfo(name)
        member.size = len(payload)
        return member, payload

    def test_archive_mutations(self):
        """Reject unsafe paths/headers before extracting even preceding safe files."""
        modes = ("path", "checksum", "truncated", "link", "size", "pax",
                 "duplicate", "trailer")
        for case in range(self.cases):
            rng = self.random_for(4, case)
            mode = modes[case % len(modes)]
            with self.subTest(seed=self.seed, case=case, mutation=mode), \
                    tempfile.TemporaryDirectory(prefix="tusb32x-fuzz-package-") as temporary:
                root = Path(temporary)
                work = root / "work"
                work.mkdir()
                sentinel = root / "outside.txt"
                sentinel.write_bytes(b"outside must survive")
                payload = bytes(rng.randrange(256) for _ in range(rng.randrange(1, 65)))
                name = "dir/" + "".join(rng.choices(string.ascii_letters, k=12)) + ".txt"
                self.assertEqual(package.validate_name(name), tuple(name.split("/")))
                entries = [self.member("a.txt", b"safe"), self.member(name, payload)]
                required = {"a.txt", name}
                source = root / "input.tar.gz"
                # Every eighth case proves the whole extraction path accepts
                # the original archive before the generated mutation rejects.
                if case % 8 == 0:
                    positive_work = root / "positive"
                    positive_work.mkdir()
                    source.write_bytes(gzip.compress(self.tar_bytes(entries), mtime=0))
                    extracted = package.extract_archive(source, positive_work, required)
                    self.assertTrue(extracted.resolve().is_relative_to(positive_work.resolve()))
                    self.assertEqual((extracted / name).read_bytes(), payload)
                    self.assertEqual((extracted / "a.txt").read_bytes(), b"safe")
                limits = package.ArchiveLimits()
                if mode == "path":
                    component = "".join(rng.choices(string.ascii_letters, k=9))
                    unsafe = rng.choice((f"../{component}", f"/{component}",
                                         f"{component}/../x", f"{component}/CON.txt",
                                         f"{component}\\x", f"{component}/x:stream",
                                         f"{component}//x", f"{component}./x"))
                    with self.assertRaises(package.PackageError):
                        package.validate_name(unsafe)
                    entries[1][0].name = unsafe
                elif mode == "link":
                    entries[1][0].type = rng.choice((tarfile.SYMTYPE, tarfile.LNKTYPE))
                    entries[1][0].linkname = "../outside.txt"
                    entries[1][0].size = 0
                elif mode == "size":
                    oversized = payload + b"bounded"
                    entries[1] = self.member(name, oversized)
                    limits = package.ArchiveLimits(max_member_bytes=len(oversized) - 1)
                elif mode == "pax":
                    entries[1][0].pax_headers = {"path": "../outside.txt"}
                elif mode == "duplicate":
                    entries.append(entries[rng.randrange(2)])
                raw = self.tar_bytes(entries)
                if mode == "checksum":
                    raw = bytearray(raw)
                    # Change one octal mode digit outside the checksum field.
                    # Bytes after its NUL terminator may legally be ignored;
                    # mutating those would not guarantee an invalid checksum.
                    offset = rng.randrange(100, 107)
                    raw[offset] = ord("1") if raw[offset] == ord("0") else ord("0")
                    raw = bytes(raw)
                elif mode == "truncated":
                    raw = raw[:rng.randrange(1, 511)]
                elif mode == "trailer":
                    # Keep the physical block length valid; corrupt zero padding.
                    raw = raw[:-1] + bytes([rng.randrange(1, 256)])
                source.write_bytes(gzip.compress(raw, mtime=0))
                with self.assertRaises(package.PackageError):
                    package.extract_archive(source, work, required, limits)
                self.assertEqual(sentinel.read_bytes(), b"outside must survive")
                self.assertFalse(list(work.rglob("extracted")))
                self.assertFalse(list(work.rglob("renamed-tusb32x")))
                self.assertFalse(list(work.rglob("a.txt")))


def main():
    """Run a bounded replay and return nonzero for every failed invariant."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--seed", type=lambda text: int(text, 0), default=DEFAULT_SEED)
    parser.add_argument("--cases", type=int, default=DEFAULT_CASES)
    args = parser.parse_args()
    if not 0 <= args.seed < 1 << 64 or not 1 <= args.cases <= 4096:
        parser.error("seed must be uint64 and cases must be in 1..4096")
    BoundaryFuzz.seed = args.seed
    BoundaryFuzz.cases = args.cases
    print(f"boundary fuzz seed={args.seed} cases={args.cases}", flush=True)
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(BoundaryFuzz))
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
