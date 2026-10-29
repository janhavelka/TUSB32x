# 27 — Prove plain C++ and generic owner integration

Read [COMMON.md](../../prompts/COMMON.md). Prerequisites: prompts 1–26. Deliver a runnable plain
C++ example and an owner integration fixture with no framework or product
dependency. This prompt demonstrates the API already implemented; it does not
introduce a FieldCore wrapper.

## Audit and delegation

Re-audit FieldCore-node I2cTask/transport/binding/time contracts and two useful
generic consumers in LDC1614, MB85RC or TMP1x2. Inspect actual declarations,
tests and AGENTS; do not modify sibling projects. Read baseline operation/result/
time contracts and ENGINEERING_RULES ownership sections.

A coding agent may implement the host example while a separate reviewer checks
two-device fairness and lifetime cases. Give disjoint scopes; the lead integrates,
runs checks and owns Git under AGENT_WORKFLOW.

## Implement the consumers

1. Add `examples/host/basic/` using ordinary CMake and callback functions. Bind,
   admit an operation, poll with callback budget one, take the exact-token result,
   inspect its operation Status/effect and copy a cached snapshot. Treat
   IN_PROGRESS as accepted work, not failure; delivery OK is not operation success.
2. Use a small explicitly simulated transport so the example runs without a
   physical chip. Label it in source and README. Do not leak fake devices or
   simulation shortcuts into production core.
3. Add or extend the generic owner fixture to interleave two independent driver
   instances on one externally serialized bus, with one configuration owner per
   physical target. Bus serialization alone cannot protect caches from another
   writer to the same chip. Verify observable per-turn
   budgets, fairness, stale-token rejection, retained-result backpressure,
   cancellation, uncertain effects and caller-selected recovery.
4. Demonstrate an external reset/power-loss notification through explicit local
   invalidation, followed by a separately chosen reconcile/apply request. A
   successful unrelated read cannot repair configuration trust or cached freshness.
   No product queue types, inheritance, singleton or new scheduler belongs in core.
5. Show safe teardown and borrowed context lifetimes. Owner access is serialized;
   ISR signals only. No bus lock is held between polls. Base TUSB320/I source-capable
   startup after VDD return requires application readiness and explicit recovery
   after identity; do not copy its register sequence into consumer code.
6. Document this concrete mapping in `docs/integration.md`:

| Owner concept | Library boundary | Application responsibility |
| --- | --- | --- |
| I2cOwnerTransport | DriverConfig callbacks/context and terminal Status | Bus lifetime and transaction serialization |
| Request slot | OperationToken, OperationTiming, start methods | Queue capacity and scheduling |
| Service turn | poll with finite callback budget | Fairness and current owner time |
| Completion | takeResult and OperationResult | Routing outcome/effects to requester |
| Diagnostics | TransportStats, DriverDiagnostics and snapshots | Required-device, stale and retry policy |
| Power/reset event | Local invalidation and explicit later operation | Supply sequencing and USB policy |

Reference FieldCore paths as evidence, not lib_deps or include paths. Keep copied
facts and product health policy outside the chip driver.
FieldCore's transport can report completion time/effects richer than Status-only
callbacks. Do not imply those survive the bridge: the ordinary TUSB32x API still
needs a fresh next owner timestamp and conservative acceptance evidence. Validate
the backend's aggregate bound, including SDK internal waits/cleanup; disclose such
backend behavior instead of claiming the core can remove it.

## Verify and document

Build, link and run the example and owner fixture without Arduino/IDF, sibling
include paths or dependencies on Git/Python to consume core. Check all public
headers compile independently and use the actual public API rather than private
test hooks. Reuse existing registered owner tests where they already prove a
requirement; add only missing integration cases.

Document callback lifetime, aggregate timeouts, monotonic clock, zero-I2C methods,
results/effects and shutdown in source Doxygen and the host README. Update the
examples index, integration guide, coverage, CHANGELOG and validation. Keep the
simulated nature visible; host evidence cannot qualify physical Type-C behavior.

## Finish this prompt

Both consumers must actually link and run with meaningful assertions. The mapping
must be sufficient for another ordinary C++ owner without a FieldCore dependency.
Fix local defects, update progress, **commit and sync** this complete unit on the
same execution branch and verify upstream equality.
