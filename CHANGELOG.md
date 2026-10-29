# Changelog

User-visible library, example and tooling changes are recorded below. Software
verification and linked firmware remain separate from physical qualification.
The source checkout keeps detailed execution history in
[implementation progress](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/implementation-progress.md)
and [validation](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md).

## Unreleased

- Archived the completed implementation prompts and made active guidance about
  maintaining the library. Simplified the overview and contribution guides, and
  added software-validation setup instructions for work without a board.

- Added source-checkout board design and first-board bring-up worksheets, plus
  researched, unsent TI clarification requests. Corrected the TUSB322I attached
  current wording without changing any silicon capability gate.

- Closed final integration gaps: Arduino/native-IDF examples now expose primary
  callback-kind evidence for readonly/soak plans, and Arduino rejects further
  commands after suspected USB receive loss while existing owner work continues.
  Added application regressions and reconciled final review/package records.

- Completed independent core review and protocol, owner and portability checks.
  Corrected the peer inventory and validator names in the conformance matrix.
  No executable core change was needed; physical qualification remains separate.

### Added

- C++17 core with fixed storage, Status helpers, validation and register decoding.
  Supports six explicit TUSB320 variants and part of TUSB322I, with separate
  address and register rules. GPIO-only devices remain outside the API.
- Cooperative operations for initialization, profile observation and apply,
  reconciliation, reset, original-part dead-battery recovery, port refresh and
  IRQ service. One active operation and one retained result use instance-local
  tokens, fixed deadlines and a default limit of one callback per poll.
- Raw diagnostics limited to one callback, transport counters and passive health.
  Clearing statistics does not restore configuration trust or discard results.
- One shared 55-row CLI with strict parsing, separate staging, cached/live
  diagnostics and machine records. Bounded console buffers report record loss
  and preserve results when output cannot be reserved.
- Application-owned Arduino Wire and native ESP-IDF adapters with documented
  synchronous configuration, aggregate timeout accounting and controller retirement.
  Arduino CLI/two-device and native CLI examples link for S2/S3 in the audited
  configurations. Native SDK rows are 5.5.5 and 6.1.0; exact pins and prerequisites
  are in the [development guide](docs/development.md).
- Runnable ordinary C++ host consumer and generic two-device owner fixtures,
  using explicitly simulated transport and public APIs without product dependencies.
- Python sessions and nine bounded runner suites: readonly, soak, orientation,
  accessory, role-current, interrupt, reset-termination, fault and console.
  Hardware changes require explicit setup and identity. Artifacts retain the
  operation outcome, cleanup result and remaining uncertainty separately.
- Version sync/check/bump/set tools use `library.json`; example firmware identity
  is generated separately. Package checks validate archive contents and build
  an external consumer without adding maintainer tools to core requirements.
- One local validation entry point over registered native, contract, Python,
  Doxygen and package checks, plus declared extracted firmware links. Negative
  fixtures prove malformed boundary/version/documentation/CLI/archive inputs fail.
  Dedicated CI uses strict Debug/Release host builds, Linux sanitizers, immutable
  pins and bounded artifacts. Firmware jobs share one identified archive.

### Fixed

- Failed writes retain possible hardware effects and original backend detail,
  including NACK/BUSY outcomes. Cancellation, timeout and end preserve those
  effects. Fresh clock samples after callbacks start silicon waits without
  extending deadlines or shortening required intervals.
- Profile writes preserve actual controls rather than staged desired settings.
  Unchanged gated TUSB322I fields permit independent supported changes; semantic
  DRP equivalence does not authorize replaying an obsolete literal write image.
  Reset, mode and recovery obligations remain independent of ordinary settings trust.
- Port decoding separates detached, not-applicable, unknown and reserved data.
  Coherence reads invalidate fields that changed during observation. IRQ ACK
  preserves actual controls; later failure retains the sample taken before ACK.
- CLI malformed input cannot be edited into a valid command, lost records cannot
  become truncated success, and failed output reservation leaves the core result
  queryable. Automatic work still advances under cached/rejected command turns.
- Sessions retain original request/token ownership across interleaved responses,
  distinguish result delivery from payload outcome and reject late success,
  framing loss, reboot and identity changes. Cleanup cannot erase a primary
  failure or replay uncertain hardware changes.
- Planning a target power cycle revokes base-part recovery readiness across
  suites and sessions until a newly qualified typed recovery completes.
  Clear registers, identity checks and unrelated reads do not restore readiness.
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
