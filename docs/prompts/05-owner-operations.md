# P05 — Implement lifecycle, configuration and cooperative operations

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** LDC1614 and
LSM6DS3TR time/result/effect contracts, INA3221/ADS1115 jobs, TMP1x2 staged state,
and FieldCore's bounded owner transport/lifetime assumptions.

## Work

1. Implement bus-silent bind/validation/admission/cancel/result access/end and
   explicit invalidation. Invalid replacement preserves prior valid state. Reject
   interleaved mutations while a job owns the instance. No unconsumed-result loss.
2. Implement initialization/identification and read-only observe/verify paths
   separately from explicit profile application and reset. Startup must not force
   source mode, advertise new current or clear a pending interrupt as a convenience.
3. Decompose multi-transfer work into finite states. Owner poll budget 0 means no
   callback; default 1 means at most one logical transfer callback with its physical
   work documented. Wait states do no I2C. No hidden busy loops or retries.
4. Implement the selected external time model, immutable operation deadline,
   remaining-time callback clipping, post-return deadline checking and bounded
   progress. Test stalled time, backward/invalid time if supported, wrap/overflow,
   extremely short deadlines, and delayed callback return. Timing starts from
   completion evidence, not an old timestamp sampled before a blocking transfer.
5. Implement variant-specific role/profile sequences. Base 320 mode changes are
   restricted to unattached state; treat an attach racing the check as a documented
   hardware/application limitation, not a guarantee that software eliminates races.
   Extended LI/HI/LAI/HAI use disable termination, set mode, externally timed 5 ms
   wait, then enable termination per P01. Do not substitute `0x45`. TUSB322I uses
   its separately specified sequence; never invent the 320 delay as its datasheet fact.
6. Implement explicit soft reset with conservative effect evidence, documented
   maximum reset time and readiness/verification. Do not assume complete power-on
   defaults or erase desired profile. PORT re-sampling and actual role are observable
   separately from requested configuration. EN/power timing remains application-owned.
7. Track desired/observed/trusted generations. Compare persistent config fields
   only; ignore live RO/W1C/command values when verifying a profile. Preserve
   uncertainty after any partially applied or ambiguously completed write.
8. Cancellation/timeout stops further automatic I2C and retains effects. If a
   sequence may leave termination disabled, expose that fact and an explicit
   reconcile/cleanup operation. Never perform hidden rollback from cancel/end.
   Rebind must not erase unresolved same-device effects merely to appear healthy.
9. Implement any synchronous conveniences only through this state machine with
   documented callback/time bounds and caller-owned timing/yield; omit unnecessary
   wrappers rather than create two divergent protocol implementations.

## Tests and completion

- Fault and cancellation at every state, especially after disable-term, mode write,
  reset acceptance and before verification. Assert exact traffic and retained effect.
- Busy/stale-token/result-backpressure/failed-bind tests; multiple independent
  instances; repeated result reads/take behavior; end/rebind during incomplete work.
- Shared-owner fixture interleaves a second fake device through TUSB timed waits;
  demonstrates fairness and no bus lock held between calls.
- No automatic bus reset, power cycle, error retry or recovery policy. No I2C in
  cached getters, admission, cancellation or ordinary teardown. Update contracts
  and feature matrix with actual symbols and maximum work.
