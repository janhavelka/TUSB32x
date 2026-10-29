# Seeded fuzz tests and fault injection

These tests vary input and inject failures into the real CLI, owner engine,
serial-session parser and archive reader. They need no board, serial port or SDK.
They use the existing C++ compiler, Python standard library and test fakes.

A seed selects a repeatable sequence of cases. This is bounded mutation fuzzing:
it does not use coverage feedback, search indefinitely or prove that every input
is safe. Existing explicit protocol and timing tests remain necessary.

## What is checked

| Suite | Input and injected faults | Required result |
| --- | --- | --- |
| `tusb32x_fault_fuzz` | Malformed write commands, fragmented input, short receives, failed reads/writes, applied-then-error writes, varying poll budgets and stalled output | Invalid commands cause no writes; callbacks stay within budget; original errors and possible effects survive; a full output queue cannot consume a retained result |
| `tusb32x_boundary_fuzz` | Corrupt record fields, split traffic, missing/duplicate/stale/foreign completion records, failed serial I/O and capture exhaustion | No false success or foreign result take; failed sessions retain their owned token and failure report |
| `tusb32x_boundary_fuzz` | Unsafe archive paths, links, duplicate members, header damage, truncation and size limits | Reject before extracting files; an outside sentinel stays unchanged; valid controls still extract |

The C++ campaign uses the existing fixed device fake. It models software callback
outcomes, including a write that took effect before returning an error. It does
not model electrical timing or qualify the I2C adapter. The Python campaign uses
the real session and artifact code with a virtual serial peer. Archive cases call
the real checked extractor; they never compile or execute archive contents.

## Run with existing tools

From the repository root:

```text
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug -R "tusb32x_.*fuzz" --output-on-failure
```

The normal validation command also runs these suites. The native group owns the
C++ campaign and the Python group owns the serial/archive campaign; neither is
registered twice. Each CTest has a 30-second limit. Release builds keep checks
active. Direct campaigns accept finite case counts and print their seed.

For a larger repeatable run with a multi-configuration build on Windows:

```text
build\native\test\Debug\tusb32x_fault_fuzz.exe --seed 12345 --cases 1024
python tools/test_boundary_fuzz.py --seed 0xdeadbeef --cases 256
```

For a single-configuration Linux build, the executable is normally
`build/native/test/tusb32x_fault_fuzz`. A C++ failure prints its case seed and a
`--cases 1` replay command. Python reports the campaign seed, case index and
mutation; replay that seed with enough cases to include the index. Fix the defect
and keep a focused regression for that boundary. Do not change an expected
failure into success merely to accept the current implementation.

## Sanitizers and limits

The existing Linux Clang ASan/UBSan CI row runs the C++ campaign automatically.
ASan checks memory access; UBSan checks selected undefined operations. These
checks instrument the C++ executable. They do not instrument Python's interpreter
or prove archive/session correctness; explicit Python assertions check those
rules. The Python CI owner runs that separate campaign.

No sanitizer or SDK needs to be installed on the working Windows machine to use
the existing remote row. A normal local run is useful but is not sanitizer
evidence. Test success is software evidence only. Physical bus timing, USB/IRQ
wiring and exact-board behavior still require a fixture.
