# P14 — Independently audit the complete library and close gaps

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** re-enumerate
Projects, record changed reference revisions, and inspect the strongest current
peer for each concern. Do not assume P00's inventory or this promptset is timeless.
Use independent review agents for protocol, owner/portability and CLI/tooling where
available; the lead integrates fixes and owns shared Git actions.

Use [AGENT_WORKFLOW.md](AGENT_WORKFLOW.md) to select available models by review
complexity. Prefer strong independent review for TI/owner state and a coding model
for executable CLI/tooling review. A fast agent can check links/inventory, but is
not the sole reviewer of W1C/reset or lifetime behavior. Give reviewers explicit
read-only scopes; the lead assigns and integrates fixes.

## Review and fix

1. Compare every requirement in P00-P13 and COMMON with actual code, tests and
   documents. Check family support breadth, maintenance ownership, naming/layout,
   status/diagnostics, examples, CLI, Python runners, AGENTS and package contents.
   Missing implementation is work to finish, not a passing checklist item.
2. Re-read TI register/timing evidence independently. Challenge variant defaults,
   addresses/identity, `0x45`/`0xA0` availability, mixed W1C and reset writes, role
   transition order/delays, current/accessory meaning and TUSB322 exception handling.
3. Challenge owner boundaries: callback terminality/timeouts/lifetimes, no framework
   leakage, passive bind and teardown, no retries or OFFLINE admission, zero/one
   transfer budgets, clock domain and post-callback timing, uncertain writes,
   cancellation, effect retention, stale tokens and result backpressure.
4. Challenge event integrity: read failure must not publish detach; pending INT
   survives unrelated setters; ACK and VCONN fault use correct masks; reassertion
   cannot cause unbounded service; no promise of atomic capture or lossless events.
5. Execute hostile CLI input and fake serial transcripts. Check real framework
   parity, firmware provenance, stale/duplicate/late/rebooted replies, incomplete
   suites, bounded output/soak memory, cleanup uncertainty and actual exit codes.
6. Review all clean consumers, generated metadata and package/docs links. Confirm
   all tests are registered and host-only success is not presented as native IDF
   linking, physical hardware, electrical compliance or new-variant qualification.
7. Fix findings in scope, adding focused regression tests. Re-run affected checks;
   once stable, run the required final matrix. Follow exact-final-commit CI policy
   where applicable. Commit and sync final fixes on the existing promptset branch
   under AGENTS.md; verify upstream equality. Do not invent a remote or turn
   routine Git synchronization into release publication.

## Review tracks and concrete questions

| Track | Inspect | Questions to answer with evidence |
| --- | --- | --- |
| Device/protocol | Capability table, byte codecs, write composer, timing and fake-device tests | Can any setter ACK IRQ, replay reset, touch an unsupported register or apply staged values unintentionally? What does a failed phase leave in hardware? |
| Owner/public API | Baseline declarations, operation engine, time/effect tests and generic owner consumer | Do names/Status codes match docs? Can stale tokens affect newer work? Can end lose a result/effect? Does callback time shorten a wait? |
| Diagnostics | TransportStats, managed evidence, projectHealth, port/config snapshots | Are callback and physical transaction counts separated? Does OFFLINE influence I2C? Can resetting statistics erase trust/effects? |
| CLI/adapters/runners | Shared processor, thin platform frontends, Python parser and real CLI fixture | Are setters staged, mutations explicit, invalid inputs silent, records correlated and terminal results consumed only once? |
| Distribution | CMake/PIO/IDF modes, packaged files, extracted consumers, CI registration | Does a clean consumer require a sibling, Git, Python or Arduino? Are all claimed tests/builds actually executed? |
| Maintainability/docs | Public/source Doxygen, helpers/files, READMEs/changelog/guides and generated docs | Is there duplicate state/protocol logic, unnecessary abstraction, stale comment or a hidden public symbol? |

Review the implementation against these specific misuse cases:

- Caller treats successful result delivery as successful hardware operation.
- Invalid input or an old token changes a retained result or increments I2C failures.
- A diagnostic read makes the application think configuration or port freshness
  is verified when only the transport succeeded.
- A timeout ends a job after a write but drops its possible hardware effect.
- A profile is staged while IRQ is pending; ACK accidentally writes the new profile.
- Base source-capable startup claims dead-battery recovery from a clear 0x45 bit,
  without evidence of the required 30 ms sequence after VDD return.
- IRQ observation succeeds but ACK fails; the implementation discards the useful
  observation or falsely reports the whole service succeeded.
- A cached snapshot keeps historical validity after external invalidation and is
  mistaken for currently usable port information.
- A newer status command finishes before an older operation; the runner mistakes
  the older operation's valid delayed result for a board reboot.
- A full output queue consumes result take before its complete reply can be queued.
- A STOP-separated access silently gets two full timeout budgets or is reported
  as one physical transaction.
- USB stalls, reconnects or DTR resets the board; runner accepts stale success or
  restores a profile against the wrong session.
- A compiler build has NDEBUG, so assertion-only tests vanish and still report pass.
- Doxygen hides an undocumented public method, and the checker mistakes silence
  for coverage. A source comment promises behavior that the implementation violates.

Add tests for real findings at the boundary that failed. Do not build a large new
test framework solely to prove one function. Remove unused compatibility aliases,
parallel platform protocol copies and speculative extensibility discovered by
the review. Keep private implementation private; improve observable contracts
rather than expose fields to tests.

For each finding record `severity | file/symbol | reproducible failure or evidence |
fix | regression check | remaining limit`. A review opinion without evidence is
not a passing or failing hardware result. All findings that can be fixed locally
are fixed before the final validation run. Missing physical fixtures remain an
explicit qualification gap, not an invented implementation defect.

## Documentation and simplicity gate

Every fix updates its Doxygen/source explanation and affected guide/CLI/changelog
in the same change. Re-run focused checks, then only the broader checks justified
by the final changes. Confirm the library remains a small direct device driver:
one protocol path, one operation engine, fixed storage, plain typed facts, no
owned scheduler/bus/power policy and no generic framework introduced for convenience.
Record intentional departures from ENGINEERING_BASELINE and CLI_CONTRACT with
their reasons and propagated changes. Hand off in simple engineering English.

## Final deliverables

- `docs/final-audit.md` with reviewed revision/dirty state, reviewers, evidence,
  findings/severity, fixes, remaining limits and final decisions.
- Complete progress ledger and feature coverage matrix; updated validation results
  and reproducible commands. Required unrun checks remain visible as NOT RUN/BLOCKED.
- Final response: implemented family/API, portable ownership, examples/CLI/runners,
  actual tests/builds/CI/hardware results and exact remaining blockers. Link docs.
- Do not call the whole library hardware-qualified when only LAI or only one board
  was exercised. Do not call work complete with known implementation defects,
  omitted required components or failing mandatory checks still fixable locally.
