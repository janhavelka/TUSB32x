# P04 — Implement exact register transport and variant decoding

Read [COMMON.md](COMMON.md), [ENGINEERING_BASELINE.md](ENGINEERING_BASELINE.md),
[MAINTAINABILITY.md](MAINTAINABILITY.md), P01 outputs and P02 contracts. Implement
the protocol now, with its tests and documentation. P07 is additional coverage,
not a reason to postpone tests.

## Re-audit before editing

Refresh current source, tests, examples, runners and CI for the relevant
peers. The clean 2026-10-23 inspected baselines are:

- PCA9555 `498178e4d9e4acb6fab7d8b816ad5021efe8806a`,
  `include/PCA9555/CommandTable.h`, `include/PCA9555/PCA9555.h`,
  `src/PCA9555.cpp` (`_mapTransportResult` and write-effect handling),
  `test/test_basic.cpp`, `test/test_public_contracts.cpp`. Reuse explicit
  uncertainty and register constants. Do not import GPIO-pair logic, pointer
  parking or erase side-effect evidence merely because a binding ends.
- ADS1115 `381236ebfed813cd7a9aca34520ce031d8a0ff6b`,
  `src/ADS1115.cpp`, `test/test_basic.cpp`: preserve native error detail and
  test applied writes followed by errors. Its 16-bit words are not this protocol.
- LDC1614 `c0ab4c1712c4135b925354d70c4e6bed5cca62d8`,
  `src/LDC1614.cpp`, `test/support/FakeLdc1614Device.h`,
  `test/test_basic.cpp`: adopt explicit capabilities and scripted phase failures.

Recheck INA3221 transfer/status handling and TMP1x2 register access as selected in
P00. Cite files/revisions and record adopted or rejected conventions. Re-read the
exact archived TI register/programming sections listed in P01.

## Implementation shape

Use named `constexpr` register addresses, masks and timing values in the audited
register-table location. A small descriptor or explicit switch should express
variant differences. Keep pure codecs and write composition easy to test. Do not
introduce inheritance per chip, a generic register virtual machine, plugin
registry, generated DSL or a callback framework for this small register map.
Split files by responsibility when it helps a reader, not to satisfy a line limit.

1. **Validation and access.** Support both documented addresses for each of the
   six TUSB320 variants. Base 320/I uses `0x60/0x61`; LI/HI/LAI/HAI uses
   `0x47/0x67`. Allowlists cover ID `0x00..0x07`, status/control `0x08..0x0A`,
   `0x45`, and extended-only `0xA0`. Reject unknown variants, illegal addresses,
   unavailable controls and undocumented ranges before calling transport.
   Raw diagnostics obey the same evidence and side-effect constraints. A raw
   one-callback write must not bypass a required multistep mode or dead-battery
   recovery procedure; reject sequence-changing writes and direct callers to
   the typed operation.
2. **Transfers.** A byte write sends `[register, value]`. A read explicitly
   supplies `[register]` then receives the exact requested bytes. Implement only
   verified contiguous ranges; reject crossing undocumented holes or pointer
   wrap. Document STOP-separated versus repeated-START adapter framing without
   calling repeated START a TI requirement. Serialization covers the complete
   pointer/read access. Bound bytes and physical phases separately from callbacks.
3. **Terminal result contract.** Match P02 callbacks. Success means the whole
   requested transfer completed. If the callback only returns `Status`, its
   adapter must reject short transfers; the core cannot infer missing byte counts.
   Adapter-local count/effect evidence must be validated before returning
   success. A generic NACK/error is not proof no data reached the chip. Preserve
   native `detail`, timeout, possible write effect and unchanged read output.
   Use a local fixed-size receive buffer; publish it only on complete success.
4. **Budgeting.** Each callback attempt consumes one `maxTransfers` unit,
   including failures. A combined logical read may contain two STOP-separated
   bus transactions. No callback gets a fresh full timeout for each phase. Do
   not add hidden retries or consume a second callback in a one-callback poll.
   Leave scheduling to P05; keep low-level access reusable by its explicit phases.
5. **Identity.** Compare all eight ascending bytes
   `30 32 33 42 53 55 54 00`; never compare a host-endian integer or reversed
   string. Do not read `0xA0` on base silicon. Extended `0x01/0x02` revision
   evidence does not discover enable polarity. Follow P01 future-revision policy
   and report mismatch/unknown without silently expanding capabilities.
6. **Codecs.** Cover every P01 encoding, including base versus extended debounce,
   reserved advertisement 3 and preference 2, accessory 7 and local source/sink
   meaning. Store raw status with validity/applicability. No detached port becomes
   a usable CC2 orientation merely because direction resets to one. Current is
   CC signaling; it is not a current measurement or permission to draw power.

## Mixed-register composition: required rules and examples

Use the P01 verified masks below. Persistent RW masks exclude command/W1C bits.
These examples define bytes, not permission to change roles in one unsafe step.
Managed mode changes must follow P05's full procedure.

| Operation | Required composition |
| --- | --- |
| `0x08` advertisement | Set only legal `0xC0` field; live lower bits cannot become settings |
| Base `0x09` settings | Use RW mask `0x06`; write IRQ `0x10` as zero |
| Extended `0x09` settings | Use RW mask `0x07`; write IRQ `0x10` as zero |
| Explicit `0x09` IRQ ACK | Preserve current observed/trusted applied RW fields, OR `0x10`; never OR a staged profile |
| Base `0x0A` ordinary settings | Persistent mask `0xF0`; reset `0x08` must be zero |
| Extended `0x0A` ordinary settings | Persistent mask `0xF7`; reset `0x08` must be zero |
| Explicit soft reset | Compose verified applied persistent fields and `0x08`; record that effects may outlive an error/cancel |
| `0x45` Rd/Rp disable | Change only `0x04`; preserve TI-internal `0x03`; do not expose internal bits as editable settings |
| Base dead-battery recovery | After identity validation, read `0x45`; require internal `0x03` bits zero, write literal `0x04`, wait at least 30 ms, write literal `0x00`, verify `0x07` bits zero |

Write zero to reserved/RO positions where specified by the verified map; do not
echo live status under the pretext of preservation. For `0x45`, verify the
TI-internal-bit rule and preserve observed values without inventing a recovery
write if they differ from the documented default. No generic byte RMW helper
may bypass these access rules.

Minimum golden examples, independently checked against TI:

- LAI `0x09` read `0xB5` means applied RW `0x05` with IRQ pending. A staged
  profile `0x02` must not influence ACK: expected write `[0x09,0x15]`, not
  `[0x09,0x12]`. An explicit settings update to `0x02` writes `[0x09,0x02]`
  and keeps IRQ pending.
- Base `0x09` read `0xB4`: expected ACK `[0x09,0x14]`. Reserved bit 0 is never
  introduced by an extended profile.
- LAI `0x0A` read `0xDE`: changing debounce to encoding 1 composes `0x56`,
  with reset bit zero; it must not write `0x5E` and replay reset. No implicit
  mode change or termination release is allowed as part of another setter.
- Extended DRP from clean control `0x00` follows `0x01`, `0x31`, wait at least
  5 ms after completion of `0x31`, `0x30`. Preserve other applied fields in a
  nonzero profile. `0x45` is not a replacement for the first/last step.
- `0x45` preservation: use `(observed & 0x03) | requestedDisableBit` for
  the documented fields if P01 confirms that policy; never zero internal bits
  merely because a default constant is available.
- Base `startRecoverDeadBattery`: after successful identity validation, read
  `[0x45]` and require `(value & 0x03)==0`; then write `[0x45,0x04]`, wait
  at least 30 ms after completion, write `[0x45,0x00]`, read `[0x45]` and
  verify `(value & 0x07)==0`. The 1 ms owner-clock margin makes the wait
  31 elapsed ticks from the fresh anchor. An initial value `0x01`, `0x02`
  or `0x03` rejects before any write. This is the literal sequence from
  SLLSEN9F §7.4.3 p14, reconciled with its internal-bit rule in Table 7-7 p19.
  A read of final zero without the sequence cannot establish recovery.
  P05 schedules these phases in the existing engine; no implicit initialization
  write, soft reset, profile replay, IRQ ACK or automatic cleanup is added.
  Do not publish status sampled between the two writes as port observations.
  The application owns the stable VDD-return prerequisite and decides when the
  source-capable startup procedure is needed. LI/HI, LAI/HAI and TUSB322I reject
  this operation before transport; their dead-battery sections do not prescribe
  it (SLLSEP2D/SLLSEQ8D §7.3.3 p14; SLLSEO5D §6.4.4 p14).

If actual RW state is unknown, reconcile it in an explicit bounded operation or
return an unavailable/untrusted result. No ACK or setter may apply a desired
profile opportunistically. Reconciliation is evidence gathering, not blind replay.

For the TUSB322I extension, use its separate signature/defaults/allowlist and
capabilities only when P01 closes the corresponding facts. Do not access `0x45`;
mode 00 is DRP. Keep IRQ `0x10` and VCONN fault `0x08` acknowledgement separate,
so clearing one cannot silently clear the other. Advertisement access ambiguity
must remain a visible feature gate until resolved.

## Diagnostics and tests in this stage

Every callback contributes exactly once to all-callback transport statistics.
Explicit raw Sync traffic is excluded from managed health projection, never from
the source counters. The CLI probe alias is managed initialization, not a raw
address-only probe. Failed local validation performs no transport and increments
no bus-failure counter. Raw writes report effects and invalidate affected trust.
Getters and reset-statistics helpers are passive and cannot repair trust.

Add focused protocol and codec suites using fixed-size scripted expectations
and a small W1C/reset/live-status fake. Required cases:

- Each legal address, each forbidden variant/register pair, wrong/reversed ID,
  partial ID read, unknown revision and base revision-read rejection.
- Exact TX/RX lengths and bytes, explicit pointer each time, range boundary and
  wrap rejection, success/failure with unchanged output, useful native detail.
- All field encodings and invalid enum casts. Iterate the small raw byte domain
  where useful; expected values must not call production codecs or mask helpers.
- All composition examples above, staged-versus-applied ACK, IRQ arriving
  between read and write, reset bit read as one, and preservation of internal bits.
- Recovery vectors: exact literal writes, unexpected internal-bit rejection,
  unsupported variants, no transient port publication during the sequence,
  readback mismatch and failure after either write. P05 tests the 31-tick wait,
  cancellation/deadline boundaries and preservation of possible effects.
- Fault before callback, error with no application, error after application,
  short transfer where exposed, late success, and unavailable phase information.
  Prove no hidden write replay and no health/trust repair from unrelated success.

Run focused native tests and strict framework-free compilation. Record adapter
framing tests still requiring hardware. Update `docs/register-map.md`,
`docs/variant-matrix.md`, transport documentation, feature matrix and validation
records now. Add Doxygen to public register/capability/diagnostic contracts and
source comments beside non-obvious composition with exact TI revision/section/
table citations. Explain why W1C/reset/applied fields are handled this way; do not
fill code with comments that merely restate assignments.
