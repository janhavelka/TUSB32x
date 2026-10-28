# Private register transport and identity checks

Prompt 9 implements src/TUSB32xProtocol.h/.cpp as a private library boundary.
Its helpers are exercised by the registered tusb32x_protocol suite. The public
owner facade added in prompt 11 uses this same callback path for read-only
initialization. The transport boundary itself owns no lifecycle or scheduler;
public raw diagnostic methods reuse it in prompt 17. See
[diagnostics and health](diagnostics.md) for admission, effects and filtered health.

## Framing and work bounds

Register and address byte notation below is hexadecimal.

| Request | Callback and bytes | Local bounds |
| --- | --- | --- |
| Read | writeRead, TX `[register]`, RX exact requested count | 1..11 bytes wholly within 00..0A, or one present 45/A0 |
| Write | write, TX `[register,value]`, no RX | One precomposed byte to present writable 08/09/0A/45 |
| Identity/revision comparison | No callback | Eight ascending signature bytes or one observed revision |

Every transport helper invokes at most one terminal callback. Local rejection
invokes none and does not change counters, read output or write evidence. There
are no retries, allocation, retained buffers, clocks or framework dependencies.
The receive buffer is fixed at 11 bytes; output is copied only after normalized
OK. A Status-only callback cannot report a short transfer: the adapter must
detect one and return failure before claiming OK.
The fixed host fake rejects a scripted short receive marked OK before copying
bytes. This catches contradictory tests; it does not prove an adapter checks its
actual byte count. Partial receives reported as failures leave public output and
complete observations unchanged.

TI SLLSEN9F section 7.5 p15, SLLSEP2D/SLLSEQ8D section 7.4 p15 and SLLSEO5D
section 6.5 pp15-16 show pointer selection followed by STOP and a new read.
The standard adapter contract uses that sequence, with serialization across
the STOP. A writeRead callback can contain two physical transactions. Repeated
START needs explicit adapter/part evidence. Each read supplies a fresh pointer;
implicit pointer persistence and atomic hardware snapshots are not assumed.

The timeout is the smaller of the positive uint64 available budget and the
configured uint32 millisecond cap, narrowed only after clipping. A zero budget
is INVALID_PARAM here; the later owner decides deadline outcomes before calling.
One aggregate budget includes lock acquisition, both phases, inter-phase timing
and terminal cleanup. The application owns the bus and serialization across all
users, and must supply synchronous callbacks without delayed DMA or reentry.
No core/adapter retry or recovery is added. SDK internal waits/cleanup need an
independent bound audit and explicit application acceptance; a timeout argument
or a lateness check after return cannot prove that bound. Physical STOP/START,
final NACK, timing and SDK cleanup remain adapter/hardware qualification work.

## Access and identity evidence

Base 320/I accepts seven-bit 60/61; LI/HI/LAI/HAI/322I accepts 47/67. Wire address
bytes C0/C1, C2/C3, 8E/8F and CE/CF are adapter output, never callback target values.
Register 0x45 is not a target address. All offsets outside 00..0A and present 0x45/0xA0
are rejected, including holes and wrapping. Base A0 and 322I 0x45 are absent.
Identity and revision registers are read-only. Every 322I 0x08/0x09 write remains
UNSUPPORTED_FEATURE because V_ADVERTISEMENT/V_FAULT_CLEAR are unresolved.

The private write primitive checks register access and receives an already
composed byte. Its caller must enforce identity readiness, actual-state proof,
safe mixed-register composition and silicon procedures. This helper grants no
permission to bypass mode/reset/recovery prerequisites; composition is prompt 10
and owner enforcement is implemented. Public raw writes share the composer
and require current proof; they cannot execute multi-step procedures.

Identity comparisons require all eight ascending bytes, including the NUL:
`30 32 33 42 53 55 54 00` for the six 320 parts and
`32 32 33 42 53 55 54 00` for 322I. Wrong length/null is INVALID_PARAM;
complete mismatches return CHIP_ID_MISMATCH. LI/HI requires revision 0x01 and
LAI/HAI/322I requires 0x02. Other values, including unexplained public-code 0x06,
return CHIP_ID_MISMATCH. Base revision comparison is UNSUPPORTED_FEATURE and
never causes an A0 read. Raw evidence remains with the caller, unchanged by
comparison. Matching bytes do not infer enable polarity or expand capabilities.
Sources: SLLSEN9F Table 7-7 p16; SLLSEP2D/SLLSEQ8D Tables 7/12 pp16/20;
SLLSEO5D Tables 6-7/6-11 pp17/20. See the [register map](register-map.md) and
[variant policy](variant-matrix.md#identity-and-revision-policy).

## Status, accounting and possible writes

Only OK and documented I2C_* callback statuses are accepted. Every other code,
including IN_PROGRESS, owner TIMEOUT/BUSY/CANCELLED and invalid enum values,
becomes I2C_ERROR with full int32 backend detail and static explanatory text.
Legal callback status/detail/text are preserved. I2C_TIMEOUT stays a transport
failure, never an owner TIMED_OUT outcome; generic errors never imply a NACK phase.

At the single invocation boundary, totalAttempts increments before the callback;
exactly one success/failure total and lastStatus update after normalization.
All counters saturate independently. Managed callbacks also update their own
totals/status/streak; success resets the managed streak. Raw callbacks never
change managed evidence. A retained last transport failure survives later
success. Affected completion-time flags become false; old numeric timestamp
storage remains unusable history. No callback supplies a completion timestamp
or changes initialized, identity, configuration trust or lifecycle state.

Each result separately records whether its callback ran and write acceptance:
NOT_ATTEMPTED for reads/local rejection, CALLBACK_SUCCEEDED for an OK write,
POSSIBLE for every invoked failed write. Status alone has no separate phase/byte
evidence, even for NACK_ADDR or I2C_BUSY. The fake proves that the same failure
can occur before or after a device mutation. Callback success does not establish
the final state or public HardwareEffect::CONFIRMED; later readback/owner policy
must do that. See [accounting contracts](api-contracts.md#transport-accounting-and-passive-health).

## Arduino integration

The example-only [Wire adapter](arduino-transport.md) implements terminal callbacks
for the pinned S2/S3 SDK. Its shared gate, configuration prerequisites, divided
aggregate budget and failed-receive controller retirement are adapter constraints.
Portable core code does not include Wire.
