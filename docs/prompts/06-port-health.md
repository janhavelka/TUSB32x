# P06 - Implement port evidence, explicit IRQ service and passive diagnostics

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[MAINTAINABILITY.md](MAINTAINABILITY.md) and [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md).
Implement src/TUSB32xPort.cpp and src/TUSB32xDiagnostics.cpp through P04 helpers
and P05's one operation engine. Keep physical USB policy in the application.
Re-audit current sibling code, tests, examples, runners and CI before edits.

## Fresh evidence

Read INA228's real interrupt observe/acknowledge code and tests; LDC1614
TransportStats/wrappers; LSM6DS3TR diagnostics/raw/probe paths; TMP1x2
DriverState/HealthSnapshot/track; ADS1115/INA3221 passive health and effect
retention; BME280 config trust separately from communication success. Compare
at least two actual peers for each semantic/naming choice. Read their current
examples/CLI, runner parsers, test registration and CI. Record paths/revisions/
dirty state and filtered-vs-all traffic differences; names alone prove nothing.

## Work 1: typed observations and publication

1. Implement pure small decoders for every P01-supported attachment, orientation,
   current, accessory and active-cable encoding. Use the baseline exact names and
   validity fields. Reserved encodings retain raw bytes without fake CC1/default
   current/detach values or an invented transport failure.
2. Interpret attachment relative to the local port. Requested mode is config
   evidence; actual local source/sink/accessory comes from status. Preserve peer
   interpretation and variant-specific debug/charge-through distinctions.
3. Source advertisement and local-sink current detection are different facts.
   Neither is measured current, PD or board power authorization. Do not convert
   default current unconditionally to 500 mA. Preserve unresolved attached-current
   tracking wording from the TI audit as a documented limit.
4. Stage required reads in an internal candidate; publish PortSnapshot only when
   complete. Failed later reads retain previous valid snapshot and age, not detach.
   Partial raw diagnostics may remain separately with explicit validity.
5. Snapshot includes raw bytes/register validity, typed field validity, sequence,
   acquisition owner-time/time validity, config generation/trust and mixed-time
   coherence. Software publication is atomic under owner serialization; hardware
   register capture is not asserted atomic by a burst.
6. Changed bookends get only the documented finite recheck budget. Report remaining
   uncertainty; never spin until stable. Sequence counts published observations,
   not every physical edge.

## Work 2: ACK-only and full IRQ service

Use startRefreshPort, startAcknowledgeInterrupt and startServiceInterrupt through
the same token/deadline/result engine. Cached getters never read or ACK.

1. Keep pending observation, retained evidence, ACK acceptance and recheck separate.
   A read is not ACK unless exact variant documentation proves its behavior.
   Preserve pending IRQ in every unrelated control update.
2. ACK-only uses fresh observed actual control bits for deliberate W1C composition.
   Desired duty 30% with hardware duty 50% must ACK while preserving 50%. Never use
   the staged/desired image or replay RO/event bits as a write template.
3. Full service observes, retains/publishes, explicitly ACKs and performs bounded
   recheck. Each callback is a P05 owner step. Budget/deadline ends leave pending
   evidence/progress visible. No repeated ACK loop or automatic follow-up job.
4. Failed ACK may have cleared the latch. Retain uncertainty. Readback can establish
   current state but cannot prove no event disappeared between phases. Coalescing
   latch cannot provide lossless edge history.
5. Polling works without IRQ pin. Optional owner-supplied GPIO level has its own
   timestamp/provenance, not a claim of every edge. Core owns no ISR/GPIO; example
   ISR signals owner only, with no I2C or logging.
6. For supported TUSB322I, separate VCONN fault evidence from IRQ ACK. Preserve
   unresolved RCU access wording from P01; the abbreviation alone does not prove
   clear-on-read behavior or justify a destructive-read guarantee.
7. Initial API needs no event queue/dispatcher. If a fixed buffer is justified,
   specify capacity/overflow/ownership first; no heap or app callbacks in transport.

## Work 3: neutral counters and diagnostics

1. Implement baseline TransportStats totalAttempts/totalSuccess/totalFailures/
   lastStatus with saturating uint32 counters. One wrapper counts every callback,
   including initialization/raw/probe/ACK/reset, once. Count before/after actual
   invocation, not per API call or expected physical bus transaction.
2. Normalize illegal callback IN_PROGRESS to I2C_ERROR preserving detail; no pending
   transfer may retain buffers. A failed callback consumes one attempt, independently
   of the later operation outcome or cancellation.
3. DriverDiagnostics has explicitly filtered managedTransport evidence; production
   callbacks contribute, raw diagnostics do not. All callbacks still appear in
   TransportStats. No hidden exclusions or duplicated source counters.
4. Local invalid/BUSY, deadlines without callback, waiting, cancellation, profile
   mismatch and detach do not increment I2C failures. Callback I2C_BUSY does.
5. Keep last failed Status/detail/time/time validity after later success. lastStatus
   may show success. Never invent completion time. Actual adapter byte/transaction
   counts, if exposed, stay separate from core callback units.
6. clearTransportStats resets source cumulative totals/lastStatus only. It does
   not reset live managed failure streak, last-failure evidence, initialization,
   trust, port age, results or effects. At saturation arithmetic equality of totals
   is not guaranteed; test it rather than wrapping.

## Work 4: one optional pure health projection

Use baseline DriverState UNINIT/READY/DEGRADED/OFFLINE, separate HealthPolicy
(default threshold 3; valid >=1), HealthSnapshot and Status-returning projectHealth
with out unchanged on invalid policy. No second health state machine.

| Evidence | State |
| --- | --- |
| No completed read-only initialization, end, external invalidation | UNINIT, including failed initialization with recorded errors |
| Initialized with zero consecutive managed failures | READY |
| Initialized with failures 1..threshold-1 | DEGRADED |
| Initialized with failures >=threshold | OFFLINE |
| Managed callback success after initialization | Streak 0, READY |
| Raw callback, no-transfer call, detach, cumulative counter clear | Unchanged |

Helper performs no callback, state mutation, delay, recovery or scheduling.
READY can coexist with config UNKNOWN after successful mismatching reads.
OFFLINE never prevents legal work. Threshold changes affect display only;
thresholds are not silicon capabilities. Applications may ignore this helper.

## Behavioral tests and completion

Add/register test/test_port.cpp and test/test_diagnostics.cpp using fixed fakes:

- Every variant attachment/orientation/current/accessory encoding and reserved value;
  local-vs-peer, not-applicable, invalid field and raw preservation semantics.
- Failed second read, changing bookends, old snapshot retention and no fake detach;
  sequence/age/generation/coherence and bounded publication.
- Pending IRQ preserved by every setter; ACK applied 50% vs desired 30%; failed ACK,
  reassertion in every phase, coalesced events and bounded stuck-IRQ handling.
- Budgets 0/1/larger, deadline/cancel after ACK, no-pin polling and no lossless claim.
- Every callback counted once, raw/probe all-source inclusion and managed exclusion,
  callback I2C_BUSY vs local BUSY, illegal callback progress, no-I2C preconditions.
- Full health table: threshold 1/default/max, invalid policy unchanged output,
  failed init, successful managed callback after OFFLINE, saturation, retained
  last error, clear invariance and legal diagnostic while OFFLINE.
- Multiple instances share one bus without shared counters or effect/result state.

Update Doxygen for every public type/member/enum/helper/method, including current
validity/units, non-consuming vs ACK behavior, counter units/lifetime and projection
limits. Add source briefs and comments explaining masks, staged publication,
coalescing and filtered accounting. Keep plain engineering English.

Update docs/api-contracts.md, docs/architecture.md, docs/variants.md,
docs/interrupts.md, docs/diagnostics.md, decisions and the feature/source/test
matrix. Record exact focused native and applicable Doxygen/link check results in
progress and docs/validation.md now. Physical IRQ wiring, connector orientation,
dynamic-current tracking and electrical USB behavior remain separate HIL evidence;
host decoder tests cannot establish them.
