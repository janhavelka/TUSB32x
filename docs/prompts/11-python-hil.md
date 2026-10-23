# P11 — Build Python verification and hardware runners

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 and 24Cxx
live-CLI runner fixtures, LDC1614 provenance/deadline verdicts, INA228 transfer
assertions and MB85RC/RV3032 strict parsing. Inspect flaws as well as good patterns.

## Work

1. Add a named runner such as `tools/tusb32x_hil_runner.py`, standard-library
   parser/unit tests and an explicitly declared optional serial dependency.
   Provide `--help`, parser-only tests and explicit `--port`, baud, timeout,
   target/variant/address, expected firmware identity, suite and output directory.
   No auto-probing serial ports, flashing, board reset or hidden test selection.
   Explicitly configure/report DTR/RTS before open where the backend supports it,
   and document unavoidable open/reconnect toggles. Detect any resulting reboot
   and invalidate framing, pending results and cleanup eligibility; no universal
   no-reset guarantee may be inferred from merely avoiding a reset command.
2. Reuse current CLI grammar. Nonce markers correlate command boundaries; jobs
   need matching token and exactly one valid terminal result. Drain/record stale
   input without letting it certify a new command. Enforce deadlines before write
   and after read; late success remains a failure for that budget.
3. Validate required fields/counts and reject missing, duplicate, contradictory,
   malformed, truncated, restarted or out-of-order evidence. A prompt or echoed
   OK/PASS is not completion. Expected errors are scoped to their exact command.
4. Record host source revision/dirty state separately from firmware-reported
   revision/dirty/build/framework/target. Check expected identity and target before
   mutations; unknown identity cannot pass an identity-required suite. Revalidate
   after reset/reconnect; do not resume cleanup against an unchecked device.
5. Stream timestamped transcripts and write JSON plus concise Markdown summaries.
   Use bounded retained tails/aggregates, explicit run/suite verdicts, atomic
   checkpoints before mutations and recoverable partial evidence on interruption.
   Missing mandatory evidence or unrun selected suite prevents overall PASS.
6. Define read-only suites for metadata, bind/identify, status/cached getters,
   zero/one-transfer budgets, malformed input/state fences, and optional bounded
   polling soak. Poll reads count as activity; do not require a changing attachment
   value to pretend the device is alive. Prove freshness using sequences/timestamps.
7. Define separately selected fixture suites: both connector orientations; local
   sink/source/DRP; advertised current categories; supported accessories; INT
   assertion/clear/reassertion; mode/reset/termination sequence; NACK/disconnect/
   power-cycle invalidation; USB-console reconnect/backpressure; long-run fairness.
   Test attached-current changes as a measured unresolved behavior, not a guarantee.
8. Mutating suites capture profile, identity and attachment baseline and verify
   suitable restoration. Preserve failure/cleanup/uncertainty separately. Never
   replay ambiguous configuration writes or restore a power role after target or
   attachment assumptions changed. No automatic VBUS or supply manipulation.
9. Provide a `tools/validate.py` (or audited equivalent) selecting native tests,
   guards, builds and parser checks. Required skipped tools produce INCOMPLETE/
   non-success, not a green complete-validation result. HIL never runs implicitly.

## Tests and completion

- Unit-test fragmented/ANSI/CRLF/stale/duplicate/late/rebooted replies, changing
  firmware identity, serial exceptions, absent ports, timeout boundaries,
  transcript truncation, interruption and restoration outcomes.
- Run the runner parser against one persistent native instance of the **real**
  shared CLI and scripted device model; avoid testing only hand-written good logs.
- Add `docs/hil-runner.md` and `docs/hardware-validation.md` with exact commands,
  wiring/fixtures, expected observations and PASS/FAIL/NOT RUN criteria per variant.
- Hardware runs only with available authorized target information. Report host
  runner correctness separately from physical device/adapter/connector validation.
