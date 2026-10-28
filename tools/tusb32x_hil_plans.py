"""Finite read-only CLI observation plans over the existing borrowed Session.

No serial opening, protocol parsing, device write, reset or retry lives here.
run_plan accepts injected monotonic seconds/sleep and saves bounded aggregate
evidence through Session.artifacts. Exceptions preserve failed/partial evidence;
PlanFailure exits 1 and PlanBlocked exits 2 in the command-line owner.
"""

from dataclasses import asdict, dataclass
import math
import time


BASE_VARIANTS = {"tusb320", "tusb320i"}
EXTENDED_VARIANTS = {"tusb320li", "tusb320hi", "tusb320lai", "tusb320hai"}
VARIANTS = BASE_VARIANTS | EXTENDED_VARIANTS | {"tusb322i"}
MAX_RUN_SECONDS = 86400
MAX_SEQUENCE = (1 << 64) - 1
CACHE_COMMANDS = (
    "version", "bus", "console", "capabilities", "variant", "addr", "sample",
    "freshness", "profile show", "profile validate", "irq", "drv", "diag", "xfer_stats",
)
MALFORMED_COMMANDS = ("bus extra", "mode nonsense", "reg -1", "read extra")


class PlanFailure(RuntimeError):
    """Missing/contradictory observation or unexpected outcome; CLI exit 1."""

    exit_code = 1


class PlanBlocked(RuntimeError):
    """A target/adapter prerequisite prevents complete evidence; CLI exit 2."""

    exit_code = 2


def _integer(value, name, minimum, maximum):
    """Validate a bounded integer without coercion, signs or boolean acceptance."""
    if type(value) is not int or not minimum <= value <= maximum:
        raise ValueError(f"{name} must be an integer in [{minimum},{maximum}]")


def _seconds(value, name, maximum):
    """Validate finite positive seconds before opening; compare before float conversion."""
    if (isinstance(value, bool) or not isinstance(value, (int, float)) or
            not 0 < value <= maximum or not math.isfinite(value)):
        raise ValueError(f"{name} must be finite seconds in (0,{maximum}]")


@dataclass(frozen=True)
class ObservationPlan:
    """Selected target, finite suites and host timing; all durations are milliseconds/seconds.

    Count includes the first immediate soak observation. Intervals are quiet
    gaps after completed observations; late work never causes catch-up bursts. The whole-run
    deadline includes handshake, reads, waits, result delivery and local cleanup.
    """

    variant: str = "tusb320lai"
    address: int = 0x47
    suites: tuple = ("readonly",)
    count: int = 10
    interval_ms: int = 1000
    duration_ms: int | None = None
    run_timeout: float = 60.0
    command_timeout: float = 5.0

    def __post_init__(self):
        """Reject invalid pairs, unbounded arithmetic and impossible schedules before I/O."""
        if not isinstance(self.variant, str) or self.variant not in VARIANTS:
            raise ValueError("select a supported explicit variant")
        _integer(self.address, "address", 0, 127)
        if self.address not in ({0x60, 0x61} if self.variant in BASE_VARIANTS else {0x47, 0x67}):
            raise ValueError("address does not match variant")
        if (not isinstance(self.suites, (tuple, list)) or not self.suites or
                any(not isinstance(suite, str) or suite not in {"readonly", "soak"}
                    for suite in self.suites)):
            raise ValueError("suites must select readonly and/or soak")
        object.__setattr__(self, "suites", tuple(dict.fromkeys(self.suites)))
        _integer(self.count, "count", 1, 100000)
        _integer(self.interval_ms, "interval_ms", 1, 3600000)
        _seconds(self.run_timeout, "run_timeout", MAX_RUN_SECONDS)
        _seconds(self.command_timeout, "command_timeout", 3600)
        if self.command_timeout > self.run_timeout:
            raise ValueError("command_timeout exceeds whole-run deadline")
        if self.duration_ms is not None:
            _integer(self.duration_ms, "duration_ms", 1, MAX_RUN_SECONDS * 1000)
            if self.duration_ms > self.run_timeout * 1000:
                raise ValueError("duration_ms exceeds whole-run deadline")
        span = (self.count - 1) * self.interval_ms
        if "soak" in self.suites and (span >= self.run_timeout * 1000 or
                                     self.duration_ms is not None and span >= self.duration_ms):
            raise ValueError("soak schedule leaves no time for its final observation")

    def describe(self):
        """Return a concrete bounded JSON plan; hardware stays NOT RUN until execution."""
        registers = list(range(0x0B))
        if self.variant != "tusb322i":
            registers.append(0x45)
        if self.variant not in BASE_VARIANTS:
            registers.append(0xA0)
        actions = [
            "hilmark <fresh>; version; bus; capabilities; job status; console; profile show",
            "compare selected variant/address; bind only when ended and idle",
            "job auto 0; color off and verbose 1 only when different from saved values",
            "init <remaining command ms>; job step 0; at most 32 job step 1; "
            "result; result take <owned>",
        ]
        if "readonly" in self.suites:
            actions.extend([
                "idle cached routes: " + "; ".join(CACHE_COMMANDS),
                "silent parser rejections: " + "; ".join(MALFORMED_COMMANDS),
                "stage different current category; profile validate; prove core profiles unchanged",
                "verify aliases: observe retained desired, or reject CONFIGURATION_UNKNOWN",
                "profile read; read; sample; job auto 1; read under cached queries; job auto 0",
            ])
        actions.append("one ordinary reg command per address: " +
                       ",".join(f"0x{register:02X}" for register in registers))
        if "soak" in self.suites:
            actions.append(f"soak: {self.count} automatic read/sample operations; "
                           f"{self.interval_ms} ms quiet gaps")
        actions.append("restore saved local staging/auto/output; hilmark <fresh>")
        return dict(asdict(self), suites=list(self.suites), registers=registers,
                    hardware="NOT RUN", actions=actions)


def _payload(response, kind, source=None):
    """Select one required typed payload; Session already validated wire schemas/correlation."""
    records = [row for row in response.records if row["type"] == kind and
               (source is None or row.get("source") == source)]
    if len(records) != 1:
        raise PlanFailure(f"expected one {kind} payload")
    return records[0]


def _profile_values(response, include_staged=False):
    """Copy core profile evidence independently of request IDs and CLI staging."""
    return [{key: value for key, value in row.items() if key not in {"nonce", "request"}}
            for row in response.records if row["type"] in {"CONFIG", "CONFIG_DETAIL", "PROFILE"}
            and (include_staged or row.get("source") != "staged")]


def run_plan(session, plan, expected=None, clock=time.monotonic, sleep=time.sleep):
    """Run complete readonly/soak plans, save checks, restore local values and close framing.

    The caller owns I/O teardown. On failure, only verified local restoration is
    attempted, under the original whole-run deadline; no hardware rollback or
    result-take retry occurs. Fixed aggregates replace sample/transcript journals.
    """
    artifacts = session.artifacts
    started = clock()
    deadline = started + plan.run_timeout
    stage_deadline = deadline
    artifacts.metadata["plan"] = plan.describe()
    artifacts.metadata["observations"] = {"count": 0, "detached": 0, "last": None}
    for suite in plan.suites:
        artifacts.check(suite, "NOT RUN", "observation plan not completed")
    artifacts.check("register_observation", "NOT RUN", "allowlisted reads not completed")
    artifacts.checkpoint()
    last_time = started
    last_statistics = None

    def command_deadline():
        """Clip every exchange to the immutable run bound and check injected clock progress."""
        nonlocal last_time
        now = clock()
        if not math.isfinite(now) or now < last_time or now >= stage_deadline:
            raise PlanFailure("whole-run deadline expired or monotonic clock regressed")
        last_time = now
        return min(stage_deadline, now + plan.command_timeout)

    def command(text, code="OK", end_by=None):
        """Check delivery status without treating a failed payload as successful hardware work."""
        response = session.command(text, min(command_deadline(), end_by or deadline))
        if response.end is None or response.end["code"] != code:
            status = response.ack if response.end is None else response.end
            raise PlanFailure(f"{text}: {status['code']} detail={status['detail']}")
        return response

    def statistics():
        """Use all-source callback counts; saturation cannot support exact deltas."""
        nonlocal last_statistics
        row = _payload(command("xfer_stats"), "XFER")
        if row["source"] != "all_callbacks" or row["saturated"] != "0":
            raise PlanFailure("exact all-source counters unavailable/saturated")
        values = tuple(int(row[key]) for key in ("attempts", "success", "failures"))
        if (any(value == (1 << 32) - 1 for value in values) or
                values[0] != values[1] + values[2] or
                last_statistics is not None and any(
                    value < previous for value, previous in zip(values, last_statistics))):
            raise PlanFailure("callback totals regressed or lack exact consistent evidence")
        last_statistics = values
        return values

    def bus():
        """Require the same selected target and actual adapter callback-kind evidence."""
        row = _payload(command("bus"), "BUS")
        if row["variant"] != plan.variant or int(row["address"]) != plan.address:
            raise PlanBlocked("firmware selected target differs from requested variant/address")
        if row["adapter_available"] != "1":
            raise PlanBlocked("adapter counters required to prove no data-write callbacks")
        if int(row["adapter_writes"]) == (1 << 32) - 1:
            raise PlanBlocked("saturated adapter write counter cannot prove no writes")
        return row

    def no_writes():
        """Distinguish pointer/read callbacks from data writes and physical transactions."""
        if bus()["adapter_writes"] != initial_bus["adapter_writes"]:
            raise PlanFailure("unexpected data-write callback during observation")

    def operation(text, automatic=False):
        """Admit once, advance finite budgets, verify evidence, query and take exactly once."""
        end_by = command_deadline()
        wire_text = text
        if text in {"init", "profile read", "verify", "selfcheck", "selftest"}:
            # Manual budget/counter exchanges consume serial time too. Use the
            # existing optional owner timeout rather than its 1000 ms default;
            # immutable host/run deadlines remain independently enforced.
            timeout_ms = max(1, math.ceil((end_by - clock()) * 1000))
            wire_text += " " + str(timeout_ms)
        admitted = session.command(wire_text, end_by)
        if not admitted.admitted:
            raise PlanFailure(f"{text} admission: {admitted.ack['code']}")
        before = statistics()
        if not automatic:
            zero = _payload(command("job step 0", end_by=end_by), "JOB")
            if statistics() != before or zero["last_poll_callbacks"] != "0":
                raise PlanFailure("zero callback budget performed transport")
        for turn in range(32):
            if session.operation.complete:
                break
            previous = statistics()
            response = command("sample" if automatic else "job step 1", end_by=end_by)
            current = statistics()
            if automatic:
                # More owner turns can run while serial output drains. END still
                # reports zero handler callbacks; native tests prove each turn.
                if response.end["callbacks"] != "0":
                    raise PlanFailure("cached query performed handler callbacks")
            elif not 0 <= current[0] - previous[0] <= 1:
                raise PlanFailure("one callback budget exceeded")
        if not session.operation.complete:
            raise PlanFailure("operation exceeded 32 bounded service exchanges")
        result = session.wait_operation(min(end_by, command_deadline()))
        copied = command("result", end_by=end_by)
        if _payload(copied, "RESULT") != dict(result, request=copied.ack["request"],
                                            nonce=copied.nonce):
            raise PlanFailure("retained query changed original result")
        taken = session.take_result(min(end_by, command_deadline()))
        if taken.end["code"] != "OK":
            raise PlanFailure("result take rejected; owned slot retained, no automatic retry")
        if result["code"] != "OK" or result["effect"] != "NONE":
            raise PlanFailure(f"{text} payload: {result['code']} effect={result['effect']}")
        no_writes()
        return result

    def observe():
        """Require current valid fields; detached is useful, historical time alone is not."""
        before = _payload(command("sample"), "PORT")
        # read/status has a fixed 1000 ms owner deadline. Automatic finite polls
        # avoid spending that deadline on manual serial budget/counter exchanges.
        # Initialization separately proves explicit zero/one callback budgets.
        command("job auto 1")
        operation("read", automatic=True)
        command("job auto 0")
        response = command("sample")
        port = _payload(response, "PORT")
        detail = _payload(response, "PORT_DETAIL")
        roles = {"DETACHED": ("NOT_APPLICABLE", "NOT_APPLICABLE"),
                 "ACCESSORY": ("NOT_APPLICABLE", "NOT_APPLICABLE"),
                 "LOCAL_SOURCE": ("SOURCE", "SINK"), "LOCAL_SINK": ("SINK", "SOURCE")}
        if (port["invalidated"] != "0" or port["observed_time_valid"] != "1" or
                int(port["raw_valid_mask"]) != 3 or int(port["valid_fields"]) & 7 != 7 or
                port["raw08"] == "unknown" or port["raw09"] == "unknown" or
                any(port[key] == "UNKNOWN" for key in ("attachment", "local_role", "peer_role")) or
                roles.get(port["attachment"]) != (port["local_role"], port["peer_role"]) or
                detail["coherence"] not in {"BOOKENDS_MATCH", "CHANGED_DURING_READ"} or
                int(port["sequence"]) <= int(before["sequence"]) or
                int(port["sequence"]) == MAX_SEQUENCE or
                before["observed_time_valid"] == "1" and
                (int(port["observed_ms"]) - int(before["observed_ms"])) % (1 << 64) >= (1 << 63)):
            raise PlanFailure("port publication lacks advancing valid observation evidence")
        evidence = artifacts.metadata["observations"]
        evidence["count"] += 1
        evidence["detached"] += int(port["attachment"] == "DETACHED")
        evidence["last"] = {"port": port, "detail": detail}
        artifacts.checkpoint()

    try:
        session.begin(expected, command_deadline())
        initial_bus = bus()
        local = artifacts.metadata["local_before"]
        if local["auto"] != "0":
            command("job auto 0")
        if local["console"]["color"] != "0":
            command("color off")
        if local["console"]["verbose"] != "1":
            command("verbose 1")
        if initial_bus["bound"] == "0":
            command("bind")
        operation("init")
        if "readonly" in plan.suites:
            before = statistics()
            for text in CACHE_COMMANDS:
                command(text)
            snapshot = command("profile show")
            for text in MALFORMED_COMMANDS:
                rejected = command(text, "INVALID_PARAM")
                if rejected.command != "invalid" or rejected.end["callbacks"] != "0":
                    raise PlanFailure("malformed input did not produce a silent parser rejection")
            if statistics() != before or _profile_values(command("profile show"), True) != (
                    _profile_values(snapshot, True)):
                raise PlanFailure("cached/rejected command changed core state or counters")
            staged = _payload(snapshot, "PROFILE", "staged")
            before = statistics()
            current = "1a5" if staged["current"] == "default" else "default"
            command("current " + current)
            command("profile validate")
            if _profile_values(command("profile show")) != _profile_values(snapshot):
                raise PlanFailure("CLI staging changed retained core profiles")
            has_desired = _payload(snapshot, "CONFIG")["has_desired"] == "1"
            if statistics() != before:
                raise PlanFailure("local staging/validation performed transport")
            for alias in ("verify", "selfcheck", "selftest"):
                if has_desired:
                    operation(alias)
                else:
                    command(alias, "CONFIGURATION_UNKNOWN")
                    if statistics() != before:
                        raise PlanFailure("missing desired-profile rejection performed transport")
            operation("profile read")
            observe()
            command("job auto 1")
            operation("read", automatic=True)
            command("job auto 0")
            artifacts.check("readonly", "PASS", "local fences, budgets, results and freshness")
        for register in plan.describe()["registers"]:
            before = statistics()
            response = command(f"reg 0x{register:02X}")
            after = statistics()
            if after != (before[0] + 1, before[1] + 1, before[2]):
                raise PlanFailure("raw observation did not count exactly one successful callback")
            raw = _payload(response, "REG")
            if raw["variant"] != plan.variant or int(raw["address"]) != register:
                raise PlanFailure("raw observation changed target/register attribution")
            no_writes()
        artifacts.check("register_observation", "PASS", "finite mapped ordinary reg reads")
        if "soak" in plan.suites:
            soak_started = clock()
            soak_deadline = min(deadline, soak_started + (plan.duration_ms / 1000
                                if plan.duration_ms is not None else plan.run_timeout))
            stage_deadline = soak_deadline
            for index in range(plan.count):
                if index:
                    due = clock() + plan.interval_ms / 1000
                    while clock() < due:
                        command_deadline()
                        remaining = min(due, soak_deadline) - clock()
                        if remaining <= 0:
                            raise PlanFailure("soak duration expired")
                        before_sleep = clock()
                        sleep(min(0.05, remaining))
                        if clock() <= before_sleep:
                            raise PlanFailure("soak wait did not advance monotonic time")
                        if clock() >= soak_deadline:
                            raise PlanFailure("soak duration expired")
                observe()
                if clock() >= soak_deadline:
                    raise PlanFailure("soak duration expired during observation")
            artifacts.check("soak", "PASS", f"{plan.count} fresh observations; detached is valid")
        stage_deadline = deadline
        session.finish(command_deadline())
        command_deadline()
        artifacts.primary = "PASS"
        artifacts.checkpoint()
        command_deadline()
        return dict(artifacts.metadata["observations"])
    except BaseException as error:
        stage_deadline = deadline
        failed = artifacts.primary == "FAIL" or getattr(error, "exit_code", 2) == 1
        primary = "FAIL" if failed else "BLOCKED"
        artifacts.primary = primary
        artifacts.interrupted |= isinstance(error, KeyboardInterrupt)
        artifacts.check("plan", artifacts.primary, str(error) or type(error).__name__)
        if session.valid and session.operation is None:
            try:
                session.finish(command_deadline())
            except BaseException as cleanup_error:
                artifacts.cleanup = "FAIL"
                artifacts.interrupted |= isinstance(cleanup_error, KeyboardInterrupt)
                artifacts.metadata["cleanup_error"] = str(cleanup_error)[:1024]
                session.invalidate("local cleanup could not complete within verified plan")
        else:
            if artifacts.metadata.get("local_changed") or session.operation:
                artifacts.cleanup = "BLOCKED"
            session.invalidate("observation plan stopped; no uncertain result take/restore retry")
        artifacts.primary = primary
        artifacts.checkpoint()
        raise
