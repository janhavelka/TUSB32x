# 10 — Implement variant codecs and safe write composition

Complete [prompt 9](9-transport-identity.md) first. Read [COMMON](../../prompts/COMMON.md),
the settings/variant and silicon rules in [ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md),
and the completed TI register matrix. Deliver pure, tested codecs and one safe
write composer. This prompt does not execute role, reset, recovery or IRQ jobs.

## Focused audit and delegation

Re-audit PCA9555 `CommandTable.h` and mixed-register write tests, LDC1614
capability handling and TMP1x2 access validation. Inspect the relevant current
headers, implementation, examples, CLI/runner expectations, tests, workflows and
AGENTS; record revisions/dirty state and concrete decisions. Re-read the exact
archived TI tables; sensor register assumptions do not transfer to this device.

A coding subagent may implement independent codec test vectors in an assigned
test file. Use a strong reasoning reviewer for W1C/reset and variant ambiguity;
the lead owns production composition and shared declarations. Follow
[AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md); agents do not change Git or sibling files.

## Implement

1. Use small explicit tables/switches and named `constexpr` masks. Keep pure
   helpers private except the pure `decodeRegister` contract frozen in prompt 5.
   Implement that shared public entry point with its fixed value result, raw and
   unknown/reserved validity, unchanged output on error, and no I2C/state mutation.
   CLI formatting will use it in prompt 22; do not require a private source include
   or duplicate field conversion. Test and link the public decoder now.
   Single-byte decoding reports raw field meanings; only complete port decoding
   can establish attachment-dependent applicability. Preserve profile raw/field
   validity through reserved codes and absent variant controls.
   No chip inheritance,
   generated register language, registry or generic unsafe byte RMW abstraction.
2. Cover every specified mode, source advertisement, current detection,
   attachment, orientation, accessory, active cable, duty, preference and debounce
   encoding. Respect the original versus extended debounce mapping, advertisement
   3 and preference 2 reserved values, accessory 7 and local-source/local-sink
   interpretation. Invalid enum casts and unsupported variant/settings pairs fail
   without traffic. Keep raw evidence and typed validity separate.
3. In complete port decoding (prompt 15), detached direction/current decode to
   valid `NOT_APPLICABLE` interpretations, never usable CC2/default-current claims.
   The frozen single-byte decoder above retains raw field codes without paired
   applicability. CC indication is not measured current,
   USB PD, an unconditional 500 mA allowance or authorization to draw power.
4. Use persistent masks: advertisement `0xC0`; base/extended `0x09` settings
   `0x06/0x07`; base/extended `0x0A` settings `0xF0/0xF7`. Ordinary settings
   writes keep IRQ W1C `0x10` and reset command `0x08` zero. Do not echo live RO
   or reserved bits as settings. Explicit ACK uses current observed/trusted
   applied fields, never staged desired settings.
5. Compose explicit reset with proved persistent fields plus its command bit.
   Normal `0x45` control changes only `0x04` and preserves documented internal
   `0x03` bits. The original-device recovery procedure is separately qualified:
   initial internal bits must be zero before its literal `0x04` and `0x00` writes.
   A composer never grants permission to skip a required owner sequence.
6. Keep `disableTerm` at `0x0A` distinct from `disableRdRp` at `0x45`. Capability
   gates and required actual-state inputs prevent an ordinary/raw setter from
   bypassing mode/reset/recovery preconditions. Missing proved control state
   requires explicit observation/reconciliation or failure, never desired replay.
7. Keep TUSB322I separate: its mode 00 means DRP; semantic comparison may accept
   equivalent DRP encodings according to the specification. No `0x45`; IRQ
   `0x10` and raw VCONN fault `0x08` are independent. Initial fault evidence stays
   raw with capability/access context; no typed fault-ACK API. Unresolved
   advertisement access or fault-clear wording remains visibly gated.

## Verify and finish this prompt

Use [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md) for public-code counterexamples, rechecking
TI tables first: `08=90` separates advertisement/detection; `08=20` is distinct
charge-through current; Try.SRC emits `06`, never reserved `04`. Base `0C` means
`Accessory::DEBUG`, while base `0E` is reserved. Assert actual reset output bit
`08`, not the TI-hosted driver's ineffective mask-08/value-1 call. Add ACK
`06 -> 16` with unrelated fields preserved; fixed `11` changes controls.

Extend `tusb32x_protocol` using expected values independently derived from TI:

- Extended `0x09=0xB5`, desired `0x02`: ACK is `[0x09,0x15]`; explicit settings
  `[0x09,0x02]` preserves a pending IRQ. Base `0xB4` ACK is `[0x09,0x14]`.
- Extended `0x0A=0xDE`, debounce encoding 1: compose `0x56`, never `0x5E`.
- Clean extended DRP sequence bytes are `0x01,0x31,0x30`; this prompt tests
  composition only. Prompt 13 proves the required inter-write timing.
- Ordinary `0x45` composition is `(observed & 0x03) | requestedDisableBit`;
  recovery accepts internal zero and requires exact literal writes. Prompt 14
  executes and verifies the complete sequence.
- Exercise all small field domains, reserved encodings, invalid typed input,
  nonzero preserved fields, live-status changes, staged/applied disagreement,
  pending IRQ and a reset bit read as one. Expected tests must not reuse codecs.

Run the focused registered tests and strict host build. Add useful source comments
with TI revision/section/table citations beside non-obvious masks, plus Doxygen
for changed public register/capability contracts. Update register/variant guides,
feature matrix, decisions, README/CHANGELOG as applicable, progress and validation.
Review and commit this complete unit; sync and verify the same execution branch.
Report what now works and the later operation stages without claiming them done.
