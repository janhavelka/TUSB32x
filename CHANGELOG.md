# Changelog

This checkout contains audited implementation contracts, portable C++17 values
and a cooperative owner engine with read-only initialization, profile observation,
explicit profile application, desired verification, read-only reconciliation,
explicit reset, original TUSB320/I dead-battery recovery, complete port refresh,
explicit IRQ acknowledgement/bounded service, safe raw diagnostics and passive health.
Entries below distinguish those artifacts from planned device-driver functionality.

## Unreleased

### Added

- Complete Arduino CLI and two-device owner examples for ESP32-S2/S3, with
  explicit source filters and build identity, read-only startup result ownership,
  shared controller serialization, deferred IRQ service and orderly shutdown.
  A fixed USB mailbox isolates SDK transmit stalls from owner progress. New host
  tests cover startup/CLI handoff, output backpressure, fairness and shutdown.
  Board readiness, SDK cleanup acceptance and timing qualification are explicit;
  default images provide diagnostics without assuming a connected fixture.

- Example-only Arduino Wire callbacks with borrowed ownership, aggregate SDK
  budget, shared nonblocking serialization, safe short-write handling, explicit
  failed-receive controller retirement and native 64-bit owner time. S2/S3 link
  fixtures and host contracts cover the adapter; board timing/electrical
  qualification remains separate from the complete board examples above.

- Independent CLI audit: group help into one contiguous section per topic, reject
  duplicate/late records after request closure, and cover take-response capacity
  when the original completion is still unannounced. Core ownership stays unchanged.

- Complete 55-row shared CLI with one-callback safe raw access, public pure
  decoding, cached IRQ, opt-in input reading, explicit invalidation and independently
  sourced health/transport evidence. Raw errors retain ATTEMPTED and signed
  detail instead of implying unchanged hardware. Configuration mismatch/time
  details and validity remain visible. Executable canonical/alias/help parity
  and broken-contract fixtures join the existing transcript registration.

- Managed CLI routes for initialization, observation, apply/verify/reconciliation,
  IRQ ACK/service, reset and original-part recovery through canonical core APIs.
  One owner poll honors automatic/explicit zero/one/N budgets without double
  stepping. Origin request/nonce/token survives later queries and markers; core
  retains its sole result until explicit take with complete output reservation.
  JOB poll errors and RESULT payload outcomes remain distinct from delivery OK.
  Shared C++/Python tests cover deadlines, ambiguous writes, cancellation, output
  loss and actual IRQ/control effects. Serial and electrical proof remain separate.

- Bounded example-only local CLI with strict whole-command parsing, atomic target
  edits, explicit evidence forgetting and validated independent profile staging.
  The fixed console preserves poisoned input through newline and queues complete
  records only, with 8192 output bytes, 512-byte formatted storage and one 256-byte
  nonblocking drain per turn. Help is incremental; machine records carry typed
  status/detail, request/nonce correlation and nonwrapping exhaustion evidence.
  Registered CLI/console suites and a persistent C++/Python transcript execute the
  actual shared sources. Device commands and firmware front ends remain later work.

- Core owner portability checks: fixed-capacity two-device scheduling, queue/result
  backpressure, late callback timing/error retention and bounded generation
  saturation. A registered isolated public consumer runs every current facade
  method with allocation traps and deliberate failure sentinels in Debug/Release.
  Ordinary consumers still need only CMake and a C++17 toolchain.

- Independent prompts 16-18 audit: prevent stale TUSB322I DRP mode bits from
  entering raw or typed 0A writes after observing an equivalent 00/11 recoding.
  Keep semantic trust separate from literal write-image permission and renew the
  latter only through complete proof. Added both-direction regressions and 322I
  DFP/UFP failures and interruptions at every write/callback boundary.

- Protocol conformance matrix mapping TI/software requirements to named runtime
  cases. Added public operation checks at both legal addresses for every supported
  variant, both hardware outcomes of failed raw writes/ACKs, and unrelated-read
  trust-retention checks. The scripted fake now rejects an incomplete receive
  falsely marked successful. No production behavior or silicon gate changed.

- Bounded `readRegisterSync`/`writeRegisterSync` using the existing terminal
  callback/accounting path and safe composer. Raw writes require current actual
  proof, reject procedure shortcuts and invalidate trust before possible effect.
  Contradictory protected45 observations revoke its write image for both raw and
  typed changes while leaving independent settings available. Diagnostics retain
  managed operation and unresolved effect history after result delivery.
- Pure `validateHealthPolicy`/`projectHealth` with positive threshold (default3),
  UNINIT/READY/DEGRADED/OFFLINE, unchanged invalid-policy output and no admission or
  recovery policy. All-source counts include raw callbacks; managed health excludes
  them. Source-stat clearing preserves managed streaks, trust, effects and results.
  Registered diagnostics tests cover public raw safety/accounting and projection.
  Removed the now-empty future API fixture facade; signatures use real headers.

- Independent prompt-16 audit and service regressions for controls that change
  between the two 09 reads, final-sample ACK composition, and changed-bookend
  field validity through ACK/recheck failure. Both targeted faulty implementations
  passed the old tests and are rejected by the new cases. Production behavior
  remains unchanged; corrected stale API scope and the test-file inventory.

- Explicit `startAcknowledgeInterrupt` and `startServiceInterrupt` through the
  existing owner engine. Fresh actual controls determine the safe W1C byte;
  staged desired controls never enter ACK. ACK-only is bounded to two callbacks,
  service to complete 09/08/09 publication, one ACK and one recheck (five).
  Complete pre-ACK data survives later failure. Sampled-before, ACK attempt,
  callback acceptance and sampled-after remain independent; no lossless edge
  count or automatic reservice is implied. Current identity is required, UNKNOWN
  configuration allowed, and TUSB322I rejects both before traffic under its
  unresolved fault/09-write gate. GPIO/ISR and follow-up policy stay application-owned.

- Independent prompt-15 audit and regression for configuration contradictions
  first observed in the final port read, including cancellation, deadline and
  backward-time rejection. Clarified implemented port support, configuration-only
  proof limits and the IRQ acknowledgement/service evidence boundary.

- Read-only `startRefreshPort` through the existing owner engine and cached
  `portSnapshot` publication. Fixed 09/08/09 reads preserve final raw status,
  variant-aware role/current/accessory applicability, per-field validity,
  bookend coherence and original acquisition/configuration provenance. Failed
  partial reads retain the previous complete snapshot; complete publications
  advance a saturating sequence. Invalidation preserves historical observations.
  Added the registered port regression suite. Attached-current tracking,
  connector mapping and electrical behavior
  require separate physical qualification.

- Independent prompt-14 audit and regressions for fresh reset controls differing
  from cached proof, post-reset identity/revision contradictions and clipped
  callback timeouts. Corrected stale class Doxygen and execution progress labels.
  The existing production behavior passed the additional checks unchanged.

- Linked `startReset` and base-only `startRecoverDeadBattery` through the existing
  cooperative engine. Reset preserves actual controls in its one-shot command,
  waits 96 elapsed owner ticks and verifies once without desired replay. Recovery
  requires caller-established stable VDD and valid identity, qualifies internal
  45 bits, writes literal 04/00 around a fresh 31-tick wait and verifies readback
  within four callbacks. Failures preserve original status/detail, possible effects
  and independent interrupted-sequence history through cancellation, result take
  and rebind. Other variants reject recovery without traffic; TUSB322I reset
  requires semantic DRP proof. Added literal-byte, timing and fault-boundary
  regressions; software procedures do not establish hardware qualification.

- Independent prompt-13 audit and permanent literal-byte regressions for UFP
  role changes and settings-only 0A updates. These close gaps demonstrated by
  isolated faulty implementations that the earlier operation suite did not
  detect. No production behavior or public API changed.

- Linked `startApplyProfile` and read-only `startVerifyProfile` through the
  existing owner engine and complete identity/control proof. Apply retains a
  validated desired candidate at admission, changes only differing fields and
  verifies no-op requests without writes. Per-variant role sequences preserve
  actual controls, independent termination settings and fresh-time wait anchors.
  Failed/cancelled operations retain effects and backend detail without cleanup,
  rollback or replay. Complete mismatching verification can establish KNOWN
  actual settings while desired stays unchanged. TUSB322I advertisement/09 writes
  and direct return to DRP remain gated. Added exact-byte and write-boundary
  operation regressions; hardware qualification remains separate.

- Independent prompt-12 audit and public-operation regression vectors covering
  every valid profile setting after complete observation/reconciliation. The
  audit found no runtime defect; production behavior and API are unchanged.

- Linked `startReadProfile` and read-only `startReconcile` through the existing
  token/result engine. Profile observation preserves raw/typed validity and can
  disprove old trust without promoting new controls. Complete reconciliation
  proves actual identity/configuration and termination/reset evidence, including
  a bounded wait for retained reset uncertainty; it never replays writes or adopts
  desired settings. TUSB322I readable controls remain independent of write gates.
  Apply and desired-profile verification were added separately in prompt 13.

- Non-copyable owner facade with atomic bus-silent binding/lifecycle,
  nonreused uint64 operation tokens, one retained terminal result and exact
  snapshot/take delivery. Added read-only identity/configuration initialization,
  bounded token-checked polling, callback deadline clipping and fresh owner-time
  completion. Cancellation, end and external invalidation preserve results and
  historical evidence. Added registered owner-operation and public-link checks;
  profile writes, reset/recovery, port/IRQ jobs and public raw access remain planned.

- Public pure `decodeRegister` with raw/field validity and unchanged output on
  errors; private typed encoders, semantic settings comparison and one qualified
  write composer. Added exhaustive TI-derived codec tests and exact composed
  transport frames. TUSB322I write gates remain; owner procedures and timing,
  complete port applicability and public raw diagnostics are later work.

- Private register transport with one terminal callback/accounting boundary,
  fixed receive scratch storage, exact variant/register/range gates, conservative
  write-acceptance evidence and pure identity/revision comparisons. Added genuine
  scripted protocol tests; managed operations and public diagnostics remain planned.

- Development metadata at 0.1.0-dev.1 with library.json as version truth,
  deterministic sync/check/bump/set tooling, generated constexpr version values
  and isolated registered fixtures. Consumer builds need no version tool or Git.
- Explicit-source native IDF component registration, managed Windows PlatformIO
  launcher and pinned real native consumer environment. Added MIT/ownership and
  formatting files, strict Doxyfile and explicit package content boundaries.
  Linked framework firmware and full distribution qualification remain planned.

- Portable C++17 CMake target `TUSB32x`, pure binding/profile validators with
  distinct invalid/unsupported results, registered native contract/validator
  checks and a standalone multi-file host consumer. Tests default OFF; ordinary
  linkage needs no Git, Python, Doxygen, framework or sibling dependency.
- Native build/consumer instructions and contributing guidance. Device operations,
  firmware and hardware qualification remain planned.

### Changed

- Independently audited prompt 11 against its original requirements and diff.
  Corrected bind's documented validation errors and added an all-variant
  regression proving that clear read-only reinitialization cannot erase retained
  reset uncertainty. No production behavior or public signature changed.

- Independently audited prompt 10 against the original prompt, actual diff,
  current peers and archived TI tables. Repeated strict native, codec, consumer,
  package and documentation checks; no implementation correction was needed.

- Independently audited prompt 9 against the original requirements, actual diff,
  current peers and TI sources. Repeated strict native, protocol, package-consumer
  and documentation checks; supplemental edge cases and deliberate defect
  injection confirmed test coverage. No implementation correction was needed.

- Independently audited prompt 8 against its original requirements and source
  diff. Reject alternate-form duplicate version declarations, preserve launcher
  exit codes when ERRORLEVEL is inherited, and make packaged guide links usable.
  Added generator and Windows launcher regression checks and repeated native,
  package-consumer and documentation verification.

- Independently audited prompt 7 against its original requirements, source diff
  and current peers. Repeated native, consumer and documentation checks; corrected
  a TI table citation, repaired damaged evidence ranges and clarified Release
  selection for Ninja consumers.
  The validators and build behavior needed no correction.

- Independently audited prompt 6's contracts, values and fixtures against the
  original requirements and current peer code. Repeated local checks and added
  the reproducible fractional-time verification command; no code or API-contract
  correction was needed.

- Finished owner admission, state, time, publication and health contracts, with
  explicit result/effect retention and future failure vectors. Separated identity
  readiness from configuration trust to allow explicit interrupted recovery
  after rebind and read-only initialization. No driver methods are published yet.

- Independently audited prompt 5's actual headers, contracts and fixtures against
  the original requirements and current peers. Repeated strict compilation,
  linked Status tests, Doxygen and documentation checks; no code or API correction
  was needed. Recorded the review evidence and remaining implementation scope.

- Independently audited prompt 4 against TI sources, current peers and the
  committed diff. Made reset/ACK formulas explicit by variant, fixed the ACK
  table's unescaped bitwise-OR delimiter, and repeated procedure, timing,
  provenance and documentation checks; no silicon procedure
  or transfer bound changed.

- Independently reviewed prompt 3 against TI tables, source code and literal
  vectors. Clarified counted reads before recovery rejection and corrected the
  PCA9555 runner's evidence-retention description. Register encodings and silicon
  capability gates are unchanged.

- Independently reviewed prompt 2 against TI tables, pinned public sources and
  current peers. Added the TUSB322I VCONN supply prerequisite and extended its
  fault gate to all shared-register writes; completed missing visual table checks
  and clarified the TMP1x2 ARA source reference.

- Verified family capabilities against current TI sources and pinned public code.
  Corrected the TUSB322I VCONN resistance to a maximum, recorded LI/HI product
  recommendations and exact electrical conflicts, and selected strict revision
  acceptance without inferred capabilities.

- Independently reviewed execution prompt 1 and corrected peer console/lifetime
  claims, an API source citation, Doxygen fixture expectations and validation
  provenance. The chosen baseline and later prompt scopes remain unchanged.

- Audited all 38 prompts and shared contracts for correctness and missing
  requirements. Corrected TUSB322I role sequences and base attached-mode limits;
  specified raw profile validity, target association, reinitialization, no-op
  apply, time boundaries, callback error normalization and fixed port sampling.
- Added explicit CLI evidence forgetting and atomic variant/address selection,
  runner ownership/local-state restoration, SDK timeout and synchronous-handle
  checks, isolated IDF configuration/component discovery and Windows-safe archives.
- Kept the 38 bounded prompt scopes, local validation policy, source documentation
  and per-prompt commit/sync requirements; added a source-backed findings report.

- Confined remote CI execution and status checking in the first implementation
  promptset to prompt 34. Other prompts and final-review fixes use local checks;
  commit/sync requirements remain, and older CI runs retain their actual SHA.

- Require exact-source verification of inherited chip facts, explicit conflict
  handling and corrections across contracts, code, tests, Doxygen and guides.
- Add targeted public-code failure cases and TI electrical/timing/variant facts
  to the relevant numbered prompts while retaining the 38-task structure.

- Split the original fifteen stages into 38 complete prompts numbered 1–38,
  with dependency-aware scopes, tests/docs/delegation and per-prompt commit/sync.
- Shortened the common entry instructions and separated technical rules by topic;
  added explicit incremental API, prerequisite and incomplete-session boundaries.
- Corrected chip/protocol, snapshot, reconciliation and CLI result/correlation
  instructions after an independent promptset audit; added explicit original
  TUSB320/I dead-battery recovery to the planned API.
- Simplified repeated diagnostics into host-runner sequences and reduced duplicate
  firmware state, grammar, documentation, audit and build requirements.
- Clarified branch lifetime: create one new branch for each new promptset
  execution, then retain it through all prompts, sessions and agents.
- Expanded AGENTS.md using current sibling Git, CI, ownership and engineering
  rules, adapted to the future TUSB32x library and its concrete contracts.
- Required commits and verified upstream synchronization after every prompt and
  at coherent completed units within prompts, on one branch for the promptset.
- Expanded the fifteen implementation stages with concrete peer source references,
  selected API/error/state names, file responsibilities and test expectations.
- Made readable, simple code and plain engineering language requirements for
  implementation, comments, documentation, CLI messages and agent handoffs.
- Required source Doxygen and affected README, changelog and guide updates during
  every stage; the documentation stage now checks consistency rather than starting
  documentation at the end.

### Added

- Compile-only passive observation, explicit apply/cancellation and two-device
  owner examples, plus independent unsigned-time and clock-extension vectors.

- Implemented portable C++17 Status, callback/settings, operation/snapshot,
  diagnostic/health and register-decode result values with complete Doxygen.
- Added the exact planned API inventory, decoder boundary and architecture guide,
  plus reusable standalone-header, constexpr and multiple-translation-unit checks.
  Future driver methods remain outside delivered headers until implemented.

- Completed prompt 4 transport and device procedures: exact role changes,
  one-shot reset, original-part recovery, IRQ service, wait anchors, callback/byte
  ceilings and independent failure/time vectors. Added explicit internal-register
  qualification and retained-effect rules; unresolved silicon gates remain.

- Completed prompt 3 register access, field encodings and independent positive/
  negative vectors. Distinguished partial TUSB320/I defaults from complete
  extended reset tables, protected internal bits from reserved read-only bits,
  and readable profile settings from write permissions. Preserved TUSB322I
  access gates and equivalent DRP encodings; no driver behavior is claimed.

- Completed prompt 2 variant/source boundary and feature-to-test matrix, with
  named unresolved gates and board prerequisites. Added the separate TUSB321AI
  source and original-terms/current-verification metadata for all eight TI PDFs.

- Completed execution prompt 1: current inventory of 17 I2C libraries and the
  FieldCore consumer, source-backed engineering decisions, local tool inventory
  and a 38-prompt progress ledger on one recorded execution branch. Driver code
  and later prompt work remain unimplemented.

- Shared chip-evidence reference and TI-first public implementation audits,
  including reset-mask, reserved Try.SRC, error-preservation and IRQ examples.
- Provenance for 13 pinned public source files from six repositories, including
  ownership/license evidence, commit/blob IDs, byte counts and SHA-256 hashes.
- Additional TI evidence for addresses, identity, register access, supplies,
  physical timing, current refresh, 322 reset-to-DRP and unresolved source claims.

- Engineering baseline for Status, owner methods, transport/health evidence,
  configuration trust, results/effects and public naming conventions.
- Shared CLI/tooling contract with concrete grammar, output records and verification.
- Complexity-based subagent/model selection and independent review guidance.
- Doxygen omission checks, source documentation rules and explicit simplicity gates.

## Initial planning — 2026-10-23

- Audited seventeen sibling I2C libraries and FieldCore-node as a consumer.
- Prepared the original fifteen-stage promptset and recurring-audit contract.
- Archived seven official TI references with retrieval metadata and SHA-256 hashes.
