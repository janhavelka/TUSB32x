# 19 — Verify owner timing, fairness and framework-free consumption

Complete [prompt 18](18-protocol-conformance.md) first. Read [COMMON](../../prompts/COMMON.md),
the time/lifetime/accounting sections of
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md), current owner code and registered
tests. Close scheduling and portability gaps with executable evidence and fix any
defects. This completes the core's host checks before CLI/framework integration.

## Focused audit and delegation

Re-audit ADS1115 callback-time/owner fixtures, LSM6DS3TR changing-budget/wait
tests, INA3221 owner tests and PCA9555 public consumers/CI. Inspect current
examples, runners, registration and AGENTS; record revisions/dirty state and
adopted/rejected choices. ADS1115 tests using `Wire.h` or private-member macros
do not prove portable public consumption. Do not copy LDC1614 build requirements
for Python/IDF into a downstream ordinary C++ build.

A coding subagent may own disjoint consumer/link fixtures; a strong reasoning
reviewer may audit timing/fairness cases read-only. Keep production fixes and test
registration with the lead. Follow [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), current
models, bounded scopes and lead-only Git.

## Prove bounded time and scheduling

Review [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md)'s timing provenance. Keep TI minima/maxima,
nominal debounce and the software clock margin distinct. Public 95 ms sleeps,
1 ms retry policies, 25 ms reset spacing and 700 ms board debounce establish no
new core timing rule. Reject unbounded retry loops even if each backend call
has a timeout. Register-read freshness cannot prove physical current resampling.

Use an external fake clock whose physical sub-ms time can advance inside a
callback independently of supplied integer timestamps. Extend existing cases
where needed rather than duplicate the whole suite.

1. Budget zero gives no callback; one gives at most one even on error or IRQ
   service. Larger budgets remain ceilings; Status-only callbacks stop the owner
   turn until fresh time. Distinguish a combined callback from its possible two
   STOP-separated physical transactions. Bound local phase transitions too.
2. Bind/start/cancel/end/results/cached getters/counter clear stay bus-silent.
   Invalid, stale, busy and unsupported requests preserve represented evidence.
   No lock or transfer buffer survives a terminal callback/owner interval.
3. Hardware waits use fresh completion evidence or next fresh owner time. TI
   intervals 5/30/95 ms require 6/31/96 elapsed integer ticks with the named
   software margin; it never extends the deadline. Mode completion at 108 gives
   earliest 114; first fresh timestamp 110 gives 116. Physical completion at
   108.1 or 108.9 cannot enable at reported 113. Check actual elapsed time
   independently, before/at/after boundaries, for mode/reset/recovery.
4. Deadline 110 with a callback starting at 108 permits at most 2 ms aggregate
   timeout. Success returned at 111 cannot become on-time success using stale
   evidence. Actual `I2C_TIMEOUT`, `I2C_BUS` and generic errors remain primary
   after a late fresh timestamp; preserve actual write effects in every case.
5. Exercise uint64 rollover/maximum, half-range interval limits, backward/stale
   inputs under the documented policy, stalled time and safe narrowing. Test
   saturating generations/counters without relying on arithmetic equalities once
   saturation occurs. Use bounded internal arithmetic fixtures, no public seed
   API or billions of calls. Equal fresh ticks are valid; reused pre-callback time
   is not fresh. Cancel/end/invalidate cannot invent a completion timestamp.
   No renewed deadline, hidden retry or busy wait.
6. A small fixed-capacity external scheduler alternates two instances on one
   serialized fake bus. One waits, fails or repeatedly observes IRQ while the
   other still progresses. Fill owner queue/result capacity, reject backpressure
   without loss and retain correlated results. No FieldCore/RTOS/framework/GPIO
   dependency or shared token/effect state; this fixture is not a new core scheduler.
   Use distinct physical targets with one configuration owner each. Test changed
   callbacks with null context and target-evidence invalidation across rebind.

## Prove public consumption and executable checks

Compile each public header alone and the facade from multiple translation units.
Compile and link a normal C++17 consumer using only public headers and the built
library, with no sibling paths, framework stubs, Git or Python/generator at
consumer build time. Use an isolated downstream build; packaged consumption will
be rechecked with the completed distribution in its packaging prompt. Exercise
all currently public methods so unimplemented declarations cannot hide.

Demonstrate no heap calls during exercised core operations with an allocation
trap or equivalent behavioral check, not a text search. Test full-width backend
detail, invalid argument output preservation and static message lifetimes.
Transport health, trust, freshness, attachment, IRQ and result remain independent;
OFFLINE cannot prevent legal work. Keep tests of public behavior, not private spelling.

Run CMake/CTest's registered core suites with strict warnings and supported
ASan/UBSan configurations. Keep assertions active in Release. A deliberately
failing fixture must exit nonzero; zero tests is not success. Run the current
native PlatformIO path through `scripts/pio.cmd` on Windows when available;
report unavailable tools honestly. Do not install another PlatformIO Core.

## Finish this prompt

Update test/consumer instructions, clarified source/public Doxygen, feature
matrix, README/CHANGELOG, progress and validation with exact commands, tool
versions, counts and limits. Run focused checks after fixes, then the affected
host matrix once. Host success is not linked framework firmware or electrical
qualification. Review, commit and sync on the same execution branch, verify
upstream equality and hand off a complete, tested core to the next prompt.
