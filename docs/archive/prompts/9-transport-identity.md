# 9 — Implement register transport and identity checks

Complete prompts 1–8 first. Read [COMMON](../../prompts/COMMON.md), the transport, status,
capability and accounting sections of [ENGINEERING_BASELINE](../../prompts/ENGINEERING_BASELINE.md),
the completed TI specification and the current API contract. Deliver a tested
private transport layer and exact identity checks. Managed operations and public
raw diagnostics arrive later; do not add unavailable public methods or stubs.

## Focused audit and delegation

Re-audit PCA9555 `src/PCA9555.cpp` (`_mapTransportResult`, write effects),
ADS1115 transport/error tests and LDC1614 `test/support/FakeLdc1614Device.h`.
Check their current headers, relevant examples, runner assumptions, test
registration, CI and AGENTS. Record revisions, dirty state and adopted/rejected
choices in the progress entry. Borrow exact buffers and preserved backend detail,
not GPIO-pair logic, sensor words or assumptions about physical transactions.

An optional coding subagent may own the scripted fake and protocol tests after
the lead fixes helper signatures. A strong reasoning reviewer may inspect error
and possible-write semantics read-only. Use [AGENT_WORKFLOW](../../prompts/AGENT_WORKFLOW.md),
disjoint file ownership and current runtime models. The lead integrates and owns Git.

## Implement

1. Use one private terminal-callback wrapper and small named register/capability
   tables. Six TUSB320 variants use the exact verified addresses: base 320/I
   `0x60/0x61`, LI/HI/LAI/HAI `0x47/0x67`. Reject invalid variant/address/access
   combinations locally. Gate every TUSB322I capability on its own closed evidence.
2. Allow documented ID `0x00..0x07`, controls/status `0x08..0x0A`, `0x45`
   where present and extended-only `0xA0`. Reads always supply their pointer.
   Accept only verified contiguous ranges; reject holes, wrap and absent registers.
   TUSB322I has no `0x45`; original 320/I must never read `0xA0`.
3. Byte writes send `[register,value]`. Reads send `[register]` and receive the
   exact requested byte count. Use fixed local receive storage and publish output
   only after complete success. The Status-only callback cannot report a short
   transfer: its adapter must reject one before returning success. Preserve output
   and full native `detail` on failure; do not invent a NACK phase.
4. Document STOP-separated pointer/read framing versus repeated START from TI
   evidence. Serialization and one aggregate timeout include lock acquisition and
   all phases. One callback may include two physical transactions. Core/adapters
   add no retry/recovery; pinned SDK internal waits/cleanup need a proved aggregate
   bound and explicit application acceptance. No delayed DMA, retained buffer or
   platform dependency belongs in core. A timeout parameter alone proves no bound.
5. Install accounting at this single invocation boundary now: every actual
   callback increments saturating all-source attempt and success/failure totals
   once. Classify managed versus raw traffic explicitly; collect the baseline
   managed evidence here for later diagnostic views. Accept callback OK/I2C_* only;
   normalize all other/invalid codes to I2C_ERROR, retaining full detail.
   Local rejection is not a transfer.
   Preserve last-failure evidence after success; never invent completion time.
6. Compare the eight ascending identity bytes `30 32 33 42 53 55 54 00`, not a
   host-endian word or reversed string. A matching signature does not identify
   enable polarity. Follow the specification's future-revision policy; extended
   revision `0x01/0x02` cannot expand capabilities. TUSB322I uses its separate ID.
7. Keep possible write acceptance separate from returned Status. This helper
   reports evidence; the later owner engine applies lifecycle policy. Do not
   prematurely expose a second synchronous operation engine.

## Verify and finish this prompt

Re-open [CHIP_EVIDENCE](../../prompts/CHIP_EVIDENCE.md) and its cited source bytes. Test seven-bit
`60/61/47/67` independently from wire address pairs `C0/C1`, `C2/C3`, `8E/8F`,
`CE/CF`; recovery's `45` is a register, not a target address. Keep public code's
unexplained revision `06` outside automatic acceptance. Derive the fake from TI;
the inspected Renode model lacks faithful pointer, multi-byte and W1C semantics.

Create/register `tusb32x_protocol` with fixed-size scripted expectations. Reject
unexpected address, kind, bytes, length, timeout or extra call. Test legal and
forbidden variant/register pairs, exact framing, pointer supplied every time,
range boundaries, wrong/reversed/partial identity and unknown revision policy.
Inject no-effect errors and writes applied before an error; preserve the same
backend failure when phase evidence is unavailable. Test unchanged output,
illegal callback progress/owner errors/invalid enum values, saturating counters
and independent instances. A callback TIMEOUT must not become owner TIMED_OUT.

Expected identity bytes and access cases come from cited TI tables, not production
helpers. Keep tests of future operation scheduling for their owning prompts;
private helper tests are useful evidence but are not a working driver claim.
Run the focused registered suite and strict framework-free compilation.

Update source/file Doxygen, transport and register-map guides, the feature/source/
test matrix, README/CHANGELOG where behavior changed, progress and validation.
Document exact callback/byte bounds and adapter framing still needing hardware.
Review the complete diff, commit the coherent checked work and sync the existing
execution branch. Verify upstream equality and report the commit, checks and real
limitations; no new branch, empty checkpoint or unfinished implementation.
