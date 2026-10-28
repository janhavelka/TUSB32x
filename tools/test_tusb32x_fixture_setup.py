"""Test strict fixture declarations and finite operator console input without hardware.

All clock advancement and console input are injected; no test sleeps or reads a
real console. This suite is loaded by the existing runner test registration.
"""

from contextlib import contextmanager
import io
import json
from pathlib import Path
import tempfile
import sys
import unittest
from unittest import mock

import tusb32x_hil_fixture_setup as setup


def declaration(**changes):
    """Return a complete explicit setup suitable for a LAI orientation fixture."""
    result = {
        "board": "fixture-board-rev-A", "part": "TUSB320LAI", "adapter": "native IDF",
        "wiring": "CC1/CC2 reversible fixture; VBUS_DET divider populated; INT_N wired",
        "addr_strap": "ADDR open, address 0x47", "enable": "EN_N held low",
        "supply_mv": 3500, "partner": "partner-serial-123", "operator": "bench operator",
        "variant": "tusb320lai", "capabilities": ["orientation"],
    }
    result.update(changes)
    return result


class Clock:
    """Virtual monotonic time with recorded, nonblocking sleep requests."""

    def __init__(self):
        """Start at a nonzero time and retain every requested wait."""
        self.now = 100.0
        self.waits = []

    def __call__(self):
        """Return the virtual monotonic time without implicit advancement."""
        return self.now

    def sleep(self, seconds):
        """Advance only virtual time, recording the requested finite delay."""
        self.waits.append(seconds)
        self.now += seconds


class SetupTests(unittest.TestCase):
    """Validate the entire setup schema before any plan may perform hardware I/O."""

    @contextmanager
    def file(self, value, *, raw=False):
        """Create a temporary UTF-8 setup, preserving raw bytes for parser negatives."""
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "setup.json"
            path.write_bytes(value if raw else json.dumps(value).encode("utf-8"))
            yield path

    def load(self, value, variant="tusb320lai", suites=("orientation",)):
        """Load one temporary setup with a known fixture identity."""
        with self.file(value) as path:
            return setup.load_setup(path, "bench-01", variant, suites)

    def test_complete_setup_and_capability_absence(self):
        """An absent capability remains explicit, leaving runtime NOT RUN to the owner."""
        value = declaration(capabilities=[], startup_ready=False, base_recovery_complete=True)
        result = self.load(value)
        self.assertEqual(result, dict(value, id="bench-01"))
        self.assertIs(type(result), dict)
        self.assertNotIn("id", value)

    def test_exact_fields_and_string_boundaries(self):
        """Reject missing/unknown fields, controls, Unicode and unbounded declaration text."""
        self.load(declaration(board="b" * 256, wiring="VBUS_DET " + "w" * 1015))
        bad = [declaration(extra="ignored"), declaration(board=""), declaration(board=" " * 3),
               declaration(board="b" * 257), declaration(board="\x1b[31mboard"),
               declaration(board="boärd"), declaration(wiring="VBUS_DET " + "w" * 1016),
               declaration(wiring="CC1 CC2 and INT_N"), declaration(enable=False),
               declaration(startup_ready=1), declaration(base_recovery_complete="true"),
               declaration(variant="tusb320hai")]
        missing = declaration()
        del missing["operator"]
        bad.extend([missing, [], None])
        for value in bad:
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.load(value)

    def test_json_framing_duplicate_and_size(self):
        """Reject malformed, duplicate, non-UTF8 and oversized JSON with bounded reads."""
        good = json.dumps(declaration()).encode("ascii")
        for raw in (b'{"board":"a","board":"b"}', good + b" {}", b"\xff", b"{",
                    b"[" * 2000 + b"]" * 2000, good.replace(b"3500", b"NaN"),
                    good + b" " * (setup.MAX_SETUP_BYTES + 1 - len(good))):
            with self.subTest(raw=raw[:30]), self.file(raw, raw=True) as path:
                with self.assertRaises(ValueError):
                    setup.load_setup(path, "bench", "tusb320lai", ("orientation",))
        with self.file(good + b" " * (setup.MAX_SETUP_BYTES - len(good)), raw=True) as path:
            self.assertEqual(setup.load_setup(path, "bench", "tusb320lai", ["orientation"])
                             ["id"], "bench")

    def test_fixture_id_and_path(self):
        """Fixture IDs are bounded explicit names, and unreadable paths fail as setup errors."""
        with self.file(declaration()) as path:
            for identity in (None, "", "a" * 65, "../bench", "bench\n", "bé"):
                with self.subTest(identity=identity), self.assertRaises(ValueError):
                    setup.load_setup(path, identity, "tusb320lai", ["orientation"])
            for variant in (None, "tusb321", [], True):
                with self.subTest(variant=variant), self.assertRaises(ValueError):
                    setup.load_setup(path, "bench", variant, ["orientation"])
            for suites in ([], "orientation", [None], ["unknown"], ["orientation"] * 7):
                with self.subTest(suites=suites), self.assertRaises(ValueError):
                    setup.load_setup(path, "bench", "tusb320lai", suites)
        with self.assertRaises(ValueError):
            setup.load_setup(None, "bench", "tusb320lai", ["orientation"])

    def test_capability_types_duplicates_and_unknown(self):
        """Capability names form an exact unique bounded list, never arbitrary truthy values."""
        self.load(declaration(capabilities=list(setup.CAPABILITIES)))
        for capabilities in (None, "orientation", {}, [1], [[]], ["unknown"],
                             ["orientation", "orientation"], ["orientation"] * 9):
            with self.subTest(capabilities=capabilities), self.assertRaises(ValueError):
                self.load(declaration(capabilities=capabilities))

    def test_variant_vdd_ranges_and_local_three_amp_condition(self):
        """HI/HAI VDD retains the 2.7 V floor; the lower EN voltage is a separate fact."""
        for variant in setup.VARIANTS:
            minimum, maximum = (4500, 5500) if variant == "tusb322i" else (2700, 5000)
            for supply in (minimum, maximum):
                self.load(declaration(variant=variant, supply_mv=supply), variant)
            for supply in (minimum - 1, maximum + 1, True, float(minimum), "3500", None):
                with self.subTest(variant=variant, supply=supply), self.assertRaises(ValueError):
                    self.load(declaration(variant=variant, supply_mv=supply), variant)
            if variant != "tusb322i":
                with self.subTest(variant=variant), self.assertRaises(ValueError):
                    self.load(declaration(variant=variant, supply_mv=3499), variant,
                              ("role-current",))
                self.load(declaration(variant=variant, supply_mv=3500), variant, ("role-current",))


class ConsoleTests(unittest.TestCase):
    """Exercise strict operator evidence, absent input and every finite-input boundary."""

    def setUp(self):
        """Create a bounded transition, fake TTY, output buffer and virtual time."""
        self.step = dict(id="flip-cc1", instruction="Observe CC1 attachment",
                         partner="partner-123", supply_mv=3500, fixture="bench-01")
        self.clock = Clock()
        self.stream = mock.Mock()
        self.stream.isatty.return_value = True
        self.output = io.StringIO()

    def run_transition(self, chunks, deadline=101.0):
        """Start at an empty input boundary, then feed chunks through production checks."""
        with mock.patch.object(setup, "_read_console", side_effect=[None, *chunks]):
            return setup.console_transition(self.step, deadline, clock=self.clock,
                                            sleep=self.clock.sleep, stream=self.stream,
                                            output=self.output)

    def test_ready_skip_eof_and_exact_lines(self):
        """Only exact ready plus a complete newline attests; skip/EOF never produce proof."""
        for chunks, expected in (([b"ready\n"], True), ([b"re", b"ady\r", b"\n"], True),
                                 ([b"skip\n"], False), ([b""], False)):
            with self.subTest(chunks=chunks):
                self.assertIs(self.run_transition(chunks), expected)
        prompt = self.output.getvalue()
        for expected in ("bench-01", "partner-123", "3500mV", "same partner", "board/target",
                         "observed physical state"):
            self.assertIn(expected, prompt)

    def test_malformed_poison_and_no_echo(self):
        """Overflow, controls, multiple commands and plausible prefixes cannot attest."""
        for chunks in ([b"READY\n"], [b" ready\n"], [b"ready \n"], [b"ready\nready\n"],
                       [b"ready\rskip\n"], [b"\x1bready\n"], [b"\xff"], [b"\tready\n"],
                       [b"a" * 64, b"ready\n"], [b"a" * 65], ["ready\n"], [b"\n"]):
            with self.subTest(chunks=chunks), self.assertRaises(ValueError):
                self.run_transition(chunks)
        self.assertNotIn("READY", self.output.getvalue())

    def test_optional_plan_expectations_are_bounded_and_displayed(self):
        """Accept production physical-step expectations without admitting arbitrary fields."""
        self.step["expect"] = {"attachment": "LOCAL_SINK", "orientation": "CC1", "irq": "1"}
        self.assertTrue(self.run_transition([b"ready\n"]))
        self.assertIn("attachment=LOCAL_SINK, orientation=CC1, irq=1", self.output.getvalue())
        for expected in ([], None, {"unknown": "x"}, {"attachment": {}}, {"irq": True},
                         {"orientation": "CC1\n"}, {"accessory": "x" * 65}):
            self.step["expect"] = expected
            with self.subTest(expected=expected), self.assertRaises(ValueError):
                self.run_transition([])

    def test_prequeued_lines_cannot_attest_later_physical_steps(self):
        """A second queued ready fails the next input boundary without printing its prompt."""
        with mock.patch.object(setup, "_read_console",
                               side_effect=[None, b"ready\n", b"ready\n"]) as reader:
            kwargs = dict(clock=self.clock, sleep=self.clock.sleep,
                          stream=self.stream, output=self.output)
            self.assertTrue(setup.console_transition(self.step, 101.0, **kwargs))
            with self.assertRaisesRegex(ValueError, "empty before"):
                setup.console_transition(self.step, 101.0, **kwargs)
            self.assertEqual(reader.call_count, 3)
        self.assertEqual(self.output.getvalue().count("Fixture bench-01 step"), 1)
        for pending in (b"r", b"skip\n", b"ready\n", b"\x1b", b"x" * 64):
            self.output = io.StringIO()
            kwargs["output"] = self.output
            with mock.patch.object(setup, "_read_console", return_value=pending) as reader:
                with self.subTest(pending=pending), self.assertRaisesRegex(ValueError,
                                                                          "empty before"):
                    setup.console_transition(self.step, 101.0, **kwargs)
                reader.assert_called_once()
                self.assertEqual(self.output.getvalue(), "")

    def test_eof_at_input_boundary_is_missing_evidence(self):
        """EOF before prompting skips the step without accepting an operator observation."""
        with mock.patch.object(setup, "_read_console", return_value=b""):
            self.assertFalse(setup.console_transition(
                self.step, 101.0, clock=self.clock, sleep=self.clock.sleep,
                stream=self.stream, output=self.output))
        self.assertEqual(self.output.getvalue(), "")

    def test_no_tty_and_interrupt(self):
        """Piped input is missing evidence; Ctrl-C is preserved for owner cancellation."""
        self.stream.isatty.return_value = False
        with mock.patch.object(setup, "_read_console") as reader:
            self.assertFalse(self.run_transition([]))
            reader.assert_not_called()
        self.assertEqual(self.output.getvalue(), "")
        self.stream.isatty.return_value = True
        with self.assertRaises(KeyboardInterrupt):
            self.run_transition([b"\x03"])

    def test_absolute_deadline_empty_input_and_late_ready(self):
        """Each wait is at most 50 ms and late ready cannot satisfy the immutable deadline."""
        with mock.patch.object(setup, "_read_console", return_value=None):
            with self.assertRaises(TimeoutError):
                setup.console_transition(self.step, 100.12, clock=self.clock,
                                         sleep=self.clock.sleep, stream=self.stream,
                                         output=self.output)
        self.assertTrue(self.clock.waits)
        self.assertTrue(all(0 < wait <= 0.05 for wait in self.clock.waits))
        self.assertAlmostEqual(sum(self.clock.waits), 0.12)

        def late_ready(stream):
            """Simulate an input call completing at the deadline with a valid-looking line."""
            self.clock.now = 102.0
            return b"ready\n"

        with mock.patch.object(setup, "_read_console", side_effect=late_ready):
            with self.assertRaises(TimeoutError):
                setup.console_transition(self.step, 102.0, clock=self.clock,
                                         sleep=self.clock.sleep, stream=self.stream,
                                         output=self.output)

    def test_preexpired_deadline_and_step_validation(self):
        """Reject malformed step declarations and invalid deadlines before console output."""
        for deadline in (100.0, 99.0):
            with self.subTest(deadline=deadline), self.assertRaises(TimeoutError):
                self.run_transition([], deadline)
        for deadline in (True, "101", None, float("nan"), float("inf"), 10 ** 400):
            with self.subTest(deadline=deadline), self.assertRaises(ValueError):
                self.run_transition([], deadline)
        for changes in ({"extra": "x"}, {"fixture": "../x"}, {"supply_mv": True},
                        {"instruction": "bad\ntext"}, {"partner": ""}):
            value = dict(self.step, **changes)
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                setup.console_transition(value, 101.0, clock=self.clock,
                                         stream=self.stream, output=self.output)
        self.assertEqual(self.output.getvalue(), "")

    def test_deadline_around_output_and_oversleep(self):
        """Late output and a scheduler returning late from sleep cannot authorize a step."""
        output = mock.Mock()

        def late_write(text):
            """Model a console sink consuming the rest of the transition budget."""
            self.clock.now = 101.0

        output.write.side_effect = late_write
        with mock.patch.object(setup, "_read_console", return_value=None):
            with self.assertRaises(TimeoutError):
                setup.console_transition(self.step, 101.0, clock=self.clock,
                                         stream=self.stream, output=output)
        output.flush.assert_not_called()
        self.clock.now = 100.0

        def oversleep(seconds):
            """Model delayed host scheduling without a real sleep."""
            self.clock.now = 102.0

        with mock.patch.object(setup, "_read_console", return_value=None):
            with self.assertRaises(TimeoutError):
                setup.console_transition(self.step, 101.0, clock=self.clock, sleep=oversleep,
                                         stream=self.stream, output=self.output)

    def test_invalid_clock_and_nonadvancing_wait(self):
        """Invalid clocks and nonadvancing sleeps fail after bounded work without wall waits."""
        for value in (float("nan"), float("inf"), True, "100", 10 ** 400):
            with self.subTest(clock=value), self.assertRaises(ValueError):
                setup.console_transition(self.step, 101.0, clock=lambda: value,
                                         stream=self.stream, output=self.output)
        with mock.patch.object(setup, "_read_console", return_value=None) as reader:
            with self.assertRaisesRegex(ValueError, "without monotonic clock advancement"):
                setup.console_transition(self.step, 101.0, clock=self.clock,
                                         sleep=lambda seconds: None,
                                         stream=self.stream, output=self.output)
            self.assertEqual(reader.call_count, 2)  # Empty boundary, then one input poll.

        def regress_during_read(stream):
            """Model a read returning after an invalid backward jump in the host clock."""
            self.clock.now -= 1
            return b"ready\n"

        with mock.patch.object(setup, "_read_console", side_effect=regress_during_read):
            with self.assertRaisesRegex(ValueError, "finite monotonic"):
                setup.console_transition(self.step, 101.0, clock=self.clock,
                                         sleep=self.clock.sleep, stream=self.stream,
                                         output=self.output)

    def test_windows_backend_bounds_and_normalizes_enter(self):
        """The Windows path never calls getwch without kbhit or reads over 64 characters."""
        backend = mock.Mock()
        stream = mock.Mock()
        stream.fileno.return_value = 7
        standard = mock.Mock()
        standard.fileno.return_value = 7
        with mock.patch.object(setup.os, "name", "nt"), \
                mock.patch.object(setup.sys, "stdin", standard), \
                mock.patch.dict(sys.modules, {"msvcrt": backend}):
            backend.kbhit.return_value = False
            self.assertIsNone(setup._read_console(stream))
            backend.getwch.assert_not_called()
            backend.kbhit.side_effect = [True] * 6 + [False]
            backend.getwch.side_effect = list("ready\r")
            self.assertEqual(setup._read_console(stream), b"ready\n")
            backend.kbhit.side_effect = None
            backend.kbhit.return_value = True
            backend.getwch.side_effect = None
            backend.getwch.return_value = "x"
            backend.getwch.reset_mock()
            self.assertEqual(setup._read_console(stream), b"x" * 64)
            self.assertEqual(backend.getwch.call_count, 64)
            backend.getwch.return_value = "é"
            with self.assertRaises(ValueError):
                setup._read_console(stream)
            stream.fileno.return_value = 8
            with self.assertRaises(ValueError):
                setup._read_console(stream)

    def test_posix_backend_select_and_bounded_os_read(self):
        """The POSIX path polls select with zero timeout and reads at most 64 ready bytes."""
        selector = mock.Mock()
        stream = mock.Mock()
        stream.fileno.return_value = 7
        with mock.patch.object(setup.os, "name", "posix"), \
                mock.patch.dict(sys.modules, {"select": selector}), \
                mock.patch.object(setup.os, "read", return_value=b"ready\n") as reader:
            selector.select.return_value = ([], [], [])
            self.assertIsNone(setup._read_console(stream))
            reader.assert_not_called()
            selector.select.assert_called_once_with([7], [], [], 0)
            selector.select.return_value = ([7], [], [])
            self.assertEqual(setup._read_console(stream), b"ready\n")
            reader.assert_called_once_with(7, 64)


if __name__ == "__main__":
    unittest.main()
