# Prompt 1: ownership, API and consumer audit

Audited 2026-10-23 from current local source. This is source review, not a claim
that peer tests or TUSB32x builds ran. Siblings were read-only: no fetch, build,
installation or Git mutation. The selected contract remains
[ENGINEERING_BASELINE](../prompts/ENGINEERING_BASELINE.md); the findings below
confirm its choices and explain differences from the peers.

## Revisions and scope

All eight library repositories were on `main`; FieldCore-node was on `rodtester`.
All nine had empty `git status --porcelain` at inspection. Paths below are
relative to the named project beneath `Projects`.
Their current root `AGENTS.md` was inspected alongside source and tests.

| Project | HEAD |
| --- | --- |
| LDC1614 | `c0ab4c1712c4135b925354d70c4e6bed5cca62d8` |
| LSM6DS3TR | `513c5641d9b7dda63fddc27267ea62aa8d03ff93` |
| ADS1115 | `381236ebfed813cd7a9aca34520ce031d8a0ff6b` |
| INA3221 | `e45622dbaa5bb0b3151d980f30a1b6ea1cdb1a1b` |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9` |
| PCA9555 | `498178e4d9e4acb6fab7d8b816ad5021efe8806a` |
| BME280 | `d538f2e203ee26d6ddbc86f86d2b0cf4c73a6e98` |
| OPT4001 | `dab818e575fc25a0949d962bc0df600eb2ed5200` |
| FieldCore-node | `4e6a05e769383b95046df3936eee81984f4d9aa7` |

The FieldCore revision is newer than the historical audit. Its current owner has
routing, optional mux composition, parked jobs and exact application-cancellation
identity. These are consumer responsibilities; they do not justify adding a bus
manager or a FieldCore dependency to TUSB32x.

## Decisions from declarations, code and tests

The revision column uses the unambiguous short forms of the full revisions above.
Checks in the last column describe source evidence and the required future
TUSB32x acceptance case; none is reported as an executed driver test here.

| Concern | Peer file/symbol | Revision | Selected rule | Reason | Check |
| --- | --- | --- | --- | --- | --- |
| Status shape and spelling | LDC1614 `include/LDC1614/Status.h:35`, LSM6DS3TR `include/LSM6DS3TR/Status.h:46` | c0ab4c1 / 513c564 | `Status { Err code; int32_t detail; const char* msg; }`, `ok`, `is`, `inProgress`, `Ok`, `Error`; static messages | Established framework-neutral value contract | Preserve full negative backend detail through failure/result; distinguish owner TIMEOUT from I2C_TIMEOUT |
| Terminal callback and aggregate timeout | LDC1614 `include/LDC1614/Config.h:15`, BME280 `src/BME280.cpp:315` `mapTransportResult`; BME280 `test/test_basic.cpp:812` | c0ab4c1 / d538f2e | Borrowed synchronous callbacks, one whole-call timeout including lock and phases; reject short completion in adapter | BME280 carries byte counts; selected TUSB callback returns only Status | Callback never retains buffers; normalize nontransport statuses, preserve detail; short reads fail |
| Bus-silent admission | LSM6DS3TR `src/LSM6DS3TR.cpp:594` `bind`, `:633` `_checkStart`; ADS1115 `src/ADS1115.cpp` `bind`, `poll` | 513c564 / 381236e | Validate before commit; start/poll separation; only poll advances managed I2C | Shared owner must choose when the bus is used | Invalid bind/start preserves old state and issues zero callbacks; budget zero issues zero callbacks |
| One result and exact take | LSM6DS3TR `src/LSM6DS3TR.cpp:2364` `takeResult`; ADS1115 `src/ADS1115.cpp:1079` `takeResult` | 513c564 / 381236e | One active operation, one pending result, token match and explicit pending backpressure | Simplest fixed-memory contract; delivery success is separate from stored operation failure | Wrong token does not consume; second take fails; rejected start does not overwrite |
| Queue alternative rejected | LDC1614 `src/LDC1614.cpp:493` `_startJob`, `:615` `takeResult`; LSM6DS3TR `_checkStart` | c0ab4c1 / 513c564 | Do not copy LDC's FIFO, caller IDs or immediate replacement with a retained cancelled result | Adds capacity/identity policy without a TUSB requirement | Compare LDC `test_result_ids_fifo_backpressure_exactly_once_and_stale_clear` with LSM single-result tests |
| Cancellation and uncertain writes | LSM6DS3TR `src/LSM6DS3TR.cpp:2322` `cancelActiveJob`; INA3221 `test/test_owner_operations.cpp:629`, `:978` | 513c564 / e45622d | Cancellation is bus-silent; original transport error, partial effects and actual configuration remain separate | Cancelling software cannot undo accepted hardware writes | Fault/cancel every write boundary; preserve verified actual state before the first possible write |
| Teardown and lifetime | TMP1x2 `src/TMP1x2.cpp:78` `end`; LDC1614 `src/LDC1614.cpp:394` `end`; LSM6DS3TR `src/LSM6DS3TR.cpp:617` `unbind` | bafc340 / c0ab4c1 / 513c564 | Bus-silent end, no later use of borrowed callbacks; retain terminal result and uncertainty | TMP ends without power-down; LDC and LSM clear results on teardown, which is unsuitable here | End active/terminal operations, take retained result, confirm no old context access |
| Binding transactionality | TMP1x2 `src/TMP1x2.cpp:19` `bind`; LSM6DS3TR `src/LSM6DS3TR.cpp:594` `bind` | bafc340 / 513c564 | Validate candidate fully before changing the existing binding | TMP unbinds before validation; that does not satisfy selected TUSB behavior | Invalid replacement leaves old binding, snapshots, counters and evidence intact |
| Actual versus desired | LSM6DS3TR `cancelActiveJob`, `getDesiredProfile`; LDC1614 `updateDesiredConfig`, `_recordConfigFault` | 513c564 / c0ab4c1 | Distinct desired, observed/raw-valid, verified actual and unresolved effects | A new desired profile or unrelated read is not proof of hardware state | Apply no-op uses reads only; mismatch can still prove a different actual image |
| Passive health | INA3221 `test/test_owner_operations.cpp:1613`; LDC1614 `test/test_basic.cpp:2731`; TMP1x2 `src/TMP1x2.cpp` health wrappers | e45622d / c0ab4c1 / bafc340 | Optional UNINIT/READY/DEGRADED/OFFLINE projection; no admission or retry policy | INA and LDC tests explicitly request traffic after failures | OFFLINE still permits legal work; detach/local validation does not count as transport failure |
| Counter lifetime | LDC1614 `src/LDC1614.cpp:1049` `_tracked`, `bind`, `end`; TMP1x2 `src/TMP1x2.cpp:42` `unbind` | c0ab4c1 / bafc340 | Saturating totalAttempts/totalSuccess/totalFailures; lifetime totals survive end/rebind until explicit clear | Adopt names and saturation, not peers' lifecycle resets | Every callback once, raw diagnostics included; clearing totals cannot repair trust or managed health |
| Time model | LDC1614 `test/test_basic.cpp:1261`; INA3221 `test/test_owner_operations.cpp:300`, `:527`; LSM6DS3TR `_checkStart` | c0ab4c1 / e45622d / 513c564 | Owner uint64 time, immutable half-range deadline, one fresh post-callback boundary, separate 1 ms wait margin | LSM's direct deadline comparison is not the chosen rollover rule; a Status cannot supply completion time | Equal/backward/rollover/half-range tests; no wait starts at a pre-callback timestamp |
| Byte-register peers | PCA9555 `src/PCA9555.cpp:639` `_requestTerminal`; BME280 `src/BME280.cpp:315` `mapTransportResult` | 498178e / d538f2e | Adopt small typed register helpers, explicit partial-read/write evidence | Do not import PCA pointer-park cleanup or BME sensor calibration/measurement procedures | TUSB cancellation remains terminal and bus-silent; no sensor features or generic register framework |
| Unsafe peer behavior rejected | OPT4001 `src/OPT4001.cpp:334` `end`, `:667` offline admission; TMP1x2 `end`; INA3221 passive-health test | dab818e / bafc340 / e45622d | No silent shutdown write, OFFLINE gate or internal retry policy | OPT end discards a power-down write status; TUSB must preserve effects explicitly | End transcript is empty; a legal operation remains admissible after transport failures |
| Naming and portability | LDC1614 and LSM6DS3TR root AGENTS and public headers; FieldCore root AGENTS | c0ab4c1 / 513c564 / 4e6a05e | TUSB namespace/class, PascalCase types, camelCase functions/fields, _camelCase private members, CAPS_CASE enums/constants | Library peer vocabulary is a better fit than FieldCore's kPascalCase constants | Public/core headers compile without framework headers; no board or scheduler ownership |

## FieldCore-like owner integration

Reviewed current `include/TunnelMonitor/i2c/I2cTask.h`, `I2cOwnerTransport.h`,
`I2cDeviceBinding.h`, `I2cLifetime.h`, `src/i2c/I2cTask.cpp`,
`docs/guidelines/i2c_peripherals.md`, and
`test/native/test_owner_i2c/test_i2c_contracts.cpp`. The owner code and declarations
agree that bus setup, routing, admission, clock, cancellation correlation and
recovery belong to the application.

`I2cOwnerTransport.h:53` returns `I2cBusObservation` with status, backend error,
written/read byte counts, write effect, duration and `completedUptimeMs`.
TUSB's Status-only callback cannot carry all those fields. A future application
adapter maps the supported terminal status and complete backend detail, checks
byte counts, and retains richer application diagnostics outside the core. It must
not infer address NACK from FieldCore's generic Nack or pretend that pre-call time
is the callback completion time. Poll again with a fresh owner sample as the
baseline specifies. No broader TUSB callback API is justified by this consumer.

`I2cDeviceBinding.h:39` offers an optional bus-silent wait hint. TUSB's planned
progress snapshot is sufficient to build such a hint without scheduling in the
driver. The adapter retains the operation token and takes exactly its terminal
result before admitting replacement work. `I2cLifetime.h:14` demonstrates why
casting a wrapping 32-bit deadline to uint64 is wrong; TUSB uses the owner's
existing uint64 domain directly. The owner's outer request deadline must not
extend the accepted library deadline.

Current FieldCore `I2cTask::establishRoute` performs selector begin/select/readback
on separate turns and owns route trust. It is not a reason to count a TUSB
callback as one physical transaction. A combined callback can include more than
one physical transaction, and routing can add owner traffic outside it. Keep
callback counts and backend traffic counters distinctly named.

One configuration owner per physical target remains necessary. Bus serialization
alone does not prevent two owners from invalidating each other's cached settings.
The retained target tuple includes variant, address, context and both callback
pointers; changing a route inside reused context requires explicit invalidation
and the documented end/take/forget transition when changing target. Core code
does not inspect FieldCore routes, DeviceId, queues or module templates.

The generic C++ owner, bare-metal loop, Arduino adapter and native-IDF adapter can
all supply the same plain callbacks, fixed storage and monotonic owner time.
Their actual SDK timeout/lock bounds still require the implementation-stage
adapter audit. Function-pointer compatibility is not proof of bounded firmware
execution or physical hardware support.

## Limits and follow-up evidence

No missing owned source path prevented this review. All cited tests were read
as evidence of peer intent and behavior; they were not executed. The current
TUSB contract intentionally differs from peer result-clearing teardown,
health-session resets, sensor initialization writes and uint32/saturating time.
These differences must stay visible in the implementation tests and guides.

This review does not establish silicon facts or SDK blocking bounds. Prompts
2-4 verify device facts; later owner/adapter stages implement and test these
selected contracts. No driver, compatibility facade, production fake, bus manager
or FieldCore integration code was introduced by prompt 1.
