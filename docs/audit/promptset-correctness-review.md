# Promptset correctness and simplicity review

This dated review uses the original zero-based stage labels. The current
[scope review](promptset-scope-review.md) maps them to prompts 1–38.

Date: 2026-10-23. Reviewed the prepared prompts after commit `ca1e687` against
the archived TI sources, current sibling patterns and their own cross-stage
contracts. This review corrects implementation instructions; it does not claim
that a driver, firmware build or hardware test exists.

## Confirmed errors and conflicting requirements corrected

| Finding | Failure the old instruction could cause | Correction and required regression |
| --- | --- | --- |
| Missing original TUSB320/I dead-battery exit sequence | Source-capable startup could claim readiness without the required Rp recovery after VDD returns | Added explicit `startRecoverDeadBattery` in the existing engine: identity/internal-bit checks, literal 0x45 writes 0x04 then 0x00 separated by at least 30 ms, and readback. Test every failure/cancel boundary and exclude later variants. |
| Detached-field validity contradicted the baseline | Tests could reject correctly decoded NOT_APPLICABLE values or expose a usable default orientation/current | Field validity can certify a NOT_APPLICABLE interpretation; it does not make detached orientation/current usable. Test both meaning and masks. |
| Audit required entire-operation success before observation publication | A useful complete port observation would be lost when a later IRQ ACK or recheck failed | Publish after the required observation reads complete. Preserve that observation and independently report later service failure/ACK uncertainty. |
| Firmware bulk jobs conflicted with the sole retained result slot | Watch/stress/dump/fixture operations would need hidden child-result consumption, a second scheduler or a second result store | Host runners compose ordinary commands and explicitly take each result. Firmware remains one-operation commands, cached queries and bounded raw diagnostics. |
| Discovery had no matching core API | Scanning via callbacks or temporary rebinding would bypass tracking or disturb target/effect state | Removed discovery from the CLI. Probe is an alias of selected-target initialization and uses the same managed accounting. |
| Request monotonicity was applied to asynchronous completions | Apply request 24 finishing after status request 25 could be mistaken for a reboot | Check increasing IDs on new-input ACK/MARK; correlate delayed JOB/RESULT/END with original request/token. Test that exact interleaving. |
| Result take could consume evidence before output was secured | A full output queue could drop the only terminal result after consumption | Reserve the complete bounded reply before take. Insufficient capacity leaves the core result intact; test queue pressure without adding another result cache. |
| Manual job step and automatic stepping could both poll | One loop turn could spend twice the intended callback budget | One poll call site per turn; manual budget replaces the automatic budget, including explicit zero. |
| Cached/rejected command bounds included unrelated automatic work | A correct handler could fail a zero-callback test when an older job advanced afterward | Bound handlers separately from owner turns; test exact deltas while idle/auto-off and continued progress with auto-on. |
| Maximum record size was not checked at maximum field widths | Full-width IDs/times and the complete port record could overflow the original 384-byte storage | Use a 512-byte bound and test maximum nonce/integer/enum widths including newline/NUL. Split schemas if future additions exceed the bound. |
| Profile reset wording suggested silicon reset defaults | The selected DRP/50% candidate could be presented as the chip's reset image | Profile reset changes local candidate settings only; default candidate values and TI reset values remain distinct. |
| One programming-section citation was used for all variants | An implementer could read the wrong generation's programming contract | Cite base §7.5, LI/HI and LAI/HAI §7.4, and TUSB322I §6.5 separately. |

The missing sequence is explicit in [TI SLLSEN9F §7.4.3, printed page 14](https://www.ti.com/lit/ds/symlink/tusb320.pdf).
Its register table on page 19 also reserves the internal bits for TI. Requiring
those bits to read as their documented zero default before the literal writes
avoids inventing an internal-bit repair. The library uses its existing fresh-time
anchor and 1 ms software margin, so this 30 ms minimum requires 31 elapsed integer
ticks. A final zero read cannot prove that an interrupted sequence occurred.

The owner determines when VDD has returned and source-capable startup needs this
operation. It is neither an implicit initialize write nor a generic soft-reset
step. The corresponding [LI/HI](https://www.ti.com/lit/ds/symlink/tusb320li.pdf),
[LAI/HAI](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) and
[TUSB322I](https://www.ti.com/lit/ds/symlink/tusb322i.pdf) sections do not prescribe
this base-device procedure.

## Underspecified contracts made explicit

- `PortSnapshot.invalidated` distinguishes historical decoded fields from usable
  current evidence. It is set after teardown, external invalidation and possible
  port-affecting writes; only a complete fresh observation clears it. The owner
  still considers age and each field's applicability.
- Observation time is the owner timestamp before the first required read, a
  conservative acquisition boundary. Delayed callback completion/publication
  must not make old observations appear newer.
- KNOWN means proved actual configuration, not equality with desired settings.
  Complete verify/reconcile proof can retain KNOWN actual settings while returning
  CONFIGURATION_MISMATCH. This permits a subsequent explicit apply without a
  circular requirement to first match the interrupted desired profile.
- Invalid command IDs, pre-marker nonce, request exhaustion and maximum formatted
  field widths are specified. No arbitrary input enters machine-record keys.
- `constexpr errorName` needs its complete definition in the public header;
  declaring it there and hiding its definition in a source file is insufficient.

These are small additions to existing values and helpers. They do not introduce
a clock service, event system, power manager or alternative operation engine.

## Simplifications retained in the implementation plan

- One core operation engine, one terminal result slot and one frontend poll call
  site. Bulk sampling, dumps and fixture sequencing belong in the Python runner.
- Removed redundant firmware job-start grammar, inverse quiet switch and bulk
  diagnostic state. Selfcheck/selftest are clearly named aliases of verify with
  the same desired-profile precondition, not general hardware certification.
- Source files express responsibilities; small private responsibilities may share
  a source unit. Coarse progress labels do not freeze private switch phases.
- Per-stage peer audits remain mandatory but focus on current relevant changes;
  only P00/final review repeat the broad inventory. Keep short progress evidence
  and amend decisions when conclusions change.
- Related guides may share a document with stable links and one authoritative
  contract. Source Doxygen and the standard repository documents remain required.
- Validation reuses registered tests. A properly isolated extracted-package
  firmware matrix may prove both firmware linkage and package completeness;
  running an identical source matrix again is not mandatory without a reason.

## Verified facts deliberately left unchanged

The LAI register table maps attachment code 01 to the local source and 10 to the
local sink. The existing duty encodings, base/extended nominal debounce tables,
and documented 95 ms maximum reset duration were correct. The one-tick wait margin
is conservative software scheduling, not an extra TI requirement or proof of
attachment. These were checked against the exact variant PDFs, not renamed or
reversed to make the review appear more extensive.

Retained results, possible write effects, explicit W1C handling, read-only
initialization/reconciliation and external ownership remain necessary. The
TUSB322I advertisement-access and fault-clear wording remain genuine source
ambiguities; affected operations stay gated rather than inventing behavior.

## Workflow clarification and review limits

Each new implementation promptset execution creates one new branch, or uses the
branch the user created for that run. All prompts and resumed sessions stay on
it. Every prompt and coherent completed unit is committed and synced. This
maintenance audit does not start the implementation execution; the branch-policy
correction was committed separately as `4a1e240` and synced on the existing branch.

Three agents independently reviewed silicon, API/evidence, and CLI/tooling. The
lead selected the corrections, integrated their disjoint edits and cross-checked
shared names, state semantics and commands. Source checks and focused mechanical
validation are recorded in [the validation record](promptset-validation.md).
New regression cases here are instructions for future implementation, not
claims that C++ behavior or physical hardware has been tested already.
