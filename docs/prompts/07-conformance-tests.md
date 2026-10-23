# P07 — Close protocol, state and portability test gaps

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md)
and [MAINTAINABILITY.md](MAINTAINABILITY.md). Earlier stages already contain
behavior tests. Add missing cross-cutting and hostile cases, fix the defects
they expose, and update the public contract and documentation in this stage.

## Re-audit useful current tests

Inspect implementation, examples, test registration, runners and CI together.
Record revisions/dirty state and actual adopted/rejected conventions. These
clean 2026-10-23 baselines provide specific starting points:

| Peer revision | Files and useful patterns |
| --- | --- |
| LDC1614 `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` | `test/test_basic.cpp`, `test/support/FakeLdc1614Device.h`: explicit phase/callback expectations, `test_initialize_exact_transfer_counts_and_zero_one_large_budgets`, cancellation at every phase and preserved full error detail; `tools/test_ldc1614_hil_runner.py`: runner evidence tests |
| LSM6DS3TR `513c5641d9b7dda63fddc27267ea62aa8d03ff93` | `test/test_basic.cpp`, `test/support/FakeBus.h`: `test_ambiguous_write_effect_is_observable_and_never_retried`, `test_configuration_settle_arms_after_readback_with_changing_budgets`, cancellation after every phase; `examples/02_owner_soak/main.cpp`, `tools/test_run_hil.py`: finite owner and token-correlated results |
| PCA9555 `498178e4d9e4acb6fab7d8b816ad5021efe8806a` | `test/test_basic.cpp`, `test/test_public_contracts.cpp`, `src/PCA9555.cpp`: possible write commitment, exact buffers and public compile contracts; `.github/workflows/ci.yml`: native, guards, Doxygen and package consumers |
| ADS1115 `381236ebfed813cd7a9aca34520ce031d8a0ff6b` | `test/test_basic.cpp`: configurable callback time advance and applied failed writes; `examples/02_owner_safe_poll/main.cpp`, `.github/workflows/ci.yml`: owner example and native/IDF checks |

Refresh TMP1x2 CLI fixtures and INA3221 framework boundaries where relevant.
Do not copy peer assumptions unquestioned: ADS1115's native test includes
`Wire.h` and exposes private members with a macro; that does not prove a pure
C++ consumer. LDC1614's root CMake currently requires Python and IDF; do not
reproduce that requirement for downstream plain C++ builds. Some peers label
callback counts as transactions; here those are distinct measured quantities.
Borrow test structure, not sensor registers, arbitrary size caps or giant files.

## Test organization and independent evidence

Use focused suites such as protocol/codecs, owner operations, port/IRQ/health and
public/consumer contracts. Keep a small shared fake and readable expectation
builders. Each test should show setup, the important action or sequence, and
observable assertions. Avoid a general simulation framework, test DSL or copies
of the production state machine. Use table-driven tests when cases really
differ only by data; keep important fault sequences explicit.

The scripted transport must reject an unexpected address, callback kind, byte,
length, timeout or extra call. Record callback attempts independently of simulated
physical phases. Its device model needs only documented RO/live fields, W1C,
reset command behavior, configurable status changes and writes that can apply
before reporting failure. Expected masks and byte vectors come from cited TI
tables, not production helpers. Do not invent atomic snapshots or lossless IRQs.

Maintain a matrix `TI/software requirement -> variant/capability -> public API ->
named test -> command/result`. Use actual test symbols after implementation;
the descriptive names below are suggested acceptance cases, not a mandated API.

## Required regression cases

| Area | Required observable result |
| --- | --- |
| Six-part capabilities | Both legal addresses per variant; wrong address/model rejected without callbacks; no base `0xA0`; no base preference/accessory-disable/termination-state-machine operation |
| Identity | Exact eight-byte order; reversed/wrong/partial signature rejected; matching signature cannot infer polarity; unknown revision follows explicit policy |
| Register ranges | Pointer supplied each read; only documented contiguous ranges; holes/wrap rejected; short/incomplete receive never changes public output |
| Encodings | All advertisement/detect/accessory/attachment/duty/mode/preference/debounce encodings; base versus extended differences; raw reserved state retained; detached direction/current are valid NOT_APPLICABLE interpretations, never usable orientation/current claims |
| Settings versus W1C | P04 `0xB5 -> ACK 0x15`, staged `0x02` ignored; settings write `0x02` keeps pending IRQ; base `0xB4 -> ACK 0x14`; unknown applied settings require reconciliation or explicit failure |
| Reset command isolation | P04 `0xDE -> debounce update 0x56`; reset bit is not replayed by ordinary writes; `0x45` internal bits preserved |
| Extended mode sequence | Disable, set mode, completed-write-based 5 ms wait, enable; exact bytes preserve applied non-mode fields; fault/cancel after each boundary retains disabled/uncertain state |
| Base mode sequence | Attached precheck rejects before writing; unattached prerequisite and read/write attachment race documented without claiming atomic enforcement |
| Reset lifecycle | Command attempted once; no successful readiness before documented wait/evidence; timeout/cancel/error after command keeps reset/configuration uncertainty; `end()` is not a reset |
| Base dead-battery recovery | Explicit base 320/I operation requires identity and caller-owned VDD-return prerequisite; check 0x45 internal bits zero before literal 04, wait 31 ticks from fresh completion boundary, literal 00, verify `(reg45 & 0x07) == 0`; four callbacks maximum; no transient port snapshot or automatic initialization write |
| TUSB322I gate | Separate signature, mode-00 meaning, absent `0x45`, independent IRQ/fault ACK if supported; unresolved advertisement write rejected before callbacks; autonomous VCONN side effects documented |

Use explicit named tests for these fault boundaries:

1. `ack_uses_applied_profile_when_desired_differs`: pending interrupt and staged
   settings differ; assert write byte, remaining hardware settings and unchanged
   staging. Repeat with applied state unknown and with changed live RO bits.
2. `failed_write_may_have_applied`: run each mutating phase once with no applied
   write and once with applied data followed by error. Keep the same transport
   error when phase information is unavailable. Neither path may blindly replay
   reset/role writes or declare trust based on unrelated successful reads.
3. `cancel_after_each_phase_is_bus_silent`: before first transfer, after every
   read/write, during waits and after terminal completion. Check token/result,
   write effects, configuration generation and explicit reconciliation needs.
   Repeat across end/rebind; bookkeeping must not silently erase relevant effects.
4. `failed_status_read_preserves_observation`: old data remains distinguishably
   stale with its age/generation; failure does not synthesize detach or zero current.
   Partial multi-byte results cannot become a new complete observation.
   Preserve historical field validity/time when invalidated becomes true after
   end, external invalidation or a possible port-affecting write. Only complete
   fresh publication clears it. A successful pre-ACK service observation survives
   later ACK/recheck failure while the operation reports its original error.
   A delayed final poll must not replace the acquisition boundary with publication
   time and make old observations appear newly sampled.
5. `irq_reassertion_remains_visible`: event between read/ACK, during ACK, before
   recheck and after last recheck. Bound service work and expose remaining work.
   Status read alone does not clear IRQ; coalesced events are not counted as exact
   edges. No fabricated minimum delay between ACK attempts.
6. `termination_controls_remain_distinct`: cover every supported combination of
   disableTerm and disableRdRp. Clearing 0A while 45 remains disabled cannot report
   TerminationState::ENABLED. Failed/uncertain control writes and pending reset
   preserve UNKNOWN wherever complete configured-availability evidence is absent.
7. `reconcile_partial_profile_allows_explicit_apply`: prove actual persistent
   controls after an interrupted apply while desired differs. Publish that actual
   verified image as KNOWN, retain desired and mismatch evidence, then admit a new
   explicit apply. Do not replay the cancelled operation or claim it succeeded.
8. `dead_battery_recovery_retains_interrupted_sequence`: reject other variants and
   missing identity without callbacks; unexpected 0x45 internal bits reject before
   writes. Inject error/cancel/deadline after each read/write/wait, preserve original
   error and effects through end/rebind, and never certify completed recovery from
   a later clear bit alone. Only a new explicit request may repeat the procedure.
   After taking the result, diagnostics still expose the unresolved sequence.

## Time and owner scheduling tests

Use an external fake monotonic clock and transport that can advance it during
callbacks. Supply completion evidence according to P02; when unavailable, verify
that the driver waits for a fresh owner timestamp before time-sensitive decisions.

- With `maxTransfers=0`, no callback. With 1, at most one callback even on failure
  or an IRQ loop. With N, at most N with refreshed/charged elapsed time. A
  STOP-separated logical read may record two physical transactions and one callback.
- Bind/start/cancel/results/cached getters/statistics reset are bus-silent. Invalid,
  busy, stale-token and unsupported requests preserve prior operation evidence.
- The owner clock has resolution at most 1 ms; floor-valued timestamps are
  supported. A named 1 ms software margin makes the TI 5 ms mode wait require
  at least 6 elapsed ticks. Example: mode write starts at 100 ms and completion
  is known in that timestamp domain as 108 ms. Earliest enable is 114 ms. If
  completion time is unavailable and the next fresh owner timestamp is 110 ms,
  anchor there: earliest enable is 116 ms. Test before, at and after each boundary.
- Use a fake higher-resolution clock to complete a mode write just after a tick
  (for example, 108.1 ms, reported as 108). At reported time 113 ms only 4.9 ms
  may have passed: assert no enable write. At 114 ms the wait may expire. Repeat
  with completion just before a tick to cover the worst quantization gap. Check
  actual elapsed time in the fake, independently of the driver's tick arithmetic.
- Apply the same completion-anchor rule to reset readiness. A reset write that
  consumes time cannot use the pre-write timestamp to shorten the specified wait.
  Before relying on the 95 ms reset maximum, require 96 elapsed ticks from its
  fresh anchor. Keep the 1 ms software margin separate from the TI timing constant;
  it does not extend the operation deadline. Distinguish software wait policy
  from a hardware guarantee and port readiness.
- Apply the same fresh-anchor/sub-ms tests to the base dead-battery 30 ms wait:
  require 31 elapsed ticks before writing 0x45=0x00. Stalled time and late callbacks
  cannot shorten it; no transient port snapshot is published during recovery.
- An operation due at 110 ms with a callback starting at 108 ms gets no more
  than 2 ms remaining timeout. A success returned at 111 ms cannot be reported
  as an on-time operation from stale 108 ms evidence. Preserve actual write effects.
  Repeat with I2C_TIMEOUT, I2C_BUS and generic I2C_ERROR returned by the callback;
  each original error/detail remains primary even when fresh time is past deadline.
- Test every supported timestamp representation at rollover/maximum and every
  interval limit. Test backward/stale timestamps according to the public contract,
  overflow-safe arithmetic, saturated counters and no busy loop during waits.
- A fixed-capacity external scheduler alternates two instances on one serialized
  fake bus. One device waits, fails or produces repeated IRQs; the other still
  gets bounded progress. Fill queue/result storage, reject backpressure cleanly,
  retain correlated results and prove no lock is held across owner poll intervals.
  No FieldCore, framework, RTOS, GPIO or application-wide recovery dependency.

## Status, diagnostics and public contracts

Test invalid enums, null and invalid buffer lengths where those APIs exist,
timeout limits, unsupported register writes and static message lifetime. Check
that every callback is counted once, failures count as attempts, local rejections
do not count as I2C failures, and raw Sync traffic remains in the all-callback
source even when excluded from a managed projection. Counter reset is passive.
The probe CLI alias has the same managed accounting as initialization. Complete
verification with a desired mismatch retains KNOWN actual evidence and desired
unchanged; an unresolved transition still prevents KNOWN.

Health, trust, data freshness, port attachment, IRQ pending and operation outcome
must be independently testable. Detach is normal. OFFLINE cannot suppress an
explicitly requested legal operation or trigger automatic retries. Check all
documented projection transitions and thresholds without manufacturing traffic.

Compile each public header alone and include the facade from multiple translation
units. Compile and **link** a normal C++17 consumer using only installed/package
headers and the library, without sibling include paths, framework stubs, Git or
Python/generator tooling. Demonstrate no heap calls in exercised core operations
with an allocation trap or equivalent meaningful check, not text search alone.
Do not test private spelling when a public observable contract suffices.

## Checks, documentation and completion

Register every suite with the selected native runner/CTest so CI executes it.
The P03 host CMake path should support the following or a documented equivalent:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
scripts\pio.cmd test -e native
```

Run strict warnings and supported ASan/UBSan configurations. Test assertions must
still work in Release builds; Unity/check macros or an always-active failure
function must not disappear under `NDEBUG`. Verify that a deliberately failing
fixture exits nonzero and registered suites actually run. Use focused metadata/
boundary guards with negative fixtures only where appropriate; text search does
not establish timing, behavior, no allocation, transport isolation or portability.

After fixes, rerun focused tests and then the full host matrix once. Update
`docs/validation.md` with exact commands, tool versions, test counts and
PASS/FAIL/NOT RUN/BLOCKED. Update the feature matrix, test instructions, API
Doxygen and source comments for any clarified behavior. Run Doxygen for changed
public contracts; missing tooling is reported honestly. Later CLI, adapter and
runner tests extend this suite. Host success is not hardware compliance or
electrical qualification.
