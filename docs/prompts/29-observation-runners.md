# 29 — Deliver read-only runner plans and native integration

Turn the tested session layer into a usable command-line runner for readonly,
soak and bounded register observation. Finish these plans with live C++ integration
and evidence reporting before adding physical fixture mutation suites.

## Starting evidence and audit

Read [COMMON](COMMON.md), [AGENT_WORKFLOW](AGENT_WORKFLOW.md), and runner/record
requirements in [CLI_CONTRACT](CLI_CONTRACT.md). Read observation validity and
register-access sections of [ENGINEERING_BASELINE](ENGINEERING_BASELINE.md).
Prompt 28 supplies implemented session/identity/artifact functions; prompts 20–27
supply CLI routes, firmware identity and framework/consumer examples.

Re-audit TMP1x2 finite runner plans, 24Cxx persistent CLI transcripts, LDC1614
verdicts and INA228 callback-budget/owner-exclusion suites. Compare MB85RC/RV3032
strict parsing if relevant. Record current revisions/dirty state, exact sources
and choices. Inspect actual TUSB CLI behavior rather than assuming the docs match.

## Implement complete observation plans

1. Expose --port, --baud, --command-timeout, --write-timeout, --variant, --address,
   --expect-commit, --expect-framework, --expect-target, repeated --suite, --out,
   --dtr, --rts, --help and --dry-run. Validate everything before serial open;
   --port is required for real serial work, not for dry-run. Default suite is
   readonly. Dry-run saves the concrete plan and reports hardware NOT RUN.
2. Implement readonly: identity/provenance, cache-only routes, selected-target
   initialization and status, zero/one-callback budgets, malformed-input fences,
   staging/validation without apply, retained-result query/take and typed evidence.
   Verify/selfcheck aliases require existing retained desired state; either check
   that precondition or test its rejection. Never apply merely to make a readonly
   plan pass. No initialization reset, IRQ clear or profile replay. Before work,
   reject another owner's active/retained operation. Capture staging, job auto and
   output settings actually changed; restore these local values afterward without
   hardware apply. Lost sessions retain unrestored-state evidence, not replay.
3. Implement soak with documented duration/count/interval options and an explicit
   finite whole-run deadline; validate checked arithmetic. Counts are 1..100000 and intervals
   1..3600000 ms with documented finite overall limits. Repeated detached state is
   valid. Observation sequence/time and invalidated=0 plus relevant validity bits
   establish useful fresh evidence; a recent historical timestamp alone does not.
4. Provide bounded register observation within these plans using a finite
   variant-allowlisted list of ordinary reg commands. Document possible read
   effects; do not claim a read is harmless solely because it has no write API.
   Keep bulk counts/statistics/dumps on the host; add no firmware bulk scheduler,
   child-result scheme or second run token. Do not invent unsupported chip reads.
5. Each operation must reach its matching terminal evidence and have its result
   explicitly taken before the next begins. Cancel/take only this run's admitted
   tokens. Stop on unexplained failure. A result
   query is not another completion. Preserve unexpected writes/effects and output
   loss as failed/incomplete evidence, never scrub the transcript into success.
6. Exact cache/rejection callback-delta tests use an idle core or job auto 0.
   Separately test already-admitted work progressing once per turn under cached
   queries with auto enabled. Keep handler callbacks, core operation callbacks and
   backend physical transactions distinct. Do not infer transaction phases from
   terminal callback counts.
7. Connect the existing artifact/verdict layer to executable main: truthful exit
   codes, saved partial reports, bounded captures and separate host/firmware
   identities. Expected identity must match flashed-firmware records. Unknown
   values cannot satisfy --expect-*; labels supplied by the host are not proof.

## Execute meaningful tests

Extend `tools/test_tusb32x_hil_runner.py` for argument/pre-open rejection, plan
selection, finite arithmetic, dry-run, retained results, callback budgets and
observation invalidation, foreign-result denial and local-state restoration after
success/failure. Use fake serial/clock for failures, with no real sleeps.
Test every new guard's negative path and all exit classifications.

Extend `tools/test_cli_transcript.py` to run the full readonly plan against one
persistent tusb32x_cli_host using the actual shared CLI and scripted hardware.
Include malformed requests, failed callbacks, output-full result take and legal
older completion after newer query. Hand-written good transcripts alone do not
complete this stage. Reuse hil_runner and cli_transcript CTest registrations,
finite timeouts and the exact built fixture path. Run those tests, runner --help
and a readonly dry-run for tusb320lai/address 0x47 into a build artifact directory.

## Delegation, documentation and completion

Optional: a general coding agent can implement dry-run/planning tests in disjoint
files; another can review the native transcript path. Use stronger reasoning for
freshness or request-correlation conflicts. The lead owns final integration/Git.

Finish observation instructions in `docs/hil-runner.md`; begin
`docs/hardware-validation.md` with per-variant wiring, strap and power assumptions,
operator-edited port/expected-commit commands and NOT RUN criteria. Do not present
invented COM ports or SHAs as executed evidence. Update Python docstrings, README,
CHANGELOG, coverage, validation and progress. HIL is never invoked implicitly by
ordinary tests or validate.py. Commit and sync checked work on the same execution
branch, verify synchronization and report native/fake-serial evidence separately
from physical serial/I2C/CC evidence. Missing hardware does not justify stub plans.
