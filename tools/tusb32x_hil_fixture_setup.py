"""Validate physical fixture declarations and obtain finite operator attestations.

The JSON file documents application-selected power, enable polarity, ADDR straps,
exact wiring (including VBUS_DET), partner identity and operator assumptions. It
does not authorize board power changes or establish electrical safety by parsing
prose. Optional readiness flags remain operator declarations, not measurements.
No serial port, board control, background thread or blocking console read is used.
"""

import json
import math
import os
from pathlib import Path
import re
import sys
import time


FIXTURE_SUITES = ("orientation", "accessory", "role-current", "interrupt",
                  "reset-termination", "fault", "console")
CAPABILITIES = (
    "orientation", "audio", "debug", "charge-through", "debug-local-dfp",
    "debug-local-ufp", "role-current", "interrupt", "reset-termination", "fault", "console",
)
AUTHORIZED_ACTIONS = {"reset", "recover-dead-battery", "nack", "disconnect", "power-cycle",
                      "console-backpressure", "console-reconnect", "console-reboot"}
FAULT_CODES = {"I2C_ERROR", "I2C_NACK_ADDR", "I2C_NACK_DATA", "I2C_TIMEOUT", "I2C_BUSY",
               "I2C_BUS"}
VARIANTS = (
    "tusb320", "tusb320i", "tusb320li", "tusb320hi", "tusb320lai", "tusb320hai",
    "tusb322i",
)
MAX_SETUP_BYTES = 16 * 1024
REQUIRED = {
    "board", "part", "adapter", "wiring", "addr_strap", "enable", "supply_mv",
    "partner", "operator", "variant", "capabilities",
}
OPTIONAL = {"startup_ready", "base_recovery_complete", "stable_vdd", "source_startup",
            "authorized_actions", "nack_code", "disconnect_code", "target_vdd_continuous"}


def _text(value, name, maximum=256):
    """Require nonempty printable ASCII, bounded before it can reach a prompt."""
    if (not isinstance(value, str) or not 1 <= len(value) <= maximum or
            not value.strip() or any(not 32 <= ord(char) <= 126 for char in value)):
        raise ValueError(f"{name} must be nonempty printable ASCII, at most {maximum} chars")
    return value


def _fixture_id(value):
    """Reject IDs unsuitable for bounded evidence names and console output."""
    if not isinstance(value, str) or re.fullmatch(r"[a-zA-Z0-9_.-]{1,64}", value) is None:
        raise ValueError("fixture ID must be 1..64 ASCII letters, digits, _, . or -")
    return value


def _object(pairs):
    """Reject duplicate JSON keys instead of accepting a last value silently."""
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate setup key: {key}")
        result[key] = value
    return result


def _constant(value):
    """Reject non-JSON NaN and infinity spellings accepted by Python's decoder."""
    raise ValueError(f"invalid JSON constant: {value}")


def load_setup(path, fixture_id, variant, suites):
    """Return a validated plain fixture dict, adding its separately supplied ID.

    Read at most 16 KiB plus one overflow byte from path. Missing/unknown/duplicate
    fields, invalid UTF-8, nonprintable text, wrong types, target mismatch and
    unsupported VDD raise ValueError before hardware is opened. All TUSB320 VDD
    ranges are 2700..5000 mV; HI/HAI's 1050..3650 mV limit describes EN, not VDD
    (TI SLLSEP2D/SLLSEQ8D sections 6.3 and 6.5). TUSB322I VDD is 4500..5500 mV.
    Role-current plans include local 3 A advertisement and require VDD >=3500 mV
    for TUSB320 devices. Enable wiring/polarity and VBUS_DET remain declared
    operator responsibilities. Capability absence is evaluated as NOT RUN by the
    plan owner; this loader does not turn an absent fixture into supported hardware.
    """
    _fixture_id(fixture_id)
    if not isinstance(variant, str) or variant not in VARIANTS:
        raise ValueError("select an explicit supported setup variant")
    allowed_suites = {"readonly", "soak", *FIXTURE_SUITES}
    if (not isinstance(suites, (list, tuple)) or not 1 <= len(suites) <= 9 or
            any(not isinstance(suite, str) or suite not in allowed_suites for suite in suites) or
            len(set(suites)) != len(suites)):
        raise ValueError("setup suites must be a bounded list of supported suite names")
    try:
        with Path(path).open("rb") as source:
            raw = source.read(MAX_SETUP_BYTES + 1)
    except (OSError, TypeError) as exc:
        raise ValueError(f"cannot read fixture setup: {exc}") from exc
    if len(raw) > MAX_SETUP_BYTES:
        raise ValueError("fixture setup exceeds 16 KiB")
    try:
        setup = json.loads(raw.decode("utf-8"), object_pairs_hook=_object,
                           parse_constant=_constant)
    except (UnicodeError, json.JSONDecodeError, RecursionError) as exc:
        raise ValueError(f"invalid fixture JSON: {exc}") from exc
    if not isinstance(setup, dict) or set(setup) - REQUIRED - OPTIONAL or REQUIRED - set(setup):
        raise ValueError("fixture setup requires exactly the documented setup fields")
    for name in REQUIRED - {"supply_mv", "capabilities"}:
        _text(setup[name], name, 1024 if name == "wiring" else 256)
    if setup["variant"] != variant:
        raise ValueError("fixture setup variant does not match selected target")
    if re.search(r"\bVBUS_DET\b", setup["wiring"]) is None:
        raise ValueError("wiring must explicitly document VBUS_DET")
    minimum, maximum = (4500, 5500) if variant == "tusb322i" else (2700, 5000)
    supply = setup["supply_mv"]
    if type(supply) is not int or not minimum <= supply <= maximum:
        raise ValueError(f"supply_mv must be integer VDD in [{minimum},{maximum}]")
    if "role-current" in suites and variant != "tusb322i" and supply < 3500:
        raise ValueError("local 3 A advertisement requires supply_mv >=3500")
    for name in {"startup_ready", "base_recovery_complete", "stable_vdd", "source_startup",
                 "target_vdd_continuous"} & set(setup):
        if type(setup[name]) is not bool:
            raise ValueError(f"{name} must be a boolean")
    actions = setup.get("authorized_actions", [])
    if (not isinstance(actions, list) or len(actions) > len(AUTHORIZED_ACTIONS) or
            any(not isinstance(action, str) or action not in AUTHORIZED_ACTIONS
                for action in actions) or len(set(actions)) != len(actions)):
        raise ValueError("authorized_actions must be a unique list of documented actions")
    for name in ("nack_code", "disconnect_code"):
        if name in setup and (not isinstance(setup[name], str) or setup[name] not in FAULT_CODES):
            raise ValueError(name + " must name an exact documented I2C_* terminal status")
    capabilities = setup["capabilities"]
    if (not isinstance(capabilities, list) or len(capabilities) > len(CAPABILITIES) or
            any(not isinstance(value, str) or value not in CAPABILITIES
                for value in capabilities) or len(set(capabilities)) != len(capabilities)):
        raise ValueError("capabilities must be a unique list of supported fixture capabilities")
    setup["id"] = fixture_id
    return setup


def _read_console(stream):
    """Read at most 64 available console bytes, returning None when none are ready.

    Windows uses console key availability; POSIX select is nonblocking before
    os.read. This private seam lets tests inject finite input without real sleeps.
    Windows getwch does not echo input; neither path copies input into evidence.
    """
    if os.name == "nt":
        import msvcrt

        if stream.fileno() != sys.stdin.fileno():
            raise ValueError("Windows operator input must use the process console")
        chars = []
        for _ in range(64):
            if not msvcrt.kbhit():
                break
            chars.append(msvcrt.getwch())
        try:
            return "".join(chars).replace("\r", "\n").encode("ascii") if chars else None
        except UnicodeError as exc:
            raise ValueError("operator response must be ASCII") from exc
    import select

    descriptor = stream.fileno()
    readable, _, _ = select.select([descriptor], [], [], 0)
    return os.read(descriptor, 64) if readable else None


def console_transition(step, deadline, *, clock=time.monotonic, sleep=time.sleep,
                       stream=None, output=None):
    """Obtain one exact ready/skip line from a TTY within an absolute deadline.

    step is a plain dict with id, instruction, partner, supply_mv and fixture ID.
    Its optional expect dict holds at most six named physical/firmware comparisons,
    with bounded printable values; these are shown to the operator, not interpreted
    as measurements. The plan owner compares them with subsequent device evidence.
    True means the operator typed exactly ready followed by LF or CRLF (Windows
    console Enter is normalized to LF). It attests the displayed physical state on the
    supplied board/target with the same partner and selected supply. False means
    skip, EOF or no TTY; it is missing evidence, never a successful observation.
    Invalid input raises ValueError, expiry raises TimeoutError, and Ctrl-C raises
    KeyboardInterrupt. Input is at most 64 characters, no ANSI is accepted, waits
    are at most 50 ms, and the deadline is checked around every I/O and wait.
    A nonfinite/regressing clock or a sleep that returns without clock advancement
    raises ValueError rather than allowing an unbounded input loop.
    Input must be empty before the prompt is printed. Any queued input raises
    ValueError, preventing an earlier pretyped ready from attesting a later step.
    Console output is bounded but assumes the caller's local output sink returns.
    """
    required = {"id", "instruction", "partner", "supply_mv", "fixture"}
    if (type(step) is not dict or required - set(step) or
            set(step) - required - {"expect"}):
        raise ValueError("operator step requires id, instruction, partner, supply_mv and fixture")
    for name in ("id", "partner", "instruction"):
        _text(step[name], name, 1024 if name == "instruction" else 256)
    _fixture_id(step["fixture"])
    if type(step["supply_mv"]) is not int or not 1 <= step["supply_mv"] <= 5500:
        raise ValueError("operator supply_mv must be a bounded positive integer")
    expected = step.get("expect", {})
    expected_keys = {"attachment", "orientation", "detected_current", "advertised_current",
                     "accessory", "irq"}
    if type(expected) is not dict or set(expected) - expected_keys:
        raise ValueError("operator expect must contain only documented observation fields")
    for name, value in expected.items():
        _text(value, "expected " + name, 64)
    try:
        valid_deadline = (not isinstance(deadline, bool) and
                          isinstance(deadline, (int, float)) and math.isfinite(deadline))
    except OverflowError:
        valid_deadline = False
    if not valid_deadline:
        raise ValueError("operator deadline must be finite monotonic seconds")
    previous_time = None

    def before_deadline():
        """Check finite monotonic time and return remaining seconds before any next action."""
        nonlocal previous_time
        now = clock()
        try:
            valid_time = (not isinstance(now, bool) and isinstance(now, (int, float)) and
                          math.isfinite(now))
        except OverflowError:
            valid_time = False
        if not valid_time or previous_time is not None and now < previous_time:
            raise ValueError("operator clock must return finite monotonic seconds")
        previous_time = now
        remaining = deadline - now
        if not math.isfinite(remaining):
            raise ValueError("operator clock distance to deadline must be finite")
        if remaining <= 0:
            raise TimeoutError("operator transition deadline expired")
        return remaining

    before_deadline()
    stream = sys.stdin if stream is None else stream
    output = sys.stdout if output is None else output
    if not stream.isatty():
        before_deadline()
        return False
    before_deadline()
    pending = _read_console(stream)
    before_deadline()
    if pending is not None:
        if pending == b"":
            return False
        raise ValueError("operator input must be empty before the transition prompt")
    prompt = (
        f"Fixture {step['fixture']} step {step['id']}: {step['instruction']}\n"
        f"Partner={step['partner']} VDD={step['supply_mv']}mV. Type ready then Enter only after "
        "checking the supplied board/target, same partner, selected supply and observed physical "
        "state; type skip to omit this step.\n"
    )
    if expected:
        comparisons = ", ".join(f"{key}={value}" for key, value in expected.items())
        prompt += "Expected: " + comparisons + "\n"
    output.write(prompt)
    before_deadline()
    output.flush()
    before_deadline()
    line = bytearray()
    carriage_return = False
    while True:
        before_deadline()
        chunk = _read_console(stream)
        remaining = before_deadline()
        if chunk is None:
            sleep(min(0.05, remaining))
            if before_deadline() >= remaining:
                raise ValueError("operator sleep returned without monotonic clock advancement")
            continue
        if not isinstance(chunk, bytes) or len(chunk) > 64:
            raise ValueError("invalid bounded console read")
        if not chunk:
            return False
        for index, char in enumerate(chunk):
            if char == 3:
                raise KeyboardInterrupt
            if carriage_return and char != 10:
                raise ValueError("operator CR must be followed by LF")
            if char == 13:
                carriage_return = True
                continue
            if char == 10:
                # Reject trailing commands in a single read instead of silently
                # treating them as a fresh attestation for another transition.
                suffix = chunk[index + 1:]
                if suffix:
                    raise ValueError("unexpected input after operator response")
                if line == b"ready":
                    return True
                if line == b"skip":
                    return False
                raise ValueError("operator response must be exactly ready or skip")
            if not 32 <= char <= 126 or len(line) >= 64:
                raise ValueError("operator response is malformed or exceeds 64 characters")
            line.append(char)
