"""Exercise bounded evidence capture, atomic save failures and verdict precedence.

All files are temporary and clocks are injected. No serial backend, wall-clock
sleep or hardware is used; passing tests establish host artifact behavior only.
The registered hil_runner suite also imports these unittest cases.
"""

from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock

from tusb32x_hil_artifacts import Artifacts, CaptureLimits, EvidenceError


class ArtifactTests(unittest.TestCase):
    """Verify public capture/report behavior using temporary directories and faults."""

    def setUp(self) -> None:
        """Give each case an isolated evidence directory and a deterministic clock."""
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.now = 10.0

    def clock(self) -> float:
        """Return caller-controlled monotonic seconds without waiting."""
        return self.now

    def artifacts(self, limits: CaptureLimits | None = None) -> Artifacts:
        """Open one artifact owner and ensure its transcript closes before directory cleanup."""
        artifacts = Artifacts(self.directory, self.clock, limits)
        self.addCleanup(artifacts.close)
        return artifacts

    def report(self) -> dict:
        """Read the authoritative checkpoint from disk independently of the object."""
        return json.loads((self.directory / "report.json").read_text(encoding="utf-8"))

    def passing(self, artifacts: Artifacts) -> None:
        """Declare narrow simulated host proof without claiming hardware qualification."""
        artifacts.primary = "PASS"
        artifacts.check("simulated host proof", "PASS", "fake session only")

    def assert_no_temporaries(self) -> None:
        """Check that failed and successful replacements leave no temporary files."""
        self.assertEqual(list(self.directory.glob("*.tmp")), [])

    def test_exclusive_transcript_preserves_existing_evidence(self) -> None:
        """An existing transcript blocks a new run without changing any existing bytes."""
        transcript = self.directory / "transcript.log"
        transcript.write_bytes(b"previous run")
        with self.assertRaises(EvidenceError):
            self.artifacts()
        self.assertEqual(transcript.read_bytes(), b"previous run")

    def test_invalid_limits_rejected_before_file_creation(self) -> None:
        """Invalid, noninteger and excessive bounds never touch the output directory."""
        self.assertEqual(CaptureLimits().transcript_bytes, 32 * 1024 * 1024)
        self.assertEqual(CaptureLimits().tail_bytes, 64 * 1024)
        for value in (0, -1, True, 1.5, float("nan"), float("inf"), "12", 128 * 1024**2 + 1):
            with self.subTest(value=value), self.assertRaises(EvidenceError):
                CaptureLimits(transcript_bytes=value)
        for value in (0, True, 1024**2 + 1):
            with self.subTest(tail=value), self.assertRaises(EvidenceError):
                CaptureLimits(tail_bytes=value)
        with self.assertRaises(EvidenceError):
            CaptureLimits(transcript_bytes=5, tail_bytes=6)
        with self.assertRaises(EvidenceError):
            Artifacts(self.directory / "unused", self.clock, object())
        self.assertFalse((self.directory / "unused").exists())

    def test_streamed_escape_timestamps_and_direction_counts(self) -> None:
        """Records preserve byte values without interpreting ANSI, newline or backslash."""
        artifacts = self.artifacts()
        artifacts.record("TX", b"a\r\n\\\x00\x1b\xff")
        self.now = 11.125
        artifacts.record("RX", b"TUSB ACK\n")
        transcript = (self.directory / "transcript.log").read_bytes()
        self.assertEqual(transcript, b"10.000000000 TX bytes=7 a\\x0d\\x0a\\x5c\\x00\\x1b\\xff\n"
                         b"11.125000000 RX bytes=9 TUSB ACK\\x0a\n")
        capture = artifacts.snapshot()["capture"]
        self.assertEqual(capture["tx_bytes"], 7)
        self.assertEqual(capture["rx_bytes"], 9)
        self.assertEqual(capture["records"], 2)
        self.assertEqual(capture["transcript_bytes"], len(transcript))
        self.assertEqual(capture["tail"].encode("ascii"), transcript)

    def test_exact_cap_includes_escaped_bytes_and_framing(self) -> None:
        """A record exactly fitting the encoded cap succeeds; one extra byte loses evidence."""
        expected = b"10.000000000 RX bytes=1 \\x00\n"
        artifacts = self.artifacts(CaptureLimits(len(expected), 8))
        artifacts.record("RX", b"\x00")
        self.assertEqual((self.directory / "transcript.log").read_bytes(), expected)
        with self.assertRaises(EvidenceError):
            artifacts.record("TX", b"")
        artifacts.checkpoint()
        self.assertTrue(self.report()["capture"]["lost"])
        self.assertEqual(self.report()["exit_code"], 2)
        self.assertEqual((self.directory / "transcript.log").read_bytes(), expected)

    def test_escaped_cap_rejects_whole_fragment(self) -> None:
        """Escaping expansion is checked before writing a plausible partial record."""
        expected = b"10.000000000 RX bytes=1 \\x00\n"
        artifacts = self.artifacts(CaptureLimits(len(expected) - 1, 8))
        with self.assertRaises(EvidenceError):
            artifacts.record("RX", b"\x00")
        self.assertEqual((self.directory / "transcript.log").read_bytes(), b"")
        artifacts.checkpoint()
        self.assertIn("cap", self.report()["capture"]["error"])
        with self.assertRaises(EvidenceError):
            artifacts.record("TX", b"later")

    def test_tail_memory_and_report_tail_have_explicit_bounds(self) -> None:
        """A long transcript retains only its configured suffix and bounded report excerpt."""
        artifacts = self.artifacts(CaptureLimits(2 * 1024**2, 1024**2))
        payload = b"0123456789abcdef" * 4096
        for _ in range(17):
            artifacts.record("RX", payload)
        capture = artifacts.snapshot()["capture"]
        transcript = (self.directory / "transcript.log").read_bytes()
        self.assertEqual(capture["retained_tail_bytes"], 1024**2)
        self.assertEqual(len(capture["tail"]), 64 * 1024)
        self.assertEqual(capture["tail"].encode("ascii"), transcript[-64 * 1024:])
        self.assertEqual(capture["report_tail_omitted_bytes"], 1024**2 - 64 * 1024)
        artifacts.checkpoint()
        self.assertLess((self.directory / "report.json").stat().st_size, 512 * 1024)

    def test_small_tail_is_exact_suffix_without_growing_journal(self) -> None:
        """Repeated fragments update aggregate counters while retaining a fixed suffix."""
        artifacts = self.artifacts(CaptureLimits(65536, 19))
        for _ in range(200):
            artifacts.record("RX", b"abcdef")
        capture = artifacts.snapshot()["capture"]
        self.assertEqual(capture["records"], 200)
        self.assertEqual(capture["rx_bytes"], 1200)
        self.assertEqual(capture["retained_tail_bytes"], 19)
        self.assertEqual(capture["tail"].encode("ascii"),
                         (self.directory / "transcript.log").read_bytes()[-19:])

    def test_invalid_clock_is_rejected_before_open(self) -> None:
        """A bad initial clock cannot create evidence files."""
        for value in (-1, float("nan"), float("inf"), True, "1", 10**1000):
            with self.subTest(value=value), self.assertRaises(EvidenceError):
                Artifacts(self.directory, lambda: value)
        self.assertFalse((self.directory / "transcript.log").exists())

    def test_clock_regression_failure_can_be_checkpointed(self) -> None:
        """Checkpoint uses retained state even when the injected clock remains broken."""
        artifacts = self.artifacts()
        artifacts.record("RX", b"first")
        self.passing(artifacts)
        self.now = 9.0
        with self.assertRaises(EvidenceError):
            artifacts.record("RX", b"regressed")
        artifacts.checkpoint()
        self.assertEqual(self.report()["exit_code"], 2)
        self.assertIn("clock", self.report()["capture"]["error"])

    def test_clock_exception_can_be_checkpointed(self) -> None:
        """A raised clock exception is saved as incomplete evidence rather than success."""
        clock = mock.Mock(side_effect=[0.0, OSError("clock failed")])
        artifacts = Artifacts(self.directory, clock)
        self.addCleanup(artifacts.close)
        with self.assertRaises(EvidenceError):
            artifacts.record("RX", b"lost")
        artifacts.checkpoint()
        self.assertTrue(self.report()["capture"]["lost"])

    def test_transcript_io_failure_preserves_previous_report_then_saves_failure(self) -> None:
        """A failed write does not mutate a saved report until an explicit checkpoint."""
        artifacts = self.artifacts()
        artifacts.checkpoint()
        previous = (self.directory / "report.json").read_bytes()
        stream = artifacts._stream
        with mock.patch.object(artifacts, "_stream", wraps=stream) as broken:
            broken.write.side_effect = OSError("disk full")
            with self.assertRaises(EvidenceError):
                artifacts.record("TX", b"request")
        self.assertEqual((self.directory / "report.json").read_bytes(), previous)
        artifacts.checkpoint()
        self.assertIn("disk full", self.report()["capture"]["error"])
        self.assertEqual(artifacts.exit_code(), 2)

    def test_short_write_is_capture_loss(self) -> None:
        """A short disk write preserves its accepted prefix and records incomplete capture."""
        artifacts = self.artifacts()
        stream = artifacts._stream

        def short_write(data: bytes) -> int:
            """Accept exactly one byte to simulate a partial successful backend write."""
            return stream.write(data[:1])

        with mock.patch.object(artifacts, "_stream", wraps=stream) as broken:
            broken.write.side_effect = short_write
            with self.assertRaises(EvidenceError):
                artifacts.record("RX", b"reply")
        capture = artifacts.snapshot()["capture"]
        self.assertTrue(capture["lost"])
        self.assertEqual(capture["transcript_bytes"], 1)
        self.assertEqual(capture["tail"], "1")
        artifacts.checkpoint()

    def test_atomic_replace_failure_leaves_previous_report_and_no_temporary(self) -> None:
        """Failure replacing JSON keeps both prior files byte-for-byte intact."""
        artifacts = self.artifacts()
        artifacts.checkpoint()
        previous = {name: (self.directory / name).read_bytes()
                    for name in ("report.json", "summary.md")}
        artifacts.primary = "FAIL"
        with mock.patch("tusb32x_hil_artifacts.os.replace", side_effect=OSError("denied")):
            with self.assertRaises(EvidenceError):
                artifacts.checkpoint()
        for name, data in previous.items():
            self.assertEqual((self.directory / name).read_bytes(), data)
        self.assert_no_temporaries()
        artifacts.checkpoint()
        self.assertEqual(self.report()["primary"], "FAIL")

    def test_second_replace_failure_keeps_old_summary_and_new_authoritative_json(self) -> None:
        """Individual atomic files may have different generations after summary failure."""
        artifacts = self.artifacts()
        artifacts.checkpoint()
        previous_summary = (self.directory / "summary.md").read_bytes()
        artifacts.primary = "FAIL"
        from os import replace

        def replace_json_only(source: Path, destination: Path) -> None:
            """Allow JSON replacement and fail only the later Markdown replacement."""
            self.assertEqual(source.parent, destination.parent)
            if destination.name == "summary.md":
                raise OSError("summary locked")
            replace(source, destination)

        with mock.patch("tusb32x_hil_artifacts.os.replace", side_effect=replace_json_only):
            with self.assertRaises(EvidenceError):
                artifacts.checkpoint()
        self.assertEqual(self.report()["primary"], "FAIL")
        self.assertEqual((self.directory / "summary.md").read_bytes(), previous_summary)
        self.assert_no_temporaries()

    def test_temporary_flush_failure_preserves_previous_report(self) -> None:
        """Disk synchronization failure happens before replacement and removes temporary data."""
        artifacts = self.artifacts()
        artifacts.checkpoint()
        previous = (self.directory / "report.json").read_bytes()
        with mock.patch("tusb32x_hil_artifacts.os.fsync", side_effect=OSError("disk error")):
            with self.assertRaises(EvidenceError):
                artifacts.checkpoint()
        self.assertEqual((self.directory / "report.json").read_bytes(), previous)
        self.assert_no_temporaries()

    def test_failed_summary_corrects_passing_json_and_save_error_stays_incomplete(self) -> None:
        """A failed summary cannot leave authoritative PASS when JSON storage still works."""
        artifacts = self.artifacts()
        self.passing(artifacts)
        artifacts.checkpoint()
        previous_summary = (self.directory / "summary.md").read_bytes()
        from os import replace

        def replace_json_only(source: Path, destination: Path) -> None:
            """Fail the summary while permitting both JSON replacements."""
            if destination.name == "summary.md":
                raise OSError("summary locked")
            replace(source, destination)

        with mock.patch("tusb32x_hil_artifacts.os.replace", side_effect=replace_json_only):
            with self.assertRaisesRegex(EvidenceError, "summary locked"):
                artifacts.checkpoint()
        saved = self.report()
        self.assertEqual(saved["primary"], "PASS")
        self.assertEqual(saved["exit_code"], 2)
        self.assertIn("summary locked", saved["save_error"])
        self.assertEqual((self.directory / "summary.md").read_bytes(), previous_summary)
        self.assertEqual(artifacts.exit_code(), 2)
        artifacts.checkpoint()
        self.assertEqual(self.report()["exit_code"], 2)
        self.assertIn("summary locked", (self.directory / "summary.md").read_text())
        self.assert_no_temporaries()

    def test_save_failure_preserves_primary_failure_and_interruption_precedence(self) -> None:
        """Incomplete saving cannot downgrade a proved failure or saved interruption."""
        from os import replace

        for primary, interrupted, expected in (("FAIL", False, 1), ("PASS", True, 130)):
            with self.subTest(expected=expected):
                artifacts = Artifacts(self.directory / str(expected), self.clock)
                self.addCleanup(artifacts.close)
                self.passing(artifacts)
                artifacts.primary = primary
                artifacts.interrupted = interrupted

                def replace_json_only(source: Path, destination: Path) -> None:
                    """Leave JSON available while summary saving fails."""
                    if destination.name == "summary.md":
                        raise OSError("summary locked")
                    replace(source, destination)

                with mock.patch("tusb32x_hil_artifacts.os.replace", side_effect=replace_json_only):
                    with self.assertRaises(EvidenceError):
                        artifacts.checkpoint()
                saved = json.loads((artifacts.directory / "report.json").read_text())
                self.assertEqual(saved["exit_code"], expected)
                self.assertEqual(saved["primary"], primary)
                self.assertEqual(saved["interrupted"], interrupted)
                self.assertTrue(saved["save_error"])
                self.assertEqual(artifacts.exit_code(), expected)
                self.assertEqual(list(artifacts.directory.glob("*.tmp")), [])

    def test_initial_json_failure_preserves_prior_bytes_and_marks_owner_incomplete(self) -> None:
        """Failure before replacing JSON leaves existing evidence and a nonpassing owner."""
        artifacts = self.artifacts()
        self.passing(artifacts)
        artifacts.checkpoint()
        previous = {name: (self.directory / name).read_bytes()
                    for name in ("report.json", "summary.md")}
        with mock.patch("tusb32x_hil_artifacts.os.replace", side_effect=OSError("disk full")):
            with self.assertRaisesRegex(EvidenceError, "disk full"):
                artifacts.checkpoint()
        for name, data in previous.items():
            self.assertEqual((self.directory / name).read_bytes(), data)
        self.assertEqual(artifacts.exit_code(), 2)
        self.assertIn("disk full", artifacts.snapshot()["save_error"])
        self.assert_no_temporaries()

    def test_failed_corrective_json_preserves_published_bytes_and_reports_both_errors(self) -> None:
        """Unavailable correction storage raises honestly without claiming persisted failure."""
        artifacts = self.artifacts()
        self.passing(artifacts)
        expected_json = artifacts._json(artifacts.snapshot())
        from os import replace
        calls = []

        def fail_after_first_json(source: Path, destination: Path) -> None:
            """Publish JSON once, then fail the summary and the corrective JSON."""
            calls.append(destination.name)
            if len(calls) == 1:
                replace(source, destination)
            else:
                raise OSError("summary locked" if len(calls) == 2 else "JSON locked")

        with mock.patch("tusb32x_hil_artifacts.os.replace", side_effect=fail_after_first_json):
            with self.assertRaisesRegex(EvidenceError, "summary locked.*JSON locked"):
                artifacts.checkpoint()
        self.assertEqual(calls, ["report.json", "summary.md", "report.json"])
        self.assertEqual((self.directory / "report.json").read_bytes(), expected_json)
        self.assertFalse((self.directory / "summary.md").exists())
        self.assertEqual(artifacts.exit_code(), 2)
        self.assertIn("summary locked", artifacts.snapshot()["save_error"])
        self.assert_no_temporaries()

    def test_good_cleanup_never_repairs_failed_work(self) -> None:
        """Primary and individual proof failures retain exit one after successful cleanup."""
        artifacts = self.artifacts()
        self.passing(artifacts)
        artifacts.primary = "FAIL"
        artifacts.cleanup = "PASS"
        self.assertEqual(artifacts.exit_code(), 1)
        artifacts.primary = "PASS"
        artifacts.check("transport", "FAIL", "failed payload despite delivery OK")
        self.assertEqual(artifacts.exit_code(), 1)
        with self.assertRaises(EvidenceError):
            artifacts.check("transport", "PASS")
        self.assertEqual(artifacts.exit_code(), 1)

    def test_missing_proof_cleanup_failure_and_residual_uncertainty_are_incomplete(self) -> None:
        """PASS requires real declared proof, all of it passed, and no uncertain effects."""
        artifacts = self.artifacts()
        self.assertEqual(artifacts.exit_code(), 2)
        artifacts.primary = "PASS"
        self.assertEqual(artifacts.exit_code(), 2)
        self.passing(artifacts)
        self.assertEqual(artifacts.exit_code(), 0)
        artifacts.check("required identity", "NOT RUN")
        self.assertEqual(artifacts.exit_code(), 2)
        artifacts.check("required identity", "BLOCKED", "no fixture")
        self.assertEqual(artifacts.exit_code(), 2)
        artifacts.check("required identity", "PASS", "simulated identity only")
        for cleanup, expected in (("FAIL", 2), ("BLOCKED", 2), ("NOT RUN", 0), ("PASS", 0)):
            artifacts.cleanup = cleanup
            self.assertEqual(artifacts.exit_code(), expected)
        artifacts.residual_uncertainty.append("hardware effect unknown")
        self.assertEqual(artifacts.exit_code(), 2)

    def test_interrupt_overrides_other_failure_and_is_saved(self) -> None:
        """Saved interruption has exit 130 while preserving primary and cleanup outcomes."""
        artifacts = self.artifacts()
        artifacts.primary = "FAIL"
        artifacts.cleanup = "BLOCKED"
        artifacts.interrupted = True
        artifacts.checkpoint()
        self.assertEqual(artifacts.exit_code(), 130)
        self.assertTrue(self.report()["interrupted"])
        self.assertEqual(self.report()["primary"], "FAIL")
        self.assertEqual(self.report()["cleanup"], "BLOCKED")

    def test_unknown_status_and_invalid_check_are_rejected_without_replacing_proof(self) -> None:
        """Unknown check values cannot be silently interpreted as successful evidence."""
        artifacts = self.artifacts()
        artifacts.check("proof", "NOT RUN", "previous")
        for status in ("OK", "pass", "NOT_RUN", "", None, []):
            with self.subTest(status=status), self.assertRaises(EvidenceError):
                artifacts.check("proof", status)
        for name, detail in (("", "x"), ("n" * 129, ""), ("proof", "d" * 4097)):
            with self.assertRaises(EvidenceError):
                artifacts.check(name, "PASS", detail)
        self.assertEqual(artifacts.snapshot()["checks"]["proof"],
                         {"status": "NOT RUN", "detail": "previous"})
        artifacts.primary = "UNKNOWN"
        with self.assertRaises(EvidenceError):
            artifacts.exit_code()

    def test_checks_and_residuals_are_bounded(self) -> None:
        """Count limits fail before publishing excess proof or uncertain state."""
        artifacts = self.artifacts()
        for index in range(1024):
            artifacts.check(f"check-{index}", "PASS")
        with self.assertRaises(EvidenceError):
            artifacts.check("too many", "PASS")
        self.assertEqual(len(artifacts.snapshot()["checks"]), 1024)
        artifacts.residual_uncertainty = ["unknown"] * 64
        artifacts.checkpoint()
        artifacts.residual_uncertainty.append("overflow")
        with self.assertRaises(EvidenceError):
            artifacts.checkpoint()
        self.assertEqual(len(self.report()["residual_uncertainty"]), 64)

    def test_invalid_metadata_is_bounded_and_previous_checkpoint_survives(self) -> None:
        """Unsupported objects, cycles, excessive strings and nonfinite JSON are rejected."""
        artifacts = self.artifacts()
        artifacts.checkpoint()
        previous = (self.directory / "report.json").read_bytes()
        cyclic: dict = {}
        cyclic["cycle"] = cyclic
        for metadata in ({"x": float("nan")}, {"x": object()}, {"x": "a" * 8193},
                         {"x": 2**129}, cyclic, {"x": [0] * 4097}, {1: "bad key"},
                         {"bad": "\ud800"}):
            artifacts.metadata = metadata
            with self.subTest(metadata_type=type(metadata)), self.assertRaises(EvidenceError):
                artifacts.checkpoint()
            self.assertEqual((self.directory / "report.json").read_bytes(), previous)
        artifacts.metadata = {"host": {"git": "abc"}, "firmware": {"git": "unknown"}}
        artifacts.checkpoint()
        self.assertEqual(self.report()["metadata"], artifacts.metadata)
        self.assert_no_temporaries()

    def test_total_json_limit_rejects_excess_metadata_before_replacement(self) -> None:
        """Individually legal fields cannot exceed the total report size ceiling."""
        artifacts = self.artifacts()
        artifacts.checkpoint()
        previous = (self.directory / "report.json").read_bytes()
        artifacts.metadata = {f"key-{index}": "x" * 8192 for index in range(65)}
        with self.assertRaises(EvidenceError):
            artifacts.checkpoint()
        self.assertEqual((self.directory / "report.json").read_bytes(), previous)
        self.assert_no_temporaries()

    def test_snapshot_is_detached_and_summary_escapes_untrusted_text(self) -> None:
        """Returned-copy edits cannot alter stored proof, host identity or firmware identity."""
        artifacts = self.artifacts()
        artifacts.metadata = {"host": {"git": "host123"},
                              "firmware": {"git": "unknown", "notes": ["<script>"]}}
        artifacts.check("proof|name", "PASS", "<script>\n| detail")
        snapshot = artifacts.snapshot()
        snapshot["checks"]["proof|name"]["status"] = "FAIL"
        snapshot["metadata"]["host"]["git"] = "changed"
        self.assertEqual(artifacts.snapshot()["checks"]["proof|name"]["status"], "PASS")
        self.assertEqual(artifacts.metadata["host"]["git"], "host123")
        artifacts.checkpoint()
        summary = (self.directory / "summary.md").read_text(encoding="utf-8")
        self.assertNotIn("<script>", summary)
        self.assertIn("proof&#124;name", summary)
        self.assertIn("unknown", summary)

    def test_close_is_idempotent_and_checkpoint_remains_available(self) -> None:
        """Closing stops serial evidence while permitting final saved status."""
        artifacts = self.artifacts()
        artifacts.record("RX", b"last")
        artifacts.close()
        artifacts.close()
        artifacts.checkpoint()
        with self.assertRaises(EvidenceError):
            artifacts.record("RX", b"late")
        artifacts.checkpoint()
        self.assertTrue(self.report()["capture"]["lost"])

    def test_close_failure_is_recorded_and_can_be_saved(self) -> None:
        """A failed final flush cannot leave a passing report when the caller saves it."""
        artifacts = self.artifacts()
        stream = artifacts._stream
        with mock.patch.object(artifacts, "_stream", wraps=stream) as broken:
            broken.flush.side_effect = OSError("flush failed")
            with self.assertRaises(EvidenceError):
                artifacts.close()
            broken.close.assert_called_once()
        artifacts.checkpoint()
        self.assertIn("close failed", self.report()["capture"]["error"])

    def test_invalid_record_inputs_do_not_change_capture(self) -> None:
        """Local API validation rejects unknown directions and nonbytes without evidence loss."""
        artifacts = self.artifacts()
        for direction, data in (("BAD", b"x"), ("TX", "x"), ("RX", bytearray(b"x"))):
            with self.assertRaises(EvidenceError):
                artifacts.record(direction, data)
        self.assertFalse(artifacts.snapshot()["capture"]["lost"])
        self.assertEqual((self.directory / "transcript.log").read_bytes(), b"")


if __name__ == "__main__":
    unittest.main()
