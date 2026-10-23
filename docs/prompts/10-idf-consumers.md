# P10 — Add native ESP-IDF and generic owner consumers

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 native-IDF
CLI build, LDC1614 and MB85RC component/transport layout, SHT3x shared processor,
and FieldCore I2cTask/binding/time contracts. Check official selected-IDF docs.

Use [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md) for exact core types and
[CLI_CONTRACT.md](CLI_CONTRACT.md) for frontend parity. Keep one straightforward
adapter per platform; do not add a cross-framework service container.

## Work

1. Add `examples/esp_idf/basic/` with real CMake project/component wiring,
   `main/app_main`, sdkconfig defaults, board configuration and the same shared
   CLI as Arduino. No Arduino-as-component, Wire, Serial, Arduino clock or facade.
2. Use the chosen SDK's native master-I2C API with application-owned bus/device
   handles, finite timeout and explicit cleanup. Map native statuses conservatively,
   preserve counts/effects and ensure the chosen STOP/repeated-start behavior.
   A synchronous adapter is terminal; asynchronous/DMA work must not outlive buffers.
3. The native app owns tasks/locking, monotonic clock, GPIO ISR flag/queue and
   console backend. Keep callback/loop work bounded. Set the actual input and output
   console routes for S2/S3 USB or UART and test them independently from compilation.
4. Generate example-only firmware identity correctly on clean and incremental
   builds, without making core depend on Git/Python. Native IDF and Arduino output
   equivalent provenance, feature, job and diagnostic records.
5. Add a small plain-C++ consumer/example and a tested generic owner integration
   fixture. Bind transport, stage one operation, poll at budget one, copy facts,
   consume a tokened result, cancel locally, and let the application decide recovery.
   Show second-device fairness and externally signaled reset/invalidation.
6. Write `docs/integration.md` with a FieldCore-style mapping table: library
   callbacks -> owner transport; tokens/results/effects -> owner request slots;
   time model -> owner clock; diagnostics -> copied facts/health policy. No
   FieldCore include, inheritance, queue types, copied product status enum or
   required sibling dependency. No modifications to FieldCore in this task.
7. Prove component reuse by a small independent consumer that does not compile
   the repository example as library source. Keep dependency/path overrides
   local to examples/test fixtures; do not ship absolute workstation paths.

## Concrete native and generic structure

Native IDF project files are `examples/esp_idf/basic/CMakeLists.txt`,
`sdkconfig.defaults`, target defaults where needed, and `main/CMakeLists.txt` /
`main/main.cpp` containing `extern "C" void app_main()`. Keep adapter source under
the platform-specific path selected in the baseline, shared CLI source under
examples/common, and core component registration at the library root.

The application creates the native bus/device handles. A borrowed context carries
those handles and counters; it does not create/delete them on behalf of the core.
Use the SDK's terminal synchronous APIs and map `esp_err_t` to Status without
exposing SDK types in public library headers. Keep the numerical backend detail.
Do not fabricate partial byte counts or phase-specific NACKs from a coarse error.

Native time is converted into the selected monotonic millisecond domain once at
the adapter/owner boundary. Record rounding and minimum nonzero backend timeout;
an expired operation must not become an infinite SDK wait. The callback budget
covers lock acquisition and every pointer/read phase. Tests must cover a nearly
expired deadline before entering a backend with coarser timeout resolution.

The generic consumer uses ordinary CMake and callback functions, with no SDK or
product headers. It demonstrates `bind`, one `start*`, `poll`, `takeResult`, a
cached snapshot and explicit invalidation. A fake transport makes it runnable on
the host; it must be labeled simulated, not a physical TUSB demo. The owner test
interleaves two devices and checks observable results instead of private fields.

| FieldCore-side concept | Library fact or operation | Ownership stays here |
| --- | --- | --- |
| I2cOwnerTransport | DriverConfig callbacks/context, terminal Status and effect limits | Physical bus and transaction serialization |
| Owner request slot | OperationToken, OperationTiming, start methods | Request scheduling and queue capacity |
| Worker service turn | poll with bounded callback budget | Scheduling fairness and current time |
| Completed request | OperationResult through takeResult | Routing result to requester |
| Product diagnostics/health | TransportStats, DriverDiagnostics, port/config snapshots | Required-device/absence/stale/retry policy |
| Power/reset notification | Explicit local invalidation followed by chosen reconcile/apply | Power sequencing and USB policy |

Document this mapping; do not copy FieldCore enums or implement a product wrapper
inside the chip library. Reference source paths are evidence, not lib_deps.

## Required build and failure cases

Establish and run real `idf.py -C examples/esp_idf/basic set-target esp32s2 build`
and the corresponding esp32s3 command in isolated build directories. Confirm the
linked image contains the shared CLI and core, and has no Arduino requirement.
Test context lifetime, cleanup ordering, bus creation failure, timeout/error
mapping, missing IRQ/console backend and source-package paths. Never make the
core call `i2c_del_master_bus` or change a consumer's bus speed during bind/end.

## Documentation in this stage

Write Doxygen for each adapter context/function and source file, including borrowed
handles, milliseconds/rounding, terminal callback behavior and concurrency. Comment
the IDF build branch and source lists where their purpose is not obvious. Update
native example README, examples index, generic consumer README, integration/owner
guides, CHANGELOG and validation with actual SDK targets and results. Host-only
consumer comments explicitly state that its fake transport is simulated.

## Tests and completion

- Real native IDF configure/compile/link for esp32s2 and esp32s3 across the
  selected supported matrix; record exact versions, failures and unavailable tools.
- Build/link/run the generic C++ and fake-owner consumers without framework or
  sibling include paths. Verify IDF component manifests do not pull Arduino.
- Shared CLI behavioral tests remain identical. An SDK-header shim compile is
  useful but never reported as a linked IDF firmware build.
- Document lifecycle and resource shutdown ordering, context lifetime, driver
  access serialization and transport-error mapping for every supplied adapter.
