# P06 — Implement port observations, IRQ service and diagnostics

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** INA228 interrupt
evidence, LDC1614/LSM6DS3TR diagnostics, BME280 configuration trust and TMP1x2
passive health projection. Compare actual counter semantics rather than names.

## Work

1. Publish typed local port attachment, requested role, orientation, detected and
   advertised current categories, active-cable and accessory observations with
   raw bytes, validity, timestamp/sequence and configuration provenance. Mark
   detached/not-applicable data explicitly; do not invent defaults from failed reads.
2. Publish a completed observation atomically to software consumers. Retain old
   successful data with its age on failure. A burst is not proof of simultaneous
   hardware capture; document possible mixed-time observations and any bounded
   verification/re-read strategy. Never spin until the world stops changing.
3. Keep current category semantics variant-accurate, including charge-through
   accessory encodings. Distinguish local sink detection from source advertisement.
   No unconditional conversion of default current to permitted milliamps. Expose
   the unresolved attached-current tracking limit from the TI audit.
4. Implement explicit bounded interrupt service: observe, retain/publish relevant
   evidence, deliberately acknowledge W1C, verify/recheck and leave pending work
   visible when budget/deadline is exhausted. Preserve control bits during ACK and
   preserve pending IRQ during other configuration writes. Failed ACK is ambiguous.
5. Test reassertion during every phase; coalesced hardware events cannot yield a
   lossless event log. Expose state changes/sequence/drop or overflow evidence if
   you add a fixed event buffer. No heap, callback reentry or application callback
   while holding a bus lock. Polling works without an IRQ pin.
6. Implement all-callback TransportStats and passive DriverDiagnostics, with
   underlying adapter transaction/byte counts separately identified. Optional
   HealthSnapshot/projection uses UNINIT/READY/DEGRADED/OFFLINE vocabulary but does
   not gate requested work or retry. Document counter units/lifetime/saturation,
   last terminal error and explicit clear behavior. Managed-only counts are named.
7. Keep communication success, configuration mismatch/trust, valid detached state,
   stale data, unsupported feature and operation cancellation separate. Every
   callback attempt is counted once, every purely local rejection zero times;
   backend transaction counts are a distinct unit, as defined in COMMON.
8. Implement optional framework-neutral helpers for status names, snapshot age,
   capability display and change detection. Keep USB policy and power decisions in
   applications. For supported 322I, separate IRQ ACK from VCONN fault evidence.

## Tests and completion

- Every attachment/current/accessory/orientation encoding with meaningful validity;
  unknown/raw values; failed second register read; no synthetic detach on I2C error.
- Pending IRQ preserved by each setter; failed/reasserted ACK; bounded service
  progress; simultaneous events; polling without pin; multi-instance independence.
- Full health/projection transition and saturation matrix, including raw/probe
  counters, no-traffic calls, protocol mismatch vs physical failure, and legal
  diagnostic operation while projected OFFLINE.
- Document intended/effective semantics and snapshot limits. IRQ line behavior,
  connector orientation and dynamic current tracking remain separate HIL evidence.
