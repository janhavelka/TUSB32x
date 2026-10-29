# 4 — Specify transport, silicon procedures and timing

Prerequisites: prompts 1–3 are complete. Read [COMMON](../../prompts/COMMON.md), the baseline's
transport/state/time contracts, and the completed variant/register guides. This
prompt finishes `docs/device-behavior.md`: each enabled procedure must be
implementable from cited steps, bounds and failure cases. It does not implement
the operation engine or qualify physical hardware.

## Focused re-audit

Recheck current PCA9555 mixed-register handling, LDC1614 fake transport and
ADS1115 owner examples/tests; compare LSM6DS3TR timing/effect contracts. Inspect
their relevant AGENTS, callbacks, tests and registration. Record current revisions,
dirty state and exact accepted/rejected rules. Peers supply engineering patterns;
only TI evidence establishes this chip's sequence, framing or wait.

## Framing and write semantics

1. Specify one-byte pointers, legal lengths/boundaries and documented contiguous
   reads. Every logical read sets its pointer. SLLSEN9F §7.5,
   SLLSEP2D/SLLSEQ8D §7.4 and SLLSEO5D §6.5 describe STOP after pointer selection;
   do not mandate repeated START. Record selected adapter framing and physical
   verification still needed.
2. Define one terminal callback's aggregate timeout including lock and all bus
   phases. Callback attempts and physical transactions are different units.
   Buffers cannot outlive return; no pending DMA, retries or inferred NACK phase.
3. Use prompt 3's deliberate mixed-access composer rules. ACK preserves actual
   control settings; unrelated writes emit no IRQ/reset command. Keep 0A
   disableTerm and 45 disableRdRp distinct, including their retained evidence.

## Role changes and base recovery

Base TUSB320/I changes MODE_SELECT only while unattached; this is not a blanket
ban on attached writes. Advertisement may change after attachment (§7.3.2 p12).
A preceding status read cannot
prevent attachment racing the write: document the external owner/fixture
prerequisite and limitation. Extended LI/HI/LAI/HAI requires disable termination,
write desired mode while disabled, wait at least 5 ms after that write completes,
then enable termination. Each write can succeed, fail before effect or fail after
effect. Cancellation cannot restore a previous port state.

Specify the original TUSB320/I dead-battery exit in SLLSEN9F §7.4.3 p14:

1. Caller establishes stable VDD/I2C readiness and decides when source-capable
   startup needs recovery. Dead battery includes absent VDD with no cable.
   Successful read-only identity validation is a library prerequisite.
2. Explicit `startRecoverDeadBattery(timing, token)` reads 0x45 and requires
   TI-internal bits 1:0 to be zero; otherwise CONFIGURATION_UNKNOWN without write.
   This precheck is a conservative library policy, not an extra TI-prescribed step.
3. Write literal 0x04, wait at least 30 ms after completion, write literal 0x00,
   then verify `(reg45 & 0x07) == 0`. This respects Table 7-7 p19's internal-bit
   restriction. Bound it at four callbacks, six TX bytes and two RX bytes,
   excluding bus address phases.
4. Ignore transient status as TI requires; publish no intermediate port snapshot.
   Preserve uncertainty through cancellation/end/result consumption. A final
   zero register cannot prove the required historical sequence and wait.
5. Initialization stays read-only. Reuse one operation engine; no automatic
   replay, cleanup, startup engine or power policy. Reject this operation on
   LI/HI, LAI/HAI and 322I: SLLSEP2D/SLLSEQ8D §7.3.3 p14 and SLLSEO5D §6.4.4 p14
   do not specify it.

## Timing, reset and interrupt evidence

Use [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md) and its exact TI page tables. Record
50/75/100 ms DRP min/typ/max, 2 ms typical VBUS debounce, bus electrical timings
and the base 0.9 ns versus later 0.9 us conflict. Typical debounce and board
700 ms GPIO delays are not operation deadlines. Base §7.3.1.2 p11 recommends
explicit reset to refresh detected current; a plain read does not do that.
Keep that application policy outside refresh/init/IRQ. TUSB322I §6.3.2 p9 says
reset returns to DRP; test this effect without inventing all-register defaults.
Public implementations' sleeps/retries and error cleanup are observations to
audit, not silicon requirements. Correct contradictions across affected contracts.

Verify LAI/HAI's 400 kHz maximum, enable-to-I2C availability maximum 100 ms and
reset min/typ/max 26/49/95 ms in §6.6–6.7. Verify each other variant independently.
The 95 ms reset maximum appears in SLLSEN9F §6.7 p7,
SLLSEP2D/SLLSEQ8D §6.7 p6 and SLLSEO5D §5.7 p7; it is not an attachment or
configuration-trust guarantee. Separate nominal, minimum, maximum and policy.

Use the baseline owner clock: integer milliseconds, resolution at most 1 ms.
Keep named `OWNER_CLOCK_MARGIN_MS = 1` separate from TI values: 5 ms requires
6 elapsed ticks, 30 ms requires 31, and 95 ms scheduling requires 96. Anchor
waits to known completion or the next fresh owner timestamp, never stale
pre-callback time. Status-only callbacks do not provide completion time. Do not
extend the immutable owner deadline or add this margin to it. Include
sub-millisecond completion vectors proving no early wait, and deadline/failure
vectors at each write and verification boundary.

Specify one-shot reset and evidence required afterward; a cleared command bit
alone cannot prove configuration trust or port readiness. No automatic desired
profile reapply. Specify generation-specific IRQ sources, explicit W1C,
observe/publish/ACK/bounded-recheck and reassertion/coalescing limits. There is no
proved atomic multi-byte capture, lossless edge queue or IRQ inter-clear 5 ms
delay. A complete observation survives a later ACK/recheck failure.

322I §6.3.2.1–2 pp10–11 prescribes separate disable-term, DFP/UFP mode and
enable-term writes: clean bytes `01→21→20` / `01→11→10`, preserving other fields.
It specifies no added 5 ms wait. DRP encodings/reset-to-DRP are documented;
qualify a direct non-reset return procedure separately, without hidden reset.
Retain separate advertisement/fault gates and autonomous VCONN effects. Resolve
or retain reset effects, pointer wording, errata, startup polarity and attached-
current conflicts; gate only dependent claims and state board prerequisites.

## Deliver, review and commit

Complete cited procedure tables and source-to-API/CLI/test coverage for every
enabled variant. Include prerequisites, steps, wait anchors, possible effects,
verification, callback/byte ceilings and negative/failure vectors. Later code
comments must carry the exact source references. No implementer should need to
invent silicon facts after this prompt.

Optional delegation: a strong reasoning agent independently reviews sequences
and timing while another checks bounds/vectors read-only. The lead resolves
evidence, edits the authoritative procedure guide and handles Git. Update
decisions, progress, validation and affected README/CHANGELOG; check source
citations, arithmetic, consistency and links. Commit and sync complete checked
work on the same branch, report limits, and then proceed to prompt 5.
