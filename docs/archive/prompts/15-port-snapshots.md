# 15 — Implement complete port observations and freshness

Complete [prompt 14](14-reset-recovery.md) first. Read [COMMON](../../prompts/COMMON.md),
the PortSnapshot fields/validity and time sections of
[ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md), and the completed TI status matrix.
Deliver `startRefreshPort` and the cached `portSnapshot` getter through the existing
engine and codecs. IRQ acknowledgement is a separate following prompt.

## Focused audit and delegation

Re-audit BME280 configuration trust versus communication success, LDC1614
publication/staleness tests and INA228 status/interrupt observation boundaries.
Inspect relevant current headers, sources, examples, CLI/runner output, tests,
CI and AGENTS; record revisions/dirty state and selected differences. Sensor
measurement freshness and chip-side atomic capture must not be assumed here.

A coding subagent may own decoder/publication tests in `test/test_port.cpp`.
Keep shared engine/publication changes with the lead; a strong reasoning reviewer
may inspect mixed-time coherence and invalidation read-only. Follow
[AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md), disjoint edits and lead-only Git.

## Implement

1. Add read-only `startRefreshPort` using the same admission, token, deadline,
   budget and retained result rules. `portSnapshot` returns a cached copy with no
   transfer, hidden refresh, ACK, policy decision or logging.
2. Use existing pure codecs for attachment, orientation, current, accessory and
   active cable. Attachment is relative to the local port; requested mode is
   separate configuration evidence. Preserve distinct local/peer role and
   variant-specific debug/charge-through meanings. Unknown/reserved raw values
   keep raw evidence with invalid typed fields, never fabricated detach/CC1.
3. Source advertisement differs from local-sink detection. Neither measures
   load, establishes PD or grants board power permission. Default current does
   not unconditionally mean 500 mA. Preserve unresolved TI wording about current
   tracking after attachment as a documented limit.
4. Stage fixed 0x09/0x08/0x09 reads in a candidate; publish the final 0x09.
   No transport retry or restart until stable. Publish only complete raw port
   observations; a failed later read leaves the previous snapshot intact, with
   its historical time/sequence and validity. Partial raw evidence may exist
   separately only with explicit validity. A successful read is not IRQ ACK.
5. Populate the baseline raw bytes/register mask, per-field validity, sequence,
   acquisition time/time validity, configuration generation/trust and coherence.
   Compare 0x09 bookends under 0xE0. Matching gives BOOKENDS_MATCH, never atomic
   capture. CHANGED_DURING_READ preserves raw bytes but clears DETECTED_CURRENT,
   ACCESSORY and ACTIVE_CABLE validity; current/accessory become UNKNOWN.
   Independently decoded final 0x09 fields and 0x08 advertisement may remain valid.
6. Use owner time before the observation's first required read as its acquisition
   boundary. Do not replace it with a later publication/fresh-poll timestamp and
   make old data look new. Describe it as a conservative acquisition boundary,
   not a precise hardware sampling instant. Sequence counts publications, not
   electrical edges; start at zero and saturate at UINT64_MAX. Saturated sequence
   is not unique; tokens correlate operations.
7. `invalidated` starts true, clears only on complete fresh publication, and
   becomes true on end/external invalidation or before a possible port-affecting
   write. Preserve historical fields/time/provenance when invalidated changes.
   Configuration readback cannot clear it. Even uninvalidated data can be old;
   its age remains an application decision. Raw completeness does not make all
   fields usable: consumers check per-field validity and coherence.
8. Detached orientation/current can be valid `NOT_APPLICABLE` interpretations;
   valid decoding is not usability. Keep attachment, current applicability,
   configuration trust, transport health and operation outcome independent.
   No core GPIO, ISR, event queue or USB power/data policy is required.

## Verify and finish this prompt

Use [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md) to reject Linux's guessed detached roles and
unqualified orientation/VCONN projections. Base SLLSEN9F §7.3.1.2 p11 recommends
explicit reset to update detected current; refresh reads must never reset the
chip. Preserve the separate extended-current uncertainty. Model CC and VBUS
independently: CC alone cannot establish Attached.SNK. Raw active-cable evidence
is not usable current or proof of powered VCONN.

Register `tusb32x_port`. Cover every supported status encoding and reserved value,
local versus peer roles, not-applicable versus unknown, raw preservation and field
validity. Independently expected values must not reuse production decoding.

Test failure at each of the three reads, changed bookends without retries, invalid
cross-register fields, independently valid final 0x09 fields, old snapshot retention,
no fake detach, and delayed publication retaining the original acquisition time.
Check sequence/configuration provenance, complete-only publication and all
invalidation sources using public operations from earlier prompts. Configuration
verification alone must not refresh port data. Exercise budgets 0/1/larger,
cancel/error/deadline at each read and no writes during refresh/cached access.
Confirm reset/recovery transients do not publish port snapshots. TUSB322I fault
stays raw 0x09 bit 3 with capability/access context; no unverified typed fault ACK.

Update public/member/source Doxygen, API/architecture/variant guides, feature
matrix, README/CHANGELOG, progress and validation with actual test commands and
counts. Document that connector orientation, attached-current tracking and
electrical behavior require separate physical evidence. Run focused port and
affected owner regressions plus documentation checks. Review, commit and sync
the same execution branch; verify upstream equality before reporting completion.
