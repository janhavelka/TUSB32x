# P01 — Establish the exact TI family specification

Read [COMMON.md](COMMON.md). **Re-audit current siblings first:** TMP1x2 and LDC1614
variant handling; ADS1115/PCA9555 register metadata; 24Cxx reference provenance.
Use peers for structure only. Read the [TI audit](../audit/ti-family.md) and verify
against the official PDFs, including visual register/timing tables.

## Work

1. Preserve exact official PDFs, source URLs, retrieval dates, document numbers,
   revisions, SHA-256 hashes and vendor licensing under `docs/reference/`. Check
   for relevant errata/application notes. Do not silently replace an archived
   revision; record differences. Keep downloads out of library source/package.
2. Produce a capability table for TUSB320/TUSB320I, TUSB320LI/TUSB320HI and
   TUSB320LAI/TUSB320HAI. Primary implementation target is LAI. Explicitly assess
   TUSB321/TUSB321A and TUSB322-family claims against public documentation: exclude
   GPIO-only parts; mark unverified interfaces pending instead of inventing support.
3. Specify exact 7-bit addresses and strap semantics, PORT modes, enable polarity,
   I2C max speed, pointer/repeated-start/STOP behavior, increment/burst support,
   ID byte ordering, identity limitations, and revision-register availability.
   An address overlap or shared ID is not full variant auto-detection.
4. For every implemented register, list address, reset value, RO/RW/W1C/self-clearing
   fields, reserved handling, variant mask, decoding, legal values and side effects.
   Focus on mixed `0x08`, `0x09`, `0x0A`, and variant-specific `0xA0`; do not carry
   newer bits into the original silicon. Reject unsupported revision reads.
5. Specify local-vs-peer role meaning, detach, orientation applicability, current
   advertisement/detection, accessory encodings, active cable, DRP duty/debounce,
   Try.SRC/Try.SNK, mode selection, UFP accessory control and termination controls
   exactly where supported. Preserve raw reserved encodings without guessing.
6. Document interrupt sources, W1C acknowledgement, any documented timing limits,
   changes not covered by IRQ, coalescing/races, and status-read coherence limits.
   Specify safe control writes that cannot accidentally acknowledge pending INT.
   Do not invent an inter-clear delay; the mode-change delay is a different rule.
7. Specify startup/reset/role-change sequences and timing from authoritative
   sections. Resolve contradictory wording with evidence; otherwise record the
   ambiguity and a conservative externally managed prerequisite, not an invented
   datasheet guarantee. Separate soft reset from board enable/power sequencing.
8. Capture board constraints: VDD vs 3 A advertisement, I2C pullups/back-power,
   VBUS_DET resistor/input constraints, enable/PORT/ADDR sampling, dead-battery
   behavior, external power switches. The library does not implement USB PD.

## Deliver and check

- `docs/variant-matrix.md`, `docs/register-map.md`, `docs/device-behavior.md`,
  reference manifest, and a feature-to-source/API/CLI/test coverage table.
- Cite document/section/table/page for every substantive mask, timing and sequence.
  Distinguish guaranteed maximum, typical value and chosen software policy.
- Define positive/negative vectors and open questions before implementing codecs.
  Any unresolved fact blocks that dependent operation or support claim, not all
  unrelated implementation. Do not hide omissions behind an overly broad family name.
