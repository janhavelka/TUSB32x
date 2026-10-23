# P07 — Close protocol, state and portability test gaps

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** LDC1614 owner
failure matrix, LSM6DS3TR soak/fairness, PCA9555 write effects, TMP1x2 native/CLI
fixtures and ADS1115/INA3221 no-framework contracts. Tests already exist from
earlier stages; this stage broadens meaningful coverage and closes gaps.

## Work

1. Build a traceable test matrix: TI requirement -> capability -> API -> fault/edge
   cases -> actual named tests. Include every implemented variant's distinct
   addresses, identity, revision, defaults, masks and role-change behavior.
2. Use scripted transfer expectations plus a small behavioral device model with
   W1C, live status changes, reset timing and uncertain writes. Expected values
   come independently from TI tables. Do not make the mock a copy of driver logic.
3. Test public input/output guarantees: invalid enum, null/zero/oversized buffer,
   timeout range, overflow, unsupported register, misbinding, partial read, output
   unchanged on failure, and static message lifetime. No hidden allocation.
4. Test every operation state under NACK/error/timeout/late completion/cancel,
   unresolved write effects, raw external interference, reset/power invalidation,
   repeated end/bind, stale tokens, result backpressure and recovery observability.
5. Assert exact callback counts for bind/start/cancel/cache getters/budget 0/1/N.
   Test time waits at boundaries, after slow transfers, rollover/overflow and
   beyond deadline. No shortened mode/reset delay caused by stale owner time.
6. Run two devices through a fake external scheduler with one owner and finite
   queue/result storage. Prove fairness and no app-wide bus recovery in the core.
   Use no FieldCore, Arduino, IDF or RTOS headers in this integration fixture.
7. Compile each public header independently; compile/link a normal plain C++17
   consumer and multiple translation units; test no-framework include boundary.
   Run strict warnings and supported ASan/UBSan host configurations. Avoid
   assert-only tests that vanish under NDEBUG.
8. Register every suite in CTest and/or the agreed native runner. Add focused
   source/metadata guards only where useful, with negative fixtures. Keep real
   behavior tests primary; do not overfit checks to exact source spelling.

## Deliver and check

- A clear native command that runs all applicable suites, with test counts and
  failures propagated. Python/tool prerequisites are declared; no skipped
  mandatory test silently results in overall success.
- Re-run focused tests after fixes, then the complete host matrix once. Record
  results in validation docs, including unavailable sanitizers/toolchains.
- No hardware compliance or electrical behavior claim from host success. Later
  CLI/adapter/runner tests complement this suite and do not replace it.
