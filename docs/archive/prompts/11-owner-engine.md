# 11 — Implement lifecycle, result ownership and read-only initialization

Complete [prompt 10](10-register-codecs.md) first. Read [COMMON](../../prompts/COMMON.md),
the lifecycle, tokens, effects and time sections of
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md), and the completed API contract.
Deliver one working cooperative engine with `startInitialize`; other operation
families are added in later prompts. Every public method introduced here must
compile, link, work and have tests. Do not declare unimplemented methods.

## Focused audit and delegation

Re-audit LDC1614 and LSM6DS3TR admission/poll/cancel/take code and fixed fake tests;
TMP1x2 operation output preservation; INA3221 effect/lifetime rules; FieldCore
owner-buffer contracts as read-only consumer evidence. Inspect current matching
examples, CLI/runner assumptions, registration, CI and AGENTS. Record revisions,
dirty state and selected differences, especially one retained result and no replay.

Keep the shared engine and lifecycle with the lead. A coding subagent may own
disjoint owner tests after signatures are fixed; use a strong reasoning reviewer
for token/effect retention and stale-time decisions. Follow
[AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), current models and lead-only Git.

## Implement

1. Implement validation and atomic `bind`: invalid replacement preserves prior
   binding/evidence/results, active work returns `BUSY`, retained terminal work
   returns `RESULT_PENDING`. All lifecycle/admission/cache/result paths are
   bus-silent. Copy accepted configuration and borrow only documented context.
2. Implement `startInitialize`, token-checked `poll`, `cancel`, `resultSnapshot`
   and `takeResult`, with baseline error precedence. Admission returns
   `IN_PROGRESS`; allocate a nonzero uint64 token only then. Failed admission
   preserves caller output. Tokens never reset through end/bind/forget and never
   wrap. One active slot becomes one retained terminal slot; no replacement loss.
3. Delivery success is `OK` even when the copied operation failed. Cancel returns
   `OK` while its result contains `CANCELLED`. Matching terminal poll returns the
   retained status without traffic; matching terminal cancel leaves it unchanged.
   Wrong/zero/stale tokens cannot advance, consume or change another operation.
4. `end` cancels into a retained result, unbinds and invalidates usability without
   I2C. `invalidateDeviceState` records external uncertainty and retains effects;
   normalize an OK reason as specified. `forgetDeviceEvidence` requires unbound,
   inactive, no-result state and does not reset tokens/counters. Retain the one
   variant/address/context/both-callbacks association across same-target rebind; changing
   an unresolved target requires explicit forgetting, not a registry or cleanup.
   New targets never inherit current old-target identity/configuration/port data.
5. `startInitialize` reads identity and required initial persistent configuration
   only. It never applies defaults, resets, ACKs or changes role. Publish complete
   identity/config evidence with explicit validity. Accepted reinitialization
   clears current identity/initialized usability; failure cannot retain old READY.
   Complete identity contradiction invalidates current target evidence. No desired
   profile is adopted or interrupted effect erased. Use the existing managed wrapper.
6. Use small named private phases and coarse public progress labels. `poll` alone
   performs managed I2C. Zero budget means zero callbacks; failures consume one;
   default one means at most one. Bound local transitions too. No spin, retry,
   sleeping, lock or borrowed transfer buffer across owner turns.
7. Use the baseline uint64 half-range time arithmetic, intervals in `(0,2^63)`,
   backward-time policy and immutable deadline. Clip each callback to
   `min(i2cTimeoutMs,remainingMs)` without zero/narrowing errors. Status-only
   callbacks require returning after the callback and a fresh owner timestamp
   before time-sensitive success. Larger budget is a ceiling, not permission to
   reuse stale time. Actual callback failure/detail stays primary even if the
   next timestamp is late; late pending success becomes owner `TIMEOUT`.
8. Introduce only the cached getters whose evidence now exists. Maintain separate
   operation, configuration, termination/reset, effect and freshness state. Set
   snapshot invalidation on end/external invalidation without discarding history.
   Prepare shared effect handling for later writes without fake successful jobs.

## Verify and finish this prompt

Register `tusb32x_operations`. Test atomic rebind, invalid/BUSY/result backpressure,
unchanged outputs, repeat snapshot, exact take once, every token precedence,
exhaustion through a bounded internal arithmetic fixture, end/invalidate at every
initialization phase and retained result after unbinding. Test failed reinit after
success and callback changes with null context. No initialization writes or public
test-only seed API; test boundary arithmetic directly and normal public paths.

Cover budgets 0/1/larger, failed-attempt counting, deadline before/at/after a
callback, late success versus original backend failure, absent fresh time,
rollover/half-range limits, stalled/backward time and bounded local work. Run two
instances on one serialized fake bus and prove independent state. Public
compile/link tests call every newly available method; no framework stubs.

Add Doxygen for all implemented methods/types/fields, including lifetimes,
delivery versus outcome, error precedence, work bounds and time validity. Update
ownership/API/architecture guides, feature matrix, README/CHANGELOG, progress and
validation with exact commands and counts. Run focused tests, previous protocol
regressions and applicable documentation checks. Review, commit and sync the
same execution branch; verify upstream equality before completion.
