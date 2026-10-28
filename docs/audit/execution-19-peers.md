# Prompt 19 timing and portability audit

Read-only review on 2026-10-28, starting from TUSB32x
`4207107e5fc9bf2d5b8f7464bb4342f7763317b9` on the recorded
`promptset/tusb32x-implementation-20261023` branch. The lead and other agents own
concurrent prompt 19 changes; this reviewer owns only this report. No sibling
fetch, edit, build, Git mutation or remote CI query was performed.

Re-read parent/repository AGENTS, COMMON, prompt 19, AGENT_WORKFLOW, the baseline
time/lifetime/accounting rules, CHIP_EVIDENCE, progress and decisions. Inspected
the current five production sources, private owner-time/protocol headers, public
owner/configuration declarations, registered tests, host example and CMake files.
There is no checked-in TUSB32x `.github/workflows` directory at this stage; the
dedicated workflow checkpoint remains prompt 34.

## Current peers

All four peers were on `main` tracking `origin/main`, with no tracked or untracked
changes reported by local Git. This is local source evidence, not verification of
their remote branches or test outcomes. Paths below are relative to `Projects/`.
Each peer's current `AGENTS.md`, root `CMakeLists.txt`, `platformio.ini` and
`.github/workflows/ci.yml` were inspected in addition to the listed files.

| Peer and full revision | Sources inspected | Adopted and rejected conventions |
| --- | --- | --- |
| ADS1115 `235af3e8c722661a89a44766a4ccd903a654f6fe` | `include/ADS1115/ADS1115.h`; `src/ADS1115.cpp` poll and callback-time paths; `test/test_basic.cpp` timeout clipping, post-callback origin, wrap and slow-read cases; `examples/02_owner_safe_poll/main.cpp`; `tools/run_i2c_hil.py` | Retain the independent callback-advanced fake clock, bounded application polling and exact result consumption. Reject division of remaining time by callback budget as a TUSB32x timing model: Status-only callbacks require a new owner sample. The test's `Wire.h` and `#define private public` prove neither public portability nor isolated consumption. Its packaged `-fsyntax-only` source check is not a linked/running consumer. |
| LSM6DS3TR `362b140990bc4ba2bc7f32cef50f8d9f30b4d3cc` | `include/LSM6DS3TR/Config.h`; `src/LSM6DS3TR.cpp` admission/poll; `test/test_basic.cpp` changing-budget settle and maximum-time cases; `examples/02_owner_soak/main.cpp`; `tools/run_owner_soak.py` | Adopt a wait anchored after callback/readback, zero-budget time transitions, literal transfer counts and terminal-result backpressure. Reject its absolute-comparison/saturating-deadline policy for TUSB32x, which explicitly supports unsigned half-range rollover. Do not copy sensor settling, retries, profile restoration or multi-callback owner turns. |
| INA3221 `41b6184b75f3c152611d4236f7ea41a5ce77bc2c` | `include/INA3221/INA3221.h`; `src/INA3221.cpp` effective deadline and transfer clamp; `test/test_owner_operations.cpp`; `test/support/ScriptedTransport.h`; `examples/01_basic_bringup_cli/main.cpp`; `tools/hil_cli_runner.py` | Retain fixed transport scripts, unchanged-output/token checks, callback-failure detail and possible write acceptance. Its post-trigger origin and zero-budget cases are useful scenarios. Reject divided remaining-time budgets, an additional optional poll deadline, compatibility facades and write-based reconciliation. Fresh runner token/sequence framing is future runner evidence, not host timing or physical qualification. |
| PCA9555 `498178e4d9e4acb6fab7d8b816ad5021efe8806a` | `include/PCA9555/PCA9555.h`; `src/PCA9555.cpp` cooperative poll/result paths; `test/test_public_contracts.cpp`; `tools/package_consumer.cpp`; `tools/check_package.py`; Arduino/native-IDF CLI owner calls; `tools/run_i2c_hil.py` | Adopt public-only linked/running consumption outside the source tree and noncopyable/fixed-value contracts. Its package fixture compiles the extracted implementation and runs the executable; this is stronger than signature checks. Its Unity stub include paths and Python/PlatformIO packaging coordinator are maintainer requirements, not ordinary consumer dependencies. Do not copy pointer-park cleanup, physical-transaction count assumptions or sensor-specific health vocabulary. |

Peer workflows register actual native tests, version/contract guards and separate
firmware/package jobs. The reviewed jobs use Python 3.11 and PlatformIO 6.1.19;
INA3221 also runs its strict host compile guard, and PCA9555 runs the linked
package consumer. These definitions inform test separation, but no peer workflow
was dispatched or polled. All four root CMake files are native-IDF registration,
not standalone host CMake entry points. TUSB32x retains its existing tests-OFF
ordinary C++17 build without Python, Git, Doxygen, IDF or sibling paths. Do not
import LDC1614's unrelated build dependencies into that contract.

## Timing source recheck

Reopened the archived TI PDF bytes with PyMuPDF, rather than relying only on
CHIP_EVIDENCE prose: B SLLSEN9F p7 reset table and p14 recovery; L SLLSEP2D and A
SLLSEQ8D p6 reset tables and p14 role sequence; V SLLSEO5D p7 reset table and
pp10-11 DFP/UFP sequences. The reset entries remain 26/49/95 ms min/typ/max;
L/A require the mode write followed by 5 ms before termination re-enable; B
requires the explicit 04, 30 ms, 00 recovery sequence. V's three-write DFP/UFP
recipe adds no 5 ms wait. Exact archived revisions/hashes are in
[provenance](../reference/source/provenance.json).

The 1 ms software quantization margin remains separate: 6/31/96 elapsed integer
ticks are the conservative scheduling thresholds. No public 95 ms sleep, 1 ms
retry, 25 ms spacing or board 700 ms debounce defines another core interval.
Nominal CC debounce, physical current resampling and successful register reads
remain separate evidence. This review changed no silicon fact or support gate.

## Coverage audit at prompt entry

The following is source inspection of registered behavioral cases, not a claim
that this reviewer reran the host matrix. Final prompt 19 commands/outcomes are
recorded by the lead in [validation](../validation.md).

| Requirement | Existing evidence and finding |
| --- | --- |
| Zero/one/large budget, failed attempts, bounded local work | `initializationAndBudgets`, `failedAttemptBudgetAndUnknownActual`, IRQ budget/fault tests; production `poll` is an acyclic pass and returns after each callback. A larger budget never invents callback completion time. |
| Bus-silent lifecycle/admission/results, rejection preservation | `emptyAndAdmission`, `interruptionAtEveryPhase`, `invalidationAndFrozenResults`, `reinitializationAndAtomicBinding`, public raw-admission tests. Cached copies and terminal results do not invoke transport. |
| Quantized mode wait | `physicalModeWait` independently advances microseconds to 108.1/108.9 ms inside a callback, then samples 108/110 ms and checks actual elapsed time at 114/116 ms; `exactWaitGatesAndLateFinalTime` checks deadline equality and 322I's distinct no-extra-wait plan. |
| Quantized reset/recovery waits | `independentPhysicalWaits` advances completion by 0.1/0.9 ms, delays the first fresh sample, exercises stalled/zero budgets, before/at boundaries and actual elapsed 95/30 ms; `wrappedWaitAndDisabledControls` covers rollover. |
| Deadlines, late success and primary transport errors | `timeBoundaries`, `clippedProcedureTimeouts`, apply/reset fault matrices and late-publication cases cover clipping, immutable deadlines and error retention. Add the explicit callback-start108/deadline110/physical-completion111 case with all three error kinds and successful pending completion; separate facts currently do not make that exact independent-clock regression. |
| Half range, uint64 maximum, invalid/backward/equal ticks | `timeBoundaries`, `wrappedModeWait`, `wrappedWaitAndDisabledControls` and admission tests exercise production behavior; `contract/time_arithmetic.cpp` is supplemental compile-only evidence. Reused pre-callback time cannot be detected by this Status-only API and remains an owner precondition. |
| Saturating counters, sequence, token and generations | `saturatingCountersAndLocalHistory`, `saturationWithoutPublicSeeds`, `saturatingSequenceBoundary`, `tokenBoundaryArithmetic` exercise bounded near-maximum fixtures without public seeds. Desired/verified generation arithmetic lacked near-limit coverage; a small shared private `nextGeneration` helper with direct boundary tests closes this gap without billions of calls or a public seed API. Saturated counts are not unique identities. |
| Two targets, queue pressure and fairness | `twoInstancesAndStatistics` proves independent init/failure/tokens on addresses47/67. It does not establish fixed external queue/result backpressure or progress while the other target waits/reasserts IRQ. Add the bounded two-owner scheduler fixture, keeping scheduling outside the core. |
| Null context, callback changes and target records | `callbackAssociationAndNullContext` covers either function pointer, context, variant and address changes, same-target timeout changes, unresolved-effect rejection and explicit forgetting. Production target association includes both callbacks. |
| Allocation, lifetimes and public linking | Existing headers compile alone and passive values link across translation units; core uses fixed storage and terminal callback loans. Text inspection is insufficient for no-heap proof. Add a behavioral allocation trap and a separate public facade consumer that calls every implemented method; preserve borrowed context/static-message lifetimes and invalid-output preservation. |
| Health, trust and result independence | Diagnostics tests preserve full-width detail, unknown transport completion timestamps and managed/raw accounting; OFFLINE admits legal work. Counter clear cannot repair trust, erase possible writes or consume a result. |

No production timing, lifetime or accounting defect was confirmed by this
review. The concrete gaps were communicated to the lead before implementation;
the independent consumer and scheduler scopes are owned by other agents. The
generation helper should replace only the existing two generation increments.
The physical clock tests are host simulation; backend lock/timeout guarantees,
linked framework firmware and electrical qualification remain separate work.

Follow-up source review confirmed the lead's private `nextGeneration` extraction
replaces both intended increments and tests zero, maximum-minus-one and maximum.
The scheduler's `literalLateWriteKeepsOutcomeAndEffect` now advances physical time
inside the callback from 108 to 111 with a 2 ms timeout, preserves explicitly modeled
write acceptance, and checks OK versus all three actual failure kinds. These are
the scoped closures recommended above; their executed results belong to the
lead's final validation record.
