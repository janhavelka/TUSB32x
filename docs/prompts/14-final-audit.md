# P14 — Independently audit the complete library and close gaps

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** re-enumerate
Projects, record changed reference revisions, and inspect the strongest current
peer for each concern. Do not assume P00's inventory or this promptset is timeless.
Use independent review agents for protocol, owner/portability and CLI/tooling where
available; the lead integrates fixes and owns shared Git actions.

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
   where applicable, without manufacturing a remote or publishing without authority.

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
