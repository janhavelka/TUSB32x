# 28 — Implement strict Python sessions and evidence artifacts

Deliver the reusable, tested serial/session and evidence layer used by the later
runner suites. This prompt implements framing, deadlines, provenance and saved
outcomes; prompt 29 connects complete observation plans to the command-line tool.
Do not add stub suite functions or generic HIL orchestration infrastructure.

## Read and re-audit

Read [COMMON](COMMON.md), [AGENT_WORKFLOW](AGENT_WORKFLOW.md), the machine-record
and runner requirements in [CLI_CONTRACT](CLI_CONTRACT.md), and tooling/docs
guidance in [MAINTAINABILITY](MAINTAINABILITY.md). Prompts 20–22 must supply the
real shared CLI and persistent host fixture. Inspect their actual schemas and
tests; prompts 23–27 supply example identity and consumer evidence.

Re-audit TMP1x2 runner deadlines/markers/restoration, 24Cxx hil_identity.py,
hil_artifacts.py, hil_guards.py and transcript registration, and LDC1614 provenance
and verdict logic. Record current HEAD, dirty state, exact files and decisions.
Do not copy an EEPROM scratch policy or growing in-memory transcript journal.

## Implement the session layer

1. Create `tools/tusb32x_hil_runner.py` with ordinary functions and small data
   classes, using `tools/tusb32x_cli_contract.py`. Extract identity/artifact helpers
   only for a clear current responsibility. Use snake_case, pathlib, argparse
   where needed, unittest and argument-list subprocess calls with timeouts.
   Child Python uses sys.executable; never interpolate arguments into shell text.
2. Implement a transport/session interface with injected serial I/O and monotonic
   clock. Require an explicit port for hardware use. Validate finite command and
   write deadlines before opening, and configure/report DTR/RTS before open where
   supported, using on/off/unchanged. Document backend limits: opening a port can
   reset a board. No enumeration/probing, flashing, hidden reset or retry policy.
3. Parse exact required schemas, bounded ASCII values and duplicate/missing keys;
   record unknown extension fields without treating them as required evidence.
   Human OK/PASS and echoes are not evidence. Use fresh random begin/end nonces;
   accept exact MARK acknowledgements, request/command/token correlation and one
   original terminal RESULT/END pair per admitted operation. Query repetition is
   not another completion. Consume retained results explicitly through result
   take, preserving the difference between delivery OK and failed payload.
4. Require increasing new-input ACK/MARK request IDs while accepting an older
   admitted request's JOB/RESULT/END after a newer query response. Preserve origin
   request/token mapping. Own only tokens admitted by this run; foreign active or
   retained work is a blocked prerequisite, never implicitly cancelled/taken.
   Handle nonce=none, command=invalid and exhaustion without
   reuse. Reboot, reconnect, identity change, framing/output loss or regressed
   input IDs invalidate the session; re-establish evidence before further work.
5. Use one monotonic deadline across writes, fragments, markers and completion.
   Check before and after I/O; late success fails. Bound line bytes, response
   bytes/count and boot/ANSI noise. No new full timeout on each read. Reject wrong
   tokens, malformed/contradictory/duplicate/truncated records and short writes.
6. Keep host commit/dirty state separate from firmware library/version/commit/
   tree/build, framework/version and target. Unknown firmware identity cannot
   satisfy expected identity or mutation prerequisites. Preserve variant/address,
   trust/generations and port validity/invalidation as independent facts.
7. Stream timestamped TX/RX to disk with only a fixed tail and aggregate stats in
   RAM. Defaults: 256 KiB response, 32 MiB transcript, 64 KiB retained tail; give
   overrides finite ceilings. Cap hits make evidence failed/incomplete. Atomically
   replace report.json and summary.md using same-directory temporary files.
   Checkpoint before mutations and on exception/interruption.
8. Represent primary outcome, cleanup outcome and residual uncertainty separately.
   Track changed local staging/scheduler/output settings separately from device
   effects. Restore them only in the verified session, without hardware apply.
   Failed work plus good cleanup stays failed; missing required proof prevents
   PASS. Use per-check PASS/FAIL/NOT RUN/BLOCKED and exit conventions 0 complete
   PASS, 1 test/protocol failure, 2 invocation/missing-tool/incomplete, 130 saved
   interruption. Do not invent successful hardware evidence to exercise reporting.

## Tests and independent review

Add `tools/test_tusb32x_hil_runner.py` with fake serial and clock. Cover fragments,
CRLF/ANSI noise, stale marks, echo-only PASS, missing/duplicate/interleaved records,
wrong/reused tokens, request exhaustion, boot/identity changes, expired writes,
late replies, short writes, exceptions, missing ports, capture caps, atomic files
and Ctrl-C. Test failed result payload despite successful result delivery.
No wall-clock sleeps or pyserial dependency is needed for parser tests.

Extend the existing live C++ transcript test with a real marker/version/session
exchange; pass the built fixture path through CMake. Register hil_runner with a
finite timeout, retaining the existing cli_transcript registration. Add audited
pinned optional pyserial to `tools/requirements-hil.txt`; consult official package
documentation when choosing its version. Run parser/session and transcript tests.

Optional: delegate artifact failure tests or a read-only framing review to a
general coding agent; use stronger reasoning for correlation/deadline ambiguity.
Freeze record contracts and assign disjoint files. The lead owns integration/Git.

## Document and finish

Write module/function/exception docstrings and begin `docs/hil-runner.md` with
session schemas, limits, identity and artifact/verdict contracts. Update README,
CHANGELOG, validation and progress with the implemented layer and exact checks.
No unimplemented suite is advertised as runnable. Commit/sync coherent checked
work and the finished prompt on the existing execution branch, verify upstream
equality and report limits. Serial hardware proof remains separate from these
fake-serial and actual-C++-fixture tests.
