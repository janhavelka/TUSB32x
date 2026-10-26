# Prompt 10 current codec and write-composition peer audit

Read-only source review on 2026-10-26 (Europe/Prague). Peer-prefixed paths below
are relative to `C:/Users/Honza/Documents/Projects/`; other paths are relative to
TUSB32x. No sibling was edited, fetched or built, and no remote workflow was
dispatched, polled or queried. This audit supplies engineering precedents, not
silicon authority or a peer behavioral test result.

## Snapshot and guidance

| Repository | Full local HEAD | Branch | Initial worktree/index |
| --- | --- | --- | --- |
| TUSB32x | `dd82a796e51dcfefa77db36a921d69aec0b12346` | `promptset/tusb32x-implementation-20261023` | Clean |
| PCA9555 | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` | `main` | Clean |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `main` | Clean |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` | `main` | Clean |

Read status, staged-diff statistics, branch and HEAD locally; TUSB32x tracks
`origin/promptset/tusb32x-implementation-20261023`. These observations do not prove
upstream equality. The lead owns fetch, integration, staging, commits and push.
This agent owns only this report and performs no Git mutation.

Reopened parent/local `AGENTS.md`, each peer's `AGENTS.md`, prompt index,
`COMMON.md`, full `10-register-codecs.md`, `AGENT_WORKFLOW.md`,
`MAINTAINABILITY.md`, the baseline supplied-byte decoder contract, and current
progress/design decisions. Peers reinforce standard C++17, non-owning callbacks,
fixed storage, explicit variant facts, local validation and separate trust/effect
evidence. Their lifecycle aliases, sensor initialization/replay and hardware
semantics do not replace the frozen TUSB32x contracts. Prompt 10 implements pure
codecs/composition; role, reset, recovery and IRQ execution remain later stages.

## Current declarations, implementation and tests

| Exact sources reopened | Evidence and decision |
| --- | --- |
| `PCA9555/include/PCA9555/CommandTable.h`; `src/PCA9555.cpp:1077` and `:1320` | Named `constexpr` register constants explain access and pair boundaries. Masked preload preserves unselected proved latch bits. Raw writes reject input registers and direction writes that bypass preload; partial writes invalidate the pair shadow. Adopt named masks, explicit access and required actual-state evidence. Reject copying pair wrap, shadow-based byte RMW, reset absence or pointer-park rules. PCA9555 has no TUSB-style mixed RO/RW/W1C/reset register. |
| `PCA9555/test/test_basic.cpp:369`, `:1340`, `:2331`, `:2428`, `:2439`; `test/test_public_contracts.cpp` | Registered cases reject invalid masks without changing evidence, keep mismatched pairs fenced, preserve other port bytes during RMW, reject read-only targets, and assert no traffic for invalid registers/enums/lengths. The mixed verification test is about matching versus mismatching register pairs, not W1C fields. Adopt literal expected bytes, unchanged evidence and bus-silent rejection tests. It cannot prove TUSB IRQ/reset composition. |
| `LDC1614/include/LDC1614/Config.h:47`, `:97`; `CommandTable.h`; `LDC1614.h` register/profile contracts; `src/LDC1614.cpp:118`, `:132`, `:1203`, `:1225` | Variant is application-supplied because identity is shared. Small predicates gate absent channel registers before callbacks; writable/profile sets differ from readable registers. Adopt explicit variants, small allowlists and register-specific access. Reject identity-based capability inference, sensor word framing, and allowing a generic raw setter to bypass TUSB procedures. |
| `LDC1614/src/LDC1614.cpp:1277`; `test/test_basic.cpp:820`, `:955`, `:1002` | Pure expected-profile helper supplies register values/masks; tests enumerate both variants and exclude live read-only INIT_IDRIVE from persistent comparison. Rejection covers nonexistent/variant/nonprofile registers and zero callbacks. Adopt independent literal tables, command/status exclusion and unsupported-domain coverage. Reject its output-zeroing convention: TUSB32x `decodeRegister` must preserve caller output on error. Reject its full replay/init behavior for this stage. |
| `TMP1x2/include/TMP1x2/CommandTable.h`; `src/TMP1x2.cpp:140`, `:152`, `:197`, `:219`, `:230`, `:235` | Reads allow 0..3; both tracked and raw writes allow only 1..3 before callbacks. `MASK_WRITABLE_CONFIG=0x1FD0` explicitly excludes OS because it is a command. Read output is published after success. Adopt local access checks, temporary output and separation of persistent fields from commands. Reject importing the mask, temperature format, ALERT read effects, or unrestricted whole-word raw write policy. |
| `TMP1x2/test/test_core.cpp:103`, `:115`, `:136`; `test/variants/test_variants.cpp` | Pure conversion rejects invalid values without changing output; access validation rejects temperature writes and out-of-map reads without traffic or health failure. Read failure preserves a caller sentinel. Adopt invalid typed input, output sentinel and callback-count assertions. These tests establish no TUSB variant encoding. |

The lead must independently derive TUSB bytes from the archived exact TI tables.
Peer preservation rules support explicit persistent masks, never echoing live
RO, reserved, W1C or reset bits. Required prompt vectors include extended
`09=B5` ACK `15` versus explicit settings `02`, base `B4` ACK `14`, and
`0A=DE` with debounce 1 yielding `56`, without replaying read reset bit `08`.
ACK must use observed/trusted controls even when staged desired fields disagree.
Normal `45` changes preserve documented internal `03`; literal original-device
recovery writes require internal-zero qualification and their separate sequence.
No peer precedent authorizes relaxing TUSB322I advertisement/fault-clear gates.

## Examples, CLI and runner assumptions

Inspected `PCA9555/examples/01_basic_bringup_cli/main.cpp` raw register handlers,
help and confirmations; `tools/run_i2c_hil.py` register command classifiers and
negative cases; and `tools/test_run_i2c_hil_parser.py`. Raw CLI writes accept only
output/polarity addresses 2..5, excluding direction writes. The runner expects
range/length rejection and distinguishes register reads/writes. Adopt alignment
between advertised access and real core rejection; do not copy confirmation
grammar, paired-register dumps or expand TUSB firmware work into this prompt.

Inspected `LDC1614/examples/common/Ldc1614Cli.{h,cpp}` register descriptors,
`printRegisterValue`, decoder commands and variant staging; and
`tools/ldc1614_hil_runner.py` register-record patterns, reserved-bit validation,
identity expectations and invalid requests, with its host fixture module
`tools/test_ldc1614_hil_runner.py`. Output preserves raw words, access and
reserved validity; shared IDs explicitly report `variant=UNKNOWN`. Adopt raw
and validity alongside typed fields, and separate selected variant from identity.
Reject copying CLI-local register conversions: the TUSB CLI must format the
public pure `decodeRegister` result instead of maintaining a second codec.

Inspected `TMP1x2/examples/common/Tmp1x2Cli.cpp` pure conversions and raw commands,
`Tmp1x2CliDiagnostics.cpp` owner/result and diagnostic state, and
`tools/hil_tmp1x2_runner.py` register suite and invalid requests. The runner
expects every raw register record, exact transfer counts, pure conversion
literals and dirty trust after a same-value raw write. Adopt those distinctions
as future integration requirements. Reject copying a second firmware diagnostic
scheduler/result store or treating transport success as configuration proof.
TUSB shared CLI and Python runners remain unimplemented stages 20 onward.

## Registration and workflow source review

Read all three peer root `CMakeLists.txt` and `.github/workflows/ci.yml`, plus
PCA9555/LDC1614 `platformio.ini`. PCA/LDC register Unity native tests through
PlatformIO, with reviewed cases called by `RUN_TEST`; LDC compiles its shared CLI
there. TMP1x2 CMake registers core, CLI, console, jobs, variants, helpers, bus,
no-Arduino, version, component, Doxygen and HIL-runner tests with finite timeouts.
Registration is source evidence only; no peer test was run during this audit.

PCA's workflow uses Python 3.11, Core 6.1.19, Arduino platform 55.03.311 and IDF
v5.4, with native, CLI/HIL/parser, package and Doxygen checks. LDC's requirements
pin Core 6.1.19/pyserial 3.5; workflow sources cover native, CLI/readiness/hygiene,
runner, version, clean consumer, documentation and IDF v6.0.2. TMP1x2 specifies
Ubuntu 24.04 strict warnings/ASan/UBSan CMake/CTest, contract/Doxygen checks,
Core 6.1.19 Arduino builds and IDF v5.3.2/v5.5.1/v6.0.1 links. These are intended
commands, not evidence of green runs, supported TUSB firmware or hardware.
Reject LDC's configure-time Python/version dependency for tests-OFF consumers.

TUSB32x still has no `.github/workflows` directory. Reopened root
`CMakeLists.txt`, `test/CMakeLists.txt`, `include/TUSB32x/CommandTable.h` and prior
transport audit: tests default OFF; strict private warnings, standalone public
header translation units, Release assertions and finite CTest timeouts already
exist. Extend the registered `tusb32x_protocol` runtime suite and public link
checks; compile-only planned APIs remain separate. The public decoder must not
need a private source include. Lead validation records actual local commands
and outcomes; this source audit claims no build, remote CI or physical pass.
