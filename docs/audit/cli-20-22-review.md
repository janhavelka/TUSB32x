# Independent CLI review, 2026-10-28

Scope: completed prompts 20, 21 and 22 at `6d99a29` on
`promptset/tusb32x-implementation-20261023`. Entry was clean and equal to upstream
after fetch. Read original prompts in full, COMMON, CLI_CONTRACT, relevant
baseline/rules, maintainability and delegation instructions. No adapter or
prompt 23 implementation, sibling mutation or remote CI query is included.

Three independent reviewers covered console/parser/staging, managed operations
and retained results, and diagnostic/host-parity behavior. The lead reviewed
actual current code and the implementation diff from `c07e4e5`, verified returned
findings and ran before-fix probes. Exact final commands are in
[validation](../validation.md).

## Current peer evidence

All five checkouts were clean at the following revisions. Current paths and
adopted/rejected decisions in execution20-22 reports remain applicable; these
sources and relevant AGENTS/build registrations were rechecked read-only.

| Peer revision | Evidence and decision |
| --- | --- |
| TMP1x2 `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `examples/common/ConsoleBuffer.h`, `test/cli/test_cli.cpp` malformed/control input, public CLI and CMake registration: retain bounded single output drain and whole-record loss; no watch/temperature/startup behavior. |
| 24Cxx `936aa85e21b40b61c6f9cf0306d64c3beb6eaf6e` | `examples/common/ConsoleOutput.h`, `test/test_console/test_console.cpp`, `test/test_cli/test_cli.cpp`, `tools/test_cli_transcript.py`, CMake: retain executable persistent-process evidence; reject multiple backend calls per drain and EEPROM policy. |
| SHT3x-main `f65f15573e3c9b1ef04cce72ad08f307d021e563` | `tools/sht3x_cli_contract.py` and shared dispatch: fixed aliases are useful; sensor commands and compatibility facades are outside this CLI. |
| LDC1614 `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `tools/ldc1614_cli_contract.py`, shared CLI command rows: retain execution/effect classification; reject profile-commit/init replay and blanket confirmation grammar. |
| INA228 `b28c5ea69bcb4d26b351c52c24d3c6fae9fd282b` | `tools/run_i2c_hil.py` transfer budget/assertion cases: require exact observed callback deltas and source identity; no inference of physical transactions or sensor behavior. |

The existing workflow definitions and registrations inspected in execution20-22
are unchanged at these revisions. Firmware and hardware checks remain separate
from native CLI execution. No new TI mask, timing or capability claim was needed.

## Findings and closure

| Requirement | Finding and correction | Executable evidence |
| --- | --- | --- |
| Grouped bounded help | Fixed rows followed stage order, repeating several headings. Reordered existing C++/Python arrays into contiguous topics; no new renderer or registry. | `setupAndAppearanceContracts`: one of each of nine headings, expected order and each canonical HELP row inside its topic. New assertions against original source fail 17 times. |
| Exactly one terminal closure per request | Persistent host tracked completed operation tokens but accepted extra records for closed local requests. Added per-request closure checks and validation of every END, with trailing-output validation at EOF. | `exercise_terminal_ordering`: real schema-valid fixture records damaged only in session order; duplicate/contradictory END, late payload and foreign/future closure reject. Original reader demonstrably accepts duplicate END. |
| Explicit-step executable evidence | The route fixture could expire during output draining and accept zero callbacks. Give its manual job a sufficient immutable deadline, require exactly one callback for the first ready read, and explicitly cancel after measuring the route. | `check_cli_contract._route`: 55 rows/164 routes pass with exact step deltas and original request closure. |
| Complete response reservation before take | Five-record branch had tests; eight-record branch with unannounced original completion did not. Production logic was already correct. | `unannouncedCompletionTakeReservation`: insufficient capacity retains slot; sufficient capacity announces once then takes; original/query nonce and origin remain distinct; zero callbacks/loss. |

## Requirements reviewed without production changes

- Strict whole-command arity/numeric/enum parsing, poisoned input through newline,
  CRLF/backspace, 64 input bytes/one command, 512-byte records, 8192-byte queue and
  one 256-byte nonblocking drain; maximum-width and congestion tests use real code.
- Validated atomic cross-family target selection, explicit evidence forgetting,
  independent staging/desired/observed/applied fields and validity masks. Invalid
  commands preserve represented state while already admitted auto work may advance.
- One managed poll site, explicit step replacing auto even at zero, immutable
  deadlines, late failure preservation, core-owned retained results, loss without
  consumption/retry and request/nonce/token separation. No second operation engine.
- Core Sync raw routing, variant/procedure gates, actual-control IRQ ACK,
  all-source versus managed accounting, idle clear, saturated assertion rejection,
  passive health and opt-in input hook. No register composer or GPIO setup in CLI.
- All 55 canonical rows and aliases/help topics execute through the shared-source
  fixture. Parity checks compare real help/records/effects and reject broken alias
  and schema dependencies. Typed errors, signed detail and unavailable values
  remain separate from effect/trust/outcome. Output storage checks include NUL.
- Framework-free core/common boundaries, optional maintainer tests and existing
  source-equal package checks. No placeholder routes or extra framework hierarchy.

Host evidence does not qualify serial transport, physical CC/INT behavior,
Arduino/IDF links or board memory placement. Existing unavailable sanitizer
runtimes are reported rather than replacing them with passing mock evidence.

The closure regression includes eight malformed-order cases checked during reads
and EOF draining, one rejected unclosed manual job, and successful EOF draining
of an automatic job and incremental help. Older open operations may finish out
of request order; only records following their own closure are rejected.
