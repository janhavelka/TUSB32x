# Owner timing and portable consumption

Prompt 19 closes the core host checks before CLI and framework integration. The
[peer audit](audit/execution-19-peers.md) records current sources and rejected
assumptions. No new scheduler or clock belongs to the production library.
Prompt 27's [generic integration guide](integration.md) and runnable
[simulated example](../examples/host/basic/README.md) now demonstrate that boundary
directly. The owner fixture also executes stale-token rejection, cancellation
before/after a possible write, external power-event invalidation, separate
reconciliation/fresh publication and explicit teardown with live borrowed context.

The [native IDF application](../examples/esp_idf/basic/README.md) now demonstrates
this owner split with real S2/S3 links. A pinned main task takes every read-only
startup result, then hands command/result ownership to the shared CLI. Exclusive
UART FIFO output and retained input suffixes keep console work bounded. Shutdown
ends borrowing before deleting only the application's owned native handles;
failed deletion preserves storage/evidence without retry. Physical adapter and
serial timing remain separate qualification.

## Executable coverage

| Contract | Named runtime evidence | Registered suite |
| --- | --- | --- |
| Two physical targets, alternating owner turns, finite queue/result capacity | `waitingOwnerDoesNotHoldBus`, `queueAndResultBackpressure`, `failedOrRepeatedIrqOwnerDoesNotStarvePeer` in `test/test_owner_scheduling.cpp` | `tusb32x_operations` |
| Stale-token/retained-result isolation, cancellation and explicit reconciliation | `staleRequestsPreserveBothOwners`, `cancelledApplyNeedsResultTakeAndSeparateReconcile` | `tusb32x_operations` |
| External reset/power event does not become fresh proof through unrelated raw success | `externalPowerEventNeedsFreshProofAndPortPublication` | `tusb32x_operations` |
| Both owners end/take cancellations while the shared context remains live | `shutdownEndsBothOwnersBeforeBorrowedBusDies`; bus declared before driver storage | `tusb32x_operations` |
| Runnable ordinary two-TU owner with simulated transport/time | Same shipped main/profile sources, explicit apply/invalidate/reconcile and always-active checks | `tusb32x_host_basic` |
| Deadline110, callback starts108 with timeout2, physically returns111 | `literalLateWriteKeepsOutcomeAndEffect`: pending success becomes owner TIMEOUT; actual I2C_TIMEOUT/BUS/ERROR and full detail remain primary | `tusb32x_operations` |
| Mode completion108.1/108.9, next fresh sample108/110, minimum six ticks | `physicalModeWait`, `exactWaitGatesAndLateFinalTime` in `test/test_profile_application.cpp` | `tusb32x_operations` |
| Recovery31/reset96 ticks, independent elapsed microseconds, stalled/rollover time | `independentPhysicalWaits`, `wrappedWaitAndDisabledControls`, `clippedProcedureTimeouts` in `test/test_reset_recovery.cpp` | `tusb32x_operations` |
| Zero/one/larger budgets, bus-silent local work, immutable deadlines, invalid/backward/half-range time | `initializationAndBudgets`, `timeBoundaries`, `interruptionAtEveryPhase`, `invalidationAndFrozenResults` in `test/test_operations.cpp`; existing IRQ and apply fault matrices | `tusb32x_operations`, `tusb32x_port` |
| Callback changes with null context and target-evidence separation | `callbackAssociationAndNullContext`, `reinitializationAndAtomicBinding` | `tusb32x_operations` |
| Generation/token arithmetic at maximum without public seeds | `tokenBoundaryArithmetic` exercises the private helpers used by production, including desired/verified `nextGeneration` | `tusb32x_operations` |
| Counter/sequence saturation and independent health/trust/freshness | `saturationWithoutPublicSeeds`, `managedHealthAndClear`, `saturatingSequenceBoundary`; existing protocol accounting cases | `tusb32x_diagnostics`, `tusb32x_port`, `tusb32x_protocol` |
| Every current public method links and runs in an isolated two-TU consumer | `exercise`, `inspectFromOtherTranslationUnit` in `test/consumer/portable/` | `tusb32x_portable_consumer` |
| No allocations during exercised operations; active failure checks in Release | Armed C++ allocation/deallocation replacements and supported C allocator wrappers; exact-exit failure/allocation sentinels | `tusb32x_portable_consumer` |

Each public header also compiles alone in `tusb32x_header_contracts`. That object
target is compile evidence, not a behavioral CTest. Existing tests are retained
instead of repeating their fault matrices in the scheduler fixture.

## External ownership and bounds

The fixed test scheduler owns two pending request positions and one delivered
result per device. If the delivery position is full, the next terminal result
stays in its driver slot. Request IDs correlate external work; tokens identify
work within one instance. Queue rejection preserves the caller's ID and retained
work. Base320 at address60 and LAI at47 are distinct physical target models.

The owner alternates devices. Recovery waiting, a failed callback and repeated
bounded service with a pending recheck cannot monopolize the bus. The fixture
checks callback budgets0/1/255 and terminal release of its borrowed buffers and
bus lock. IRQ reassertion is an explicit script assumption. It is not a model of
electrical event timing, lossless capture or a production scheduler.

Each callback consumes an attempt, not necessarily one physical transaction. The
core returns after a Status-only callback. An equal integer tick can be a fresh
sample; the caller must actually resample after return. The core cannot detect a
caller replaying a pre-callback timestamp. TI intervals5/30/95ms and the separate
software1ms margin remain distinct; the immutable owner deadline never grows.

## Isolated consumption

`test/consumer/run_portable.py` is a maintainer CTest coordinator. It copies only
the standalone fixture, public headers, built static library and required parent
compile/link flags into a fresh retained build directory. The child CMake project
imports that library without finding Python, Git, Doxygen, framework headers or
sibling sources. Parent flags preserve ABI and sanitizer runtime requirements.

On Windows the child PATH contains system directories and copied MinGW runtime
DLLs only; tool executables are passed by absolute path. The harness verifies
Git/Python/Doxygen/PlatformIO are absent from that PATH. Other hosts retain their
ordinary PATH and do not claim executable absence. Standalone commands and trap
limits are in [the consumer instructions](../test/consumer/portable/README.md).

The allocation guard is armed across public operations and object lifetimes. Its
negative sentinels prove the failure path and allocation traps execute even in
Release. C wrapping covers references from linked objects, not arbitrary runtime
DLL internals. Unsupported C wrapping is reported explicitly; C++ traps still
run. This is evidence for the exercised core paths, not an exhaustive proof of
every future application or allocator.

See [validation](validation.md) for exact commands, counts and unavailable tools.
Host checks do not qualify physical timing, IRQ wiring, current resampling or
linked Arduino/native-IDF firmware. Existing TUSB322I source conflicts stay gated.
