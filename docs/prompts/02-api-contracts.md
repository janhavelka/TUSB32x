# P02 - Define and compile the portable API contracts

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[MAINTAINABILITY.md](MAINTAINABILITY.md) and [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md).
Use P00 inventory and P01 silicon evidence. This stage makes declarations and
contracts reviewable; it does not claim a working driver. Update documentation
and source Doxygen as each declaration is introduced.
Re-audit current sibling code, tests, examples, runners and CI before edits.

## Fresh evidence

Re-read public headers, implementations and matching tests in LDC1614 and
LSM6DS3TR for Status, owner time, tokens, results and passive diagnostics; ADS1115
and INA3221 for transport/profile/effect separation; TMP1x2 for explicit bool
Status and exact-token take; PCA9555 for mixed-register writes. Inspect their
actual examples/CLI, Python runner output, test registration, AGENTS and CI.
Read FieldCore I2cOwnerTransport/I2cDeviceBinding/I2cLifetime as consumer evidence.
Record current paths, revisions, dirty states and specific adopted/rejected
behavior. Compare at least two useful peers for each material convention.
The baseline's dated source table gives precise starting files, not a substitute.

## Work 1: names, types and ownership

1. Use the baseline exact folder map, namespace/class TUSB32x::TUSB32x, Err list,
   Status representation/helpers, public signature vocabulary and typed settings.
   Keep PascalCase types, uppercase enum members, camelCase public fields/methods
   and _camelCase private members. No legacy aliases for a new API.
2. DriverConfig holds terminal callbacks/context, explicit variant/address and
   callback timeout. DeviceProfile holds only verified silicon settings. Health
   thresholds, pins, bus handles, scheduling and USB policy remain separate.
3. Define every public field's default, validity, unit and lifetime from the
   baseline and P01 capability matrix. Explicit variant is required; ACK does not
   identify exact silicon. Default objects cannot apply settings or advertise power.
4. Keep one facade/operation engine and small fixed aggregates. No worker,
   scheduler, factory, registry, polymorphic result hierarchy or platform base.
5. For each Err specify origin, output preservation, traffic and effect meaning.
   TIMEOUT is owner deadline; I2C_TIMEOUT is a terminal callback failure.
   CONFIGURATION_MISMATCH is successful transport with mismatching persistent bits.
   Preserve full backend detail and static-lifetime messages.

## Work 2: exact methods and admission

Use every method in the baseline signature table. No public begin/init/unbind
aliases. Write a table for validate/bind/end/invalidate/forget; each start method;
poll/cancel/resultSnapshot/takeResult; cached getters/stat clear; raw Sync methods.
For each give binding/identity/trust prerequisites, local failure codes, callback
and CPU bounds, writes/ACK/reset effects, output validity and allowed UNKNOWN state.

1. startInitialize is read-only identity/initial observation. startReadProfile
   reads without adopting desired settings. startApplyProfile explicitly writes;
   retain desired candidate at admission but keep applied/observed copies separate.
2. startVerifyProfile compares retained desired settings; startReconcile observes
   interrupted effects without replaying role/reset/termination writes. Share
   verification helpers. If transient completion cannot be proved, retain UNKNOWN.
3. startAcknowledgeInterrupt is explicit ACK-only; startServiceInterrupt adds
   observe/publish/ACK/bounded recheck. Both write. startReset does not reapply the
   desired profile. Raw Sync writes cannot bypass required typed role sequences.
4. Active operation owns one slot: conflicting admission BUSY. Terminal result
   owns the slot: RESULT_PENDING until exact take. Token mismatch cannot advance,
   cancel or consume represented work. Failed calls preserve output arguments.
5. Token is nonzero instance-local uint64, never reused through end/bind/forget;
   exhaustion is TOKEN_EXHAUSTED. resultSnapshot copies without consuming.
   takeResult returning OK means delivery succeeded; read result.status for outcome.
6. Choose deterministic error precedence for invalid input/binding/active/result/
   stale token and document it. Repeated exact take is RESULT_NOT_AVAILABLE.

## Work 3: state, time and evidence tables

1. Write separate lifecycle, operation, configuration, termination, reset, IRQ and
   health tables using baseline enums. CANCELLED with INDETERMINATE effect is
   valid. READY with configuration UNKNOWN is valid. Detach is normal.
2. Specify desired vs observed vs verified generation/publication points. Compare
   only persistent writable fields, excluding RO/W1C/reset. Unknown/partially
   applied state cannot be repaired by an unrelated successful read.
3. Invalid bind validates before replacement and preserves old usable state.
   end is bus-silent, retains result/effects and stops later callbacks. Define the
   single retained variant/address/context record and explicit evidence forgetting
   before target change with unresolved effects. Context reuse is not identity proof.
4. Use owner uint64 milliseconds, immutable OperationTiming and tested unsigned
   half-range comparisons for intervals below 2^63. Document backward-time input,
   zero/stalled time and proper extension of a wrapping 32-bit hardware clock.
5. Default poll invokes at most one terminal callback; zero budget none. Failed
   callbacks consume budget. Status-only transport has no completion timestamp:
   return after callback, then use a fresh next poll to check deadline/arm waits.
   Larger budget is a ceiling, never permission to use stale time.
6. Document aggregate callback timeout including lock and all bus phases, exact
   terminal buffer lifetime, no pending DMA, retries or reentry. Distinguish callback
   attempts from physical pointer-write/read transactions and actual adapter counts.
7. Publish ownership/lifetime and per-operation admission/transfer tables. P04/P05
   fill numeric ceilings from real register sequences; do not invent them now.

## Work 4: snapshots and passive diagnostics

1. Complete baseline PortSnapshot fields and typed decoding from exact P01 maps:
   raw bytes, validity, local/peer roles, orientation/current/accessory evidence,
   acquisition time/sequence, config provenance and mixed-time qualification.
2. Stage complete observations before software publication. Failed second read
   retains old valid snapshot/age. Successful burst is not hardware atomic capture.
3. Retain IRQ pending/ACK uncertainty independently; no required event queue or
   callback dispatcher. GPIO observation is optional application evidence only.
4. Specify all-callback TransportStats and explicitly filtered managedTransport;
   saturation, lifetime and clearing do not repair trust or reset live failure streak.
   Optional health is one pure projection with the baseline exact transition table,
   separate HealthPolicy and unchanged output on invalid threshold. OFFLINE never
   rejects legal work or initiates retries/recovery.

## Deliver and check

- docs/architecture.md: chosen folder responsibilities and owner/transport boundary.
- docs/api-contracts.md: exact declarations, errors, admission, all state tables,
  output/lifetime, timing, transfer/CPU bounds and result/effect examples.
- docs/design-decisions.md: source/revision evidence and intentional peer differences.
- Compile-intended passive-observation, explicit-apply and cancelled-write snippets;
  two devices on one externally serialized fake owner, without FieldCore headers.
- Declaration/compile-contract fixture using only C++17; identify it as a contract
  check, not protocol or hardware validation. P03 provides real build registration.
- Updated feature/API/source matrix, progress and validation with actual commands.

Add full public file/type/member/enum/function Doxygen now, including parameters,
units, returns, no-I2C/callback bounds, side effects and lifetime. Source briefs and
comments later explain invariants and register evidence rather than assignments.
Check available C++17 compilation and local documentation links/Doxygen. Report
unavailable tooling as NOT RUN. A new implementer must not have to invent the
names, ownership, result retention or effect semantics after this stage.
