# 18 — Close variant, write-effect and protocol test gaps

Complete [prompt 17](17-diagnostics-health.md) first. Read [COMMON](COMMON.md),
the implemented API and source/test matrix, and the silicon/effect sections of
[ENGINEERING_BASELINE](ENGINEERING_BASELINE.md). Earlier prompts already tested
their features. Audit coverage, add missing adversarial cases, fix defects and
leave the complete core protocol suite passing. This is not a deferred first-test
stage or a request to duplicate every existing test.

## Focused audit and delegation

Re-audit LDC1614 explicit callback-count tests and fixed fake, LSM6DS3TR
`test_ambiguous_write_effect_is_observable_and_never_retried`, PCA9555 public/
mixed-register tests and ADS1115 applied-write failures. Inspect current relevant
headers, examples, CLI/runner fixtures, test registration, CI and AGENTS. Record
revisions/dirty state and adopted/rejected patterns; import no sensor behavior.

A strong reasoning subagent may independently review protocol/variant coverage
read-only. A coding subagent may own a disjoint regression file after the lead
assigns concrete gaps. The lead owns production fixes and integration. Follow
[AGENT_WORKFLOW](AGENT_WORKFLOW.md); no agent switches branches or changes Git.

## Audit and close the matrix

Maintain `TI/software requirement -> variant/capability -> public API -> named
test -> command/result`. Use real test names and cited TI tables. Expected bytes
and masks must not come from production helpers. The scripted transport rejects
unexpected address, kind, bytes, length, timeout and extra calls; its small device
model represents only necessary RO/live fields, W1C/reset and ambiguous writes.
Avoid a simulation framework, test DSL or copy of the implementation engine.

Check existing coverage and add the missing cases:

- All six TUSB320 variants and both legal addresses, unavailable controls/base
  revision read, wrong variant/address, exact eight-byte ID order, reversed/
  partial identity, unknown revision policy and polarity not inferred from ID.
- Explicit pointer every read, documented ranges only, holes/wrap, exact receive
  success, no publication on incomplete receive, full native error detail and
  unchanged outputs. Invalid enums/parameters/timeouts and static Status messages.
- All role/current/accessory/duty/preference/debounce encodings, reserved raw
  evidence, base/extended differences, local/peer meaning and detached valid
  NOT_APPLICABLE interpretation without usable current/orientation claims.
- Staged versus applied ACK (`0xB5 -> 0x15`, base `0xB4 -> 0x14`), settings
  preserving pending IRQ, reset isolation (`0xDE -> 0x56`) and internal `0x45`
  preservation. No raw bypass of typed sequences or unknown safe-image guess.
- Base attached-precheck rejection and documented race; extended disable/mode/
  wait/enable sequence, requested disabled final states, both termination
  controls independently effective and bounded explicit reset/recovery.
  Restrict mode sequencing/prechecks to actual mode changes; cover attached base
  current-only updates, verified no-op apply and retained pre-write actual evidence.
- TUSB322I separate signature, DRP00 semantics, no `0x45`, independent IRQ/fault
  evidence, documented DFP/UFP sequence without an invented wait, and unresolved
  advertisement-change/fault-clear/direct-DRP gates. No inferred family support.

## Required cross-operation regressions

Use explicit readable tests for these observable scenarios; retain an existing
equivalent instead of adding a duplicate solely for the suggested name:

1. `failed_write_may_have_applied`: each mutating phase with no applied data and
   applied-then-error. Preserve original failure/detail and earlier confirmed
   effects; no blind role/reset/recovery replay or unrelated-read trust repair.
2. `cancel_after_each_phase_is_bus_silent`: before first transfer, after reads/
   writes, during waits and after completion; repeat end/rebind and result take.
   Retain token/result, configuration generation and effect/termination evidence.
3. `failed_status_read_preserves_observation`: old fields/time survive failed or
   partial refresh; invalidation preserves history. Complete pre-ACK publication
   survives later ACK/recheck error with its original acquisition boundary.
   Fixed 09/08/09 changed bookends retain raw bytes with cross-register validity
   cleared, never restart until stable or invent usable current/accessory fields.
4. `irq_reassertion_remains_visible`: each read/ACK/recheck race and stuck latch,
   bounded service, no false lossless count and no implicit ACK from status read.
5. `reconcile_partial_profile_allows_explicit_apply`: establish KNOWN actual
   controls despite desired mismatch, retain desired and permit later explicit
   apply. Incomplete transition proof stays UNKNOWN; old operation stays failed.
6. `dead_battery_recovery_retains_interrupted_sequence`: exact base read/04/
   31-tick wait/00/read, internal-bit rejection, four-callback maximum, no
   transient snapshot and error/cancel/deadline at every boundary. Clear final
   controls alone cannot prove interrupted history; diagnostics retain it after
   result take, without a hidden admission restriction.

## Verify and finish this prompt

Include [CHIP_EVIDENCE](CHIP_EVIDENCE.md)'s public-code regression cases in the
existing suites: reset mask/value error, reserved Try.SRC, reversed/partial IDs,
base-versus-extended debug, current-field separation, ACK control preservation,
lost primary errors and incomplete observations. Re-derive expected bytes from
TI and independent arithmetic, never copied encoder/fake output. Correct any
inherited error in contracts, implementation, vectors and documentation together.

Run changed focused tests while fixing gaps, then the registered full host suite
once after fixes. Do not weaken checks, expose private state unnecessarily or
claim text searches prove behavior. Classify evidence as host software only.

Update clarified public/source Doxygen, API/variant/interrupt/ownership guides,
feature/source/test matrix, README/CHANGELOG, progress and validation with exact
commands, counts and PASS/FAIL/NOT RUN/BLOCKED. Apply documentation checks to
changed contracts. Review the complete regression/fix unit, commit and sync on
the same execution branch, verify upstream equality and report unresolved facts.
