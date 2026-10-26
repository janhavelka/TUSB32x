# Prompt 9 current transport peer audit

Read-only source review on 2026-10-26 (Europe/Prague). Siblings were not edited,
fetched or built. No remote CI was dispatched, polled or queried. Paths beginning
with a peer name are relative to `C:/Users/Honza/Documents/Projects/`; other paths
are TUSB32x-relative. This is source evidence, not a peer test or hardware pass.

## Snapshot and guidance

Read each local HEAD, branch and `git status --short` independently. TUSB32x's
index was also clean when this audit began; its upstream was
`origin/promptset/tusb32x-implementation-20261023`. The lead owns synchronization.

| Repository | Full local HEAD | Branch | Initial worktree |
| --- | --- | --- | --- |
| TUSB32x | `d7fbd59a3604d1af70dc9c751b44fa55742fb878` | `promptset/tusb32x-implementation-20261023` | Clean |
| PCA9555 | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` | `main` | Clean |
| ADS1115 | `235af3e8c722661a89a44766a4ccd903a654f6fe` | `main` | Clean |
| LDC1614 | `1db38b63cb93a561b6e0bea51017ab94102ebd24` | `main` | Clean |

Reopened parent/local `AGENTS.md`, each listed peer's `AGENTS.md`, prompt index,
`COMMON.md`, `9-transport-identity.md`, `AGENT_WORKFLOW.md`, current progress and
design decisions, and baseline transport/error/effect/accounting requirements.
Peers agree on non-owning callbacks, external serialization, fixed storage,
preserved backend detail and separate hardware evidence. Their physical-attempt
wording and device-specific effects do not override TUSB32x's callback counting
or verified TI framing. This agent owns only this audit; no Git mutations.

## Transport and fault choices

| Exact current source | Evidence | Adopt or reject for prompt 9 |
| --- | --- | --- |
| `PCA9555/include/PCA9555/Config.h`; `Status.h`; `PCA9555.h`; `src/PCA9555.cpp:149` `_mapTransportResult` | `TransportResult` carries exact completed TX/RX byte counts and independent `WriteEffect`. Successful short counts become I2C_ERROR with original detail. Register writes with non-OK status remain MAY_HAVE_COMMITTED even if transport claims COMMITTED. Pointer acceptance and register-data acceptance are distinct. | Adopt independent status/effect evidence and exact byte framing. Reject importing TransportResult, completed-byte inference or GPIO register-pair semantics into TUSB32x's fixed Status-only callback contract. A core cannot discover short success without adapter evidence. |
| `PCA9555/src/PCA9555.cpp:358` `_readRegs`, `_writeRegs`, `_readPair` | Local validation precedes callbacks; writes use a three-byte stack array; read-pair output is published after success. Ambiguous writes invalidate relevant shadow pairs. | Adopt local rejection, fixed buffers and publication only on success. TUSB32x uses two-byte `[register,value]` writes and independently verified contiguous byte ranges, not PCA9555 pair auto-increment or little-endian port values. |
| `PCA9555/src/PCA9555.cpp:232` `_updateHealth` and tracked/raw/cleanup wrappers | Counters saturate; a later success preserves last error. Probe bypasses tracked health; successful errata cleanup deliberately does not count as health success. | Adopt saturation and retained last-failure evidence. Reject reusing these counters as TUSB32x all-source totals: every actual callback, including raw diagnostics, must count once; managed evidence is a separate filtered view. No PCA pointer-park cleanup belongs here. |
| `PCA9555/test/test_basic.cpp:2461` `test_transport_short_counts_are_rejected_and_never_fake_success`, `:2477` timeout/deadline distinction, `:1284` non-OK committed write | Tests preserve caller output on partial read, distinguish I2C_TIMEOUT from owner TIMEOUT, and retain write uncertainty after accepted data plus failure. | Adopt these negative-case principles. Adapt short-read injection to an adapter returning failure; TUSB32x callback success has no completed-count channel. Do not replace original transport status with an owner outcome or effect label. |
| `ADS1115/include/ADS1115/Config.h`; `Status.h`; `ADS1115.h`; `src/ADS1115.cpp:2507` raw and tracked wrappers, `:2639` `_readRegister16Raw` | Synchronous Status callbacks borrow buffers; register output is assigned only after successful local two-byte receive. Callback timeout and lock ownership are documented. Tracked wrappers skip INVALID_CONFIG/INVALID_PARAM based on returned code. | Adopt scratch receive storage and preserved native detail. Reject code-based accounting exclusion: a callback returning an illegal validation status still consumed one actual invocation. Separate local preflight from callback normalization at the invocation boundary. |
| `ADS1115/test/test_basic.cpp:1475`, `:1508`, `:4203`, `:6310` | Tests cover five distinct I2C error codes with exact details/call counts, invalid buffers, and failed writes applied to hardware before returning the same error. The unclassified-write matrix intentionally preserves most owner/unknown Err values; IN_PROGRESS alone becomes INDETERMINATE (`src/ADS1115.cpp:169`). | Adopt distinct transport errors, exact backend detail and both applied/non-applied failure fixtures. Reject inherited error permissiveness: TUSB32x must accept only OK/I2C_* callback codes and normalize every other/invalid value to I2C_ERROR. Its allowed I2C_BUSY also needs coverage. |
| `LDC1614/include/LDC1614/Config.h`; `LDC1614.h`; `test/support/FakeLdc1614Device.h:20`, `:166`, `:189`, `:344` | Fixed-memory fake records kind/address/register/value/timeout, checks exact three-byte write and one-byte-pointer/two-byte-read lengths, injects failure on a numbered callback, and can commit before failure. It models destructive STATUS/DATA reads and conversion arrival between transfers. Logs stop growing at 192 entries while call count continues. | Adopt deterministic failure placement, original Status and commit-before-error injection. Reject sensor word framing, destructive-read semantics and silent log exhaustion. Prompt 9 needs fixed scripted expectations that fail on unexpected bytes, timeout, address, kind, length or extra call. Expected identity/access vectors must come from TI rather than shared production constants. |

Neither peer ACK-based probe proves TUSB32x variant or polarity. PCA9555 and
ADS1115 explicitly lack device identity registers. LDC1614's explicit variant
fact is a useful precedent: identity does not authorize capability expansion.
TUSB32x's exact ascending eight-byte signature, revision policy and gated TUSB322I
capabilities remain governed by its own archived TI evidence.

## Examples and runner assumptions

Inspected `PCA9555/examples/common/I2cTransport.h`,
`examples/01_basic_bringup_cli/main.cpp`, and `tools/run_i2c_hil.py`.
The adapter checks queued/received counts; Arduino-ESP32's staged repeated-start
pointer means a zero-byte read does not establish a NACK phase. The CLI installs
callbacks and keeps probe evidence separate from health. The runner explicitly
states that ACK and dry runs are not chip identification or physical HIL.
Adopt exact-count adapter responsibility and conservative error classification;
reject its per-phase timeout application as proof of one aggregate bound.

Inspected `ADS1115/examples/02_owner_safe_poll/main.cpp`,
`examples/common/I2cTransport.h` implementations and native adapter tests, and
`tools/run_i2c_hil.py` firmware-clean-commit validator and parser fixtures.
The owner example serializes callbacks, subtracts lock time, refreshes remaining
Wire timeout between phases and rejects short reads. It rounds a zero-tick wait
up to one tick; SDK-internal waits and cleanup are not proved bounded merely by
these checks. Adopt ownership and remaining-budget intent; do not copy this as a
proved TUSB32x adapter. Runner firmware SHA/clean output is evidence from the
target, distinct from host checkout metadata; TUSB runner implementation is later.
The common adapter's `initWire` bus-clear logic remains application setup and is
not a reusable core callback behavior.

Inspected `LDC1614/examples/esp32/I2cMasterTransport.{h,cpp}` and
`examples/common/Ldc1614Cli.cpp`, plus `tools/ldc1614_hil_runner.py` identity,
provenance, incomplete-output and no-hardware handling. The example adapter maps
ambiguous IDF transaction INVALID_STATE to generic I2C_ERROR and preserves native
detail. It passes a clamped timeout to synchronous IDF calls; this alone proves
neither aggregate SDK lock/wait bounds nor global serialization. LDC requires
repeated START for its silicon; that rule is not TUSB authority. Adopt generic
error classification and explicit target identity/incomplete evidence handling;
leave framework adapters and serial runners to their assigned stages.

## Test registration and workflow source review

All three peer `platformio.ini` files register Unity native tests with core
source compilation. `PCA9555/test/test_basic.cpp`,
`ADS1115/test/test_basic.cpp` and `LDC1614/test/test_basic.cpp` register the reviewed
behavioral cases using RUN_TEST. LDC also compiles shared CLI source in its native
environment. These are executable tests, not compile-only API assertions.

Read each peer's `.github/workflows/ci.yml`, `platformio.ini` and root
`CMakeLists.txt`. PCA pins Core 6.1.19, Python 3.11, Arduino platform 55.03.311 and
IDF v5.4, with native, timing/CLI/HIL, package and Doxygen gates. ADS pins the same
Core/Arduino platform, native platform 1.2.1 and IDF 5.3.5 container, with native,
owner examples, parser, package and docs jobs; its native/package C++11 checks
are not the TUSB32x C++17 policy. LDC pins Core 6.1.19 and pyserial 3.5 in
`requirements-dev.txt`, the same Arduino platform and IDF v6.0.2, with native,
CLI/readiness/hygiene, runner fixtures, clean consumer, version and docs gates.
These checked-in definitions establish intended commands only, not run results.
LDC's configure-time Python/version check is rejected for ordinary TUSB32x
tests-OFF consumers.

TUSB32x initially has no `.github/workflows` directory; remote matrix work remains
prompt 34. Reviewed current root `CMakeLists.txt` and `test/CMakeLists.txt`:
strict private warnings, C++17, tests default OFF, Python only for tests ON,
registered CTest executables, finite test timeouts and assertions enabled in
Release. Adopt this existing local registration for `tusb32x_protocol` and keep
future API compile fixtures distinct from behavior. No peer build, remote run,
firmware link or physical qualification is claimed by this audit.

## Lead TI and pinned-source verification

The lead re-opened exact archived SLLSEN9F/SLLSEP2D/SLLSEQ8D/SLLSEO5D address,
pointer/read, identity, 45-presence and revision tables. Read printed pages
B15/16/19, L/A15/16/20 and V15/17/20 and visually checked all four identity
tables. Current official TI downloads match all four archived SHA-256 values;
all eight archived PDFs still match provenance. Current LAI/322I datasheets
were also opened through the web tool. No silicon fact or existing gate changed.

Re-fetched and verified the exact pinned SHA-256 source bytes for
`armoryctl-tusb320`, `betrusted-ec-tusb320lai` and
`xous-tusb320lai-emulator` from docs/reference/public-implementations.json.
Read the real sources: Go's fixed61 does not establish all address pairs;
Rust's unexplained06/retry loops do not establish revision acceptance or bounds;
Renode's two-byte pointer selection and first-byte-only identity responses do
not model TI's one-byte pointer/multi-byte or W1C behavior. These remain
counterexamples, with no source copied into production or normal packages.

One private invocation point now enforces the verified byte framing, local
allowlists and callback normalization/accounting. Test bodies supply independent
TI-derived literals. General coding agents own only the audit and fixed scripted
test files; the lead owns production/shared declarations, integration and Git.
The strong reviewer inspected exact production/test sources and independently
ran registered strict Release protocol checks. Final commands/results and
limits are in [validation](../validation.md).

## Independent prompt 9 re-audit, 2026-10-26

Fresh read-only inspection reopened the same named guidance, public headers,
transport implementations, fault tests/fakes, example adapters/CLI, HIL runners,
native registration and workflow source. All three peers remain clean on main:

| Peer | HEAD |
| --- | --- |
| PCA9555 | 498178e4d9e4acb6fab7d8b816ad5021efe8806a |
| ADS1115 | 235af3e8c722661a89a44766a4ccd903a654f6fe |
| LDC1614 | 1db38b63cb93a561b6e0bea51017ab94102ebd24 |

Existing choices remain sound: adopt exact fixed buffers/framing, backend detail,
conservative effects and independent fault injection. Reject PCA's richer
completed-count/GPIO-pair semantics, ADS's permissive callback codes/accounting
exclusions, LDC sensor/destructive-read assumptions and consumer-time Python.
No peer timeout parameter, callback or ACK probe proves TUSB framing, identity
or an aggregate SDK bound. No sibling was edited, fetched or built; no remote
workflow was dispatched or queried.

Lead and strong reviewer independently revisited archived TI address/framing/
map/identity/revision tables. Current B/L/A/V official downloads match their
archived hashes; all eight PDFs and the three cited pinned public source hashes
remain intact. Lead visually inspected the four identity tables and reread the
Go fixed-address, Rust revision06/retry and Renode pointer/multi-byte examples.
No conflict or gate change was found. Repeated native/package/documentation and
supplemental edge/intentional-defect checks are recorded in validation; the
private production boundary and test suite required no correction.
