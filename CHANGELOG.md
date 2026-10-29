# Changelog

User-visible library, example and tooling changes are recorded below. Software
verification and linked firmware remain separate from physical qualification.
The source checkout keeps detailed execution history in
[implementation progress](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/implementation-progress.md)
and [validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md).

## Unreleased

- Added source-checkout board design and first-board bring-up worksheets, plus
  researched, unsent TI clarification requests. Corrected the TUSB322I attached
  current wording without changing any silicon capability gate.

- Closed final integration gaps: Arduino/native-IDF examples now expose primary
  callback-kind evidence for readonly/soak plans, and Arduino rejects further
  commands after suspected USB receive loss while existing owner work continues.
  Added application regressions and reconciled final review/package records.

- Completed independent core review with fresh protocol/owner/portability checks;
  corrected stale peer inventory and validator naming in the conformance matrix.
  No executable core change was needed. Integration/distribution review and
  physical qualification remain separate.

### Added

- Framework-neutral C++17 core with fixed-storage values, constexpr Status
  helpers, pure binding/profile validators and public register decoding.
  Six explicit TUSB320 variants and partial TUSB322I support use separate
  address, register and capability rules; GPIO-only devices remain outside the API.
- Cooperative owner operations for read-only initialization/profile observation,
  explicit apply/verification, read-only reconciliation, reset, original-part
  dead-battery recovery, port refresh and bounded IRQ acknowledgement/service.
  One active operation and one retained result use instance-local tokens,
  immutable deadlines and at most one callback per poll.
- Safe one-callback raw diagnostics, saturating all-source counters, separate
  managed transport evidence and pure passive health. Statistics clearing does
  not repair trust, discard effects or consume a result.
- One shared 55-row CLI with strict whole-command parsing, independent staging,
  core-owned operation/results, cached/live diagnostics and typed machine records.
  Bounded console input/output, complete-record loss reporting and result-take
  reservation preserve evidence under backpressure.
- Application-owned Arduino Wire and native ESP-IDF adapters with documented
  synchronous configuration, aggregate timeout accounting and controller retirement.
  Arduino CLI/two-device and native CLI examples link for S2/S3 in the audited
  configurations. Native SDK rows are 5.5.5 and 6.1.0; exact pins and prerequisites
  are in the [development guide](docs/development.md).
- Runnable ordinary C++ host consumer and generic two-device owner fixtures,
  using explicitly simulated transport and public APIs without product dependencies.
- Strict Python sessions, streamed bounded evidence and nine finite runner
  families: readonly, soak, orientation, accessory, role-current, interrupt,
  reset-termination, fault and console. Fixture mutations require explicit setup
  and identity; primary outcome, cleanup and residual uncertainty stay separate.
- Deterministic manifest-based sync/check/bump/set version tools, stable generated
  package version and separate example firmware identity. Literal package rules,
  safe archive checks and clean renamed consumers preserve required shared files
  without adding maintainer dependencies to core consumption.
- One local validation entry point over registered native, contract, Python,
  Doxygen and package checks, plus declared extracted firmware links. Negative
  fixtures prove malformed boundary/version/documentation/CLI/archive inputs fail.
  Dedicated CI uses strict Debug/Release host builds, Linux sanitizers, immutable
  pins and bounded artifacts. Firmware jobs share one identified archive.

### Fixed

- Failed writes retain possible acceptance and original backend detail, including
  NACK/BUSY outcomes. Cancellation, timeout and end do not undo effects. Fresh
  post-callback owner samples anchor completion and silicon waits without renewing
  deadlines or shortening required intervals.
- Profile writes preserve actual controls rather than staged desired settings.
  Unchanged gated TUSB322I fields permit independent supported changes; semantic
  DRP equivalence does not authorize replaying an obsolete literal write image.
  Reset, mode and recovery obligations remain independent of ordinary settings trust.
- Port decoding distinguishes detached/not-applicable, unknown and reserved data.
  Fixed read bookends withhold incoherent fields; historical bytes and timestamps
  cannot masquerade as fresh observations. IRQ ACK preserves actual controls,
  and service retains its complete pre-ACK sample after later failure.
- CLI malformed input cannot be edited into a valid command, lost records cannot
  become truncated success, and failed output reservation leaves the core result
  queryable. Automatic work still advances under cached/rejected command turns.
- Sessions retain original request/token ownership across interleaved responses,
  distinguish result delivery from payload outcome and reject late success,
  framing loss, reboot and identity changes. Cleanup cannot erase a primary
  failure or replay uncertain hardware changes.
- Base recovery readiness is revoked by a target power-cycle intent and survives
  suite/session boundaries until newly qualified typed recovery. Clear registers,
  fresh identity and unrelated reads cannot prove the historical sequence.
- Archive validation rejects portable-path aliases, links, collisions and bounded
  size/count violations before extraction. Clean-consumer provenance checks actual
  compiler headers as well as source files, including multi-configuration builds.
- Validation guards handle C++ digit separators, alias-hidden APIs and nontrivial
  pointer/reference helpers. Bounded logs and checkpoint errors preserve failed,
  incomplete or interrupted outcomes; suite CPU allowances do not alter protocol
  or operation deadlines.
- Validation and firmware-check children that finish at or after their original
  deadline now fail even with exit zero; their final waits use the remaining budget.
- Generated public API pages exclude private core types. Regression fixtures
  reject private API leaks while still checking private helper documentation.

### Documentation

- Reconciled README, navigation, owner/API guides, example commands and feature
  coverage with delivered behavior and exact TI evidence. The literal README
  quick start is compiled and executed under simulated success and failure.
- Audited public/core, adapter and shared CLI comments and generated API visibility.
  Clarified retained Status message lifetime, fresh wait anchors, initialization
  proof, callback waits and exact TI mask/timing citations.
- Aligned maintainer commands, delegation and release guidance with existing
  validation, documentation and version tools. Expanded Python input/deadline/
  artifact/exit contracts without changing executable behavior or package version.
- Kept unresolved TUSB322I access and board evidence limits explicit. Primary
  callback-kind evidence supports Arduino/native-IDF readonly/soak checks without
  claiming physical transactions. Physical serial, I2C/CC/INT, supply and timing
  qualification is not established by models, linked images or CI.
