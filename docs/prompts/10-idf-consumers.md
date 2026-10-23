# P10 — Add native ESP-IDF and generic owner consumers

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 native-IDF
CLI build, LDC1614 and MB85RC component/transport layout, SHT3x shared processor,
and FieldCore I2cTask/binding/time contracts. Check official selected-IDF docs.

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

## Tests and completion

- Real native IDF configure/compile/link for esp32s2 and esp32s3 across the
  selected supported matrix; record exact versions, failures and unavailable tools.
- Build/link/run the generic C++ and fake-owner consumers without framework or
  sibling include paths. Verify IDF component manifests do not pull Arduino.
- Shared CLI behavioral tests remain identical. An SDK-header shim compile is
  useful but never reported as a linked IDF firmware build.
- Document lifecycle and resource shutdown ordering, context lifetime, driver
  access serialization and transport-error mapping for every supplied adapter.
