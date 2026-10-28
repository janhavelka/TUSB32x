# Prompt 28 current peer audit

Read-only source review on 2026-10-28 for the strict Python session/evidence
layer. This review owns only this report. No sibling writes, builds, hardware
access, remote CI inspection or Git mutations were performed.

## Revision and instruction evidence

`git rev-parse HEAD` and `git status --short` were read independently in each
checkout before this report was written. Every listed worktree was clean.

| Checkout under `C:/Users/Honza/Documents/Projects/` | Full HEAD |
| --- | --- |
| TUSB32x | `19fcd2c37837ef823fd852581d4cfe42b6bb8d5c` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| 24Cxx | `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` |

TUSB32x remains on `promptset/tusb32x-implementation-20261023`. LDC1614 has
advanced beyond the older `c0ab4c1` revision cited in CLI_CONTRACT; the findings
below use its current files. This agent did not independently fetch or verify
upstream equality; the lead owns synchronization.

Read `../AGENTS.md`, local `AGENTS.md`, `docs/prompts/README.md`, `COMMON.md`,
`AGENT_WORKFLOW.md`, full `28-python-session.md`, the runner/machine-record
sections of `CLI_CONTRACT.md`, `MAINTAINABILITY.md`, current progress/decisions,
and each peer's root `AGENTS.md`. Peer rules support framework-neutral cores,
explicit owner state, bounded callbacks and honest hardware evidence. TUSB32x's
prompt-34-only remote CI exception governs this execution.

## Sources and decisions

Peer paths below are relative to their named sibling root. Source inspection is
evidence of a convention, not proof that its tests pass in this environment.

### TMP1x2: deadline, marker and restoration behavior

Read `tools/hil_tmp1x2_runner.py`, its `SerialSession`, `EvidenceFiles`,
`Runner.restore`, `Runner.checkpoint` and reporting paths; inspect corresponding
fault fixtures in `tools/test_hil_tmp1x2_runner.py`. Revisited declarations in
`include/TMP1x2/TMP1x2.h` and marker/owner/version implementation in
`examples/common/Tmp1x2Cli.cpp` and `Tmp1x2CliOutput.cpp`.

- Adopt one command deadline derived from the injected monotonic clock. It
  includes draining pending output, command write, response and fresh marker.
  Reads and write timeouts receive remaining time; late reads fail. Do not use
  `serial.flush()`, which the source explicitly excludes because drain can block.
- Adopt fresh random markers, exact marker parsing, short-write rejection,
  partial-evidence retention and refusal to send more bytes after framing loss
  or restart. Relevant tests cover split markers/ANSI/CRLF, contaminated markers,
  stale prompts, logging time before write and late responses.
- Strengthen checks before and after every I/O and evidence write: peer marker
  and per-command timeout mechanics are a pattern, not a complete oracle for
  prompt 28's late-success rule. Validate finite deadlines before opening.
- Reject the peer's human prompt/status grammar and uint32 token wrap test as
  TUSB protocol rules. TUSB uses exact schema-1 records, uint64 input IDs,
  origin requests and instance tokens, with exhaustion rather than ID reuse.
- Preserve its useful separation of disk streaming and bounded retained output,
  but reject the 8 MiB response/16 MiB retained-text allowances. TUSB defaults are
  256 KiB response, 32 MiB disk transcript and 64 KiB fixed retained tail, with
  finite override ceilings. Do not keep each response body again in report rows.
- Adopt checkpoint-before-mutation and cleanup refusal after lost framing or
  restart. Reject hardware restoration via `restore_commands`, recovery and
  snapshot verification: TUSB cleanup restores captured local staging, auto and
  presentation only. It never applies or recovers a hardware profile implicitly.
  Preserve primary failure even if that local cleanup succeeds.

`CMakeLists.txt` registers `hil_runner` with the actual built CLI fixture and a
180-second timeout. `.github/workflows/ci.yml` runs CMake/CTest with strict
warnings and ASan/UBSan. Adopt executable registration and a finite timeout;
these checked-in definitions are not current remote-run evidence.

### 24Cxx: identity, artifacts, guards and live transcript registration

Read full `tools/hil_identity.py`, `hil_guards.py`, relevant
`hil_artifacts.py` provenance/rendering code, and their three `test_hil_*.py`
test files. Revisited `tools/hil_runner.py` session/framing, cleanup and atomic
report paths; `tools/test_cli_transcript.py`; `include/EEPROM24Cxx/EEPROM24Cxx.h`;
and `examples/common/Eeprom24CxxCli.cpp` admission/completion/result handling.

- Adopt identity parsing independently from port access and host provenance.
  The parser preserves unknown firmware fields; explicit expectations cannot
  pass against unknown identity. Revision-prefix comparison preserves the
  observed abbreviation and cannot prove unseen commit digits.
- Adopt host source hashes, argv boundaries, Python/OS/tool versions and
  explicit unknown Git state. `hil_artifacts._git_text` uses an argument list,
  fixed repository root and subprocess timeout. Its 64 KiB output check occurs
  after `capture_output`; do not describe that check as a streaming RAM bound.
- Adopt exact required rejection evidence and independent before/after
  counters/state checks. An expected error string alone does not establish
  bus silence. Reject EEPROM capacity, scratch-window, backup, erase and
  programming policies from TUSB session design.
- Adopt line-by-line incremental parsing rather than repeatedly scanning an
  entire fragmented response. Strengthen the deadline checks: `Session.command`
  bounds its loop/read timeout but lacks a post-read clock rejection before its
  marker-success return, and `write` logs after calculating remaining time.
  TUSB must recheck after logging/I/O and reject a success arriving too late.
- Adopt refusal to cancel foreign/preexisting work or send cleanup bytes when a
  partial command might remain in the device console. Keep TUSB's explicit
  admitted-token ownership and exact `result take` instead of copying the
  EEPROM CLI's internal job consumption and additional session envelope.
- Adopt same-directory temporary report replacement and durable checkpoints.
  The peer publishes Markdown before authoritative JSON and downgrades capture
  failures; TUSB should likewise avoid publishing a new PASS after sidecar
  failure. Keep primary, cleanup and residual uncertainty as separate fields.

`CMakeLists.txt` registers Python `hil_runner` discovery and `cli_transcript`
against `$<TARGET_FILE:eeprom24cxx_cli_host>`. The transcript runs actual shared
CLI code, not hand-authored output; adopt that registration pattern. Unlike
TMP1x2, those peer tests have no explicit CTest timeout in the inspected block;
retain TUSB's existing 30-second transcript timeout and add a finite runner one.
The peer `.github/workflows/ci.yml` runs the native CMake/CTest checks with strict
warnings and sanitizers; no remote status was consulted.

### LDC1614: provenance, verdicts and capture limits

Read `tools/ldc1614_hil_runner.py` `TranscriptJournal`, `classify_command`,
`append_expectation_results`, `overall_status`, and optional-gate combination;
inspect relevant `tools/test_ldc1614_hil_runner.py` tests for correlated async
results, reset counting, firmware/binding contradictions and partial campaigns.
Revisited `include/LDC1614/LDC1614.h` job/result declarations and
`examples/common/Ldc1614Cli.cpp` implementation paths. Its CMake file performs
version checks for its component; `.github/workflows/ci.yml` directly runs
`python tools/test_ldc1614_hil_runner.py` under Python 3.11.

- Adopt exact admitted/terminal command/session matching, duplicate rejection,
  startup-banner detection and contradiction checks across firmware identity
  and configured address. Shared chip ID does not establish an exact variant.
- Adopt failure-dominant overall verdicts and explicit incompleteness: no
  commands/no usable transcript is NOT_RUN, and a partial matrix cannot PASS.
  Map TUSB to its required PASS/FAIL/NOT RUN/BLOCKED vocabulary and exit codes
  rather than copying LDC's UNKNOWN/REVIEW vocabulary.
- Reject generic OK and configurable expected-token fallback as completion
  proof. TUSB's RESULT payload determines operation success independently of
  successful `result.take` delivery. Query copies are not new completions.
- Reject host HEAD as a default statement of flashed identity. The peer compares
  actual firmware commit with explicit expected revision or host short HEAD;
  TUSB must keep host facts and firmware observations distinct in all reports.
- Reject `TranscriptJournal.parts` and `text()` as a memory model. Current code
  has a finite 128 MiB transcript cap but mirrors all accepted text in RAM and
  joins it again. A cap does not make it the required fixed 64 KiB tail. Disk
  cap hits and disk exceptions remain failed/incomplete evidence.
- Reject sensor modes, default profile replay and blanket confirmation policy;
  none establishes TUSB read-only initialization or local-only restoration.

## TUSB integration boundaries and required review focus

Revisited `include/TUSB32x/TUSB32x.h`,
`examples/common/Tusb32xCli.cpp`, `tools/tusb32x_cli_contract.py`,
`tools/test_cli_transcript.py` and `test/CMakeLists.txt`. The schema parser already
rejects duplicate/missing keys, non-ASCII and oversized machine records while
retaining valid additive fields. The session should reuse it and own byte
assembly, noise caps, temporal correlation and operation ownership separately.

The current real CLI emits MARK followed by the matching `hilmark` END; MARK is
the acknowledgement in place of ACK. Preserve that implemented exchange when
adding fresh begin/end frames. Do not equate the transcript helper's phrase
"MARK alone" with omission of the request's END.

Track increasing new-input IDs on ACK/MARK only. An older admitted operation can
finish after a newer query; save its original nonce/request/token and correlate
its spontaneous RESULT/END there. Reject a second original completion while
allowing explicitly requested result copies with the same origin/token. Foreign
active or retained work blocks a plan and is never cancelled/taken implicitly.

Prioritize fake-clock cases where writing, logging or final-marker reading
crosses the same immutable deadline; fragment/noise/capture limits; original
versus queried completion; delivery OK with failed result; clean local cleanup
after primary failure; and invalidated-session cleanup refusal. Artifact failure
must preserve a useful incomplete report without converting primary failure or
interruption to PASS. Hardware qualification remains NOT RUN.

The current checkout has no tracked `.github/` files or workflow directory
(`git ls-files '.github/*'` is empty). Do not claim a local workflow was inspected
or add prompt-34 workflow work here. Existing `cli_transcript` passes the built
host path through CMake and has a finite timeout; keep that registration while
adding the strict session exchange and separately registered `hil_runner` suite.
