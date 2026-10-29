# TI clarification requests before board design

Research date: 2026-10-29. No hardware exists for this project yet; there are no
lot codes, waveforms or measured failures to report. These are prepared questions,
not submitted support requests. No TI contact or forum post was made.

Fresh official TI PDF reads still identify SLLSEQ8D, SLLSEO5D and SLAAE24.
The existing archived sources/provenance remain unchanged. Targeted public TI
E2E searches did not locate a directly applicable resolution; native E2E search
pages were unavailable to the browsing tool. This bounded search is not proof
that TI has never answered these questions.

The [variant matrix](variant-matrix.md#unresolved-gates-and-source-conflicts)
remains the authoritative library gate list. A forum answer must retain its
author/date/exact-part scope, separately from a corrected datasheet. No access
permission or capability is expanded by this research.

## First request: TUSB320LAI startup and attached current

Suggested subject: **TUSB320LAI: EN_N startup polarity conflict and attached Rp tracking**

We are preparing a new TUSB320LAI design and a conservative I2C owner library.
There is no assembled board yet. Please clarify these documentation differences:

1. SLLSEQ8D pin table p3 specifies an externally driven EN_N low interval of at
   least 50 ms after valid VDD; SLAAE24 §4.1 p5 specifies high instead. Which is
   correct for LAI? Please provide the valid-VDD threshold, timing anchor, allowed
   ramp, earliest I2C access and correct externally driven waveform. Is the
   application's alternative EN_N-to-ground wiring valid for LAI across its
   documented supply/ramp conditions? Please distinguish active-high HAI.
2. SLLSEQ8D §7.2.1.2 p10 describes a single current report while attached, while
   SDAA284 Figure 2-1 p8 advertises active current detection from the L generation.
   On LAI, does changing remote source Rp without detach/reset update
   CURRENT_MODE_DETECT/GPIO? What debounce/update bound and interrupt behavior
   apply? Does the base part's explicit reset advice apply to LAI in any case?
3. Table12 p20 documents revision02 at A0. Public third-party code contains a
   commented assertion for06; this project has not measured06. Are any production
   LAI orderable parts/lots specified to return06? If so, which silicon revisions,
   capabilities and errata distinguish them?

Sources: [SLLSEQ8D](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf),
[SLAAE24](https://www.ti.com/lit/an/slaae24/slaae24.pdf),
[SDAA284](https://www.ti.com/lit/pdf/sdaa284).
Current decision: no universal enable waveform; no inferred live current
resampling; only documented revision accepted. Prefer a corrected document or
exact-part written guidance before releasing a design that depends on an answer.

## Separate request: TUSB322I access and mode procedures

Suggested subject: **TUSB322I: 08 advertisement access, 09 fault clearing and return to DRP**

Our optional TUSB322I support is restricted to evidence-supported operations.
Please clarify the following in [SLLSEO5D](https://www.ti.com/lit/ds/symlink/tusb322i.pdf):

1. Figure6-3 p17 labels 08[7:6] RW; Table6-8 labels it R; §6.3.3 p11 describes
   programming. Is it writable? What whole-byte values preserve unrelated fields,
   what attached-state restrictions apply, and when does Rp change take effect?
2. Table6-9 pp18–19 marks fault09[3] RCU but specifies no clear transaction.
   Does reading clear it? What does writing0 or1 do? How can IRQ09[4] be ACKed
   or duty/accessory controls changed without accidentally consuming or clearing
   fault evidence? What happens while overcurrent persists? Please supply exact
   safe transaction examples and distinguish the fault from IRQ W1C.
3. §§6.3.2.1–2 pp10–11 prescribe disable/select/enable for DFP/UFP; §6.3.2 p9
   establishes reset-to-DRP and Table6-10 p20 defines DRP encodings. Is a direct
   non-reset return from either forced role to DRP supported? Which sequence,
   code, waits and attachment restrictions apply?
4. The attached-sink descriptions say "once in the Attached.SNK state". Does
   that mean after entry, or only one detection per attachment? Please specify
   current-report updates/IRQ when the remote Rp changes without detach/reset.

Current decision: advertisement changes, every09 write (including IRQ ACK and
service), and direct non-reset apply-to-DRP remain unavailable. Raw fault reads
retain evidence but are not asserted to be non-destructive. No guessed fault
clear API or hidden reset is provided.

## Additional questions if a design depends on them

| Gate | Exact question and existing conservative decision |
| --- | --- |
| ID_OFF_RATING | A/V absolute-max tables put ID at VDD+0.3 V but leakage is specified at VDD=0, ID=5 V. Ask for an explicit supply-off operating rating before using an independent ID pull-up; VDD-tracking wiring avoids relying on that exception. |
| SUPPLY_SUMMARY | L/A operating tables stop at5.0 V, while family summaries say5.5 V. Keep5.0 V ceiling unless exact-part limits are corrected. |
| BASE_VALID_TIME | SLLSEN9F §6.6 p7 prints0.9 ns data/ACK validity versus later0.9 us. Ask TI for the original-part correction; do not silently qualify the printed timing. |
| L_CC_RATING | SLLSEP2D p4 gives conflicting CC limits, with a revision-history correction. Ask for the exact limit if selecting LI/HI; do not infer LAI limits from it. |

## Resolution record

For each response save its URL/date/author, exact part/document revision, precise
answer and remaining conditions. Check it against other exact sources. Archive
new documents with provenance when adopted; preserve superseded bytes. Update
contracts/tests/code together only if the answer actually justifies a capability
change. Later physical results need identified board/part/firmware/trace and do
not establish family-wide behavior.

A wording correction from this review: unlike LAI's explicit single-report text,
322I's "once in" wording can mean after entering the state. It does not alone
prove a one-report limit, nor continuous tracking. The gate remains unresolved.
