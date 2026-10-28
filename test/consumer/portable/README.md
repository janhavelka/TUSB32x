# Public-only portable consumer

This standalone C++17 project imports a previously built static library and its
public include directory. It neither adds the repository as a subdirectory nor
runs a generator. Its configure/build need CMake and a C++17 toolchain; Git,
Python, Doxygen, framework headers and sibling projects are unnecessary. The
maintainer isolation harness copies these sources, public headers and the built
library into a temporary directory and restricts the child tool search path.

From the repository root, first build the library with tests disabled:

```powershell
cmake -S . -B build/portable-library -G Ninja -DCMAKE_BUILD_TYPE=Release -DTUSB32X_BUILD_TESTS=OFF
cmake --build build/portable-library
```

Then configure the independent consumer, replacing the absolute artifact paths
and static-library filename for the selected compiler:

```powershell
cmake -S test/consumer/portable -B build/portable-consumer -G Ninja -DCMAKE_BUILD_TYPE=Release -DTUSB32X_PUBLIC_INCLUDE_DIR=C:/path/to/TUSB32x/include -DTUSB32X_LIBRARY_FILE=C:/path/to/TUSB32x/build/portable-library/libTUSB32x.a
cmake --build build/portable-consumer
./build/portable-consumer/tusb32x_portable_consumer.exe
```

The two translation units use only public headers. Together they link and call
every currently callable facade method, plus the public validators, register
decoder, health projector, Status helpers and error naming. The executable
checks read-only initialization/profile observation, explicit advertisement and
role apply, desired verification, reconciliation, port refresh, interrupt ACK and
service, reset, original-part dead-battery recovery, raw diagnostics, lifecycle,
cancel/result ownership and cached evidence. An independent fixed register model
provides callbacks; this is software evidence, without a hardware or framework
qualification claim. Whole-object output preservation uses byte comparisons of
the same object before/after a rejected call, including its unchanged padding.

Checks use explicit branches and remain active with `NDEBUG`. Core exercise,
including construction and destruction, runs with global C++ allocation and
deallocation traps armed. Ordinary, array, sized, nothrow and C++17 aligned forms
are covered. If a compile/link probe accepts GNU linker wrapping, direct
`malloc`, `calloc`, `realloc` and `free` references are also trapped. The output
reports whether wrapping is enabled; `TUSB32X_WRAP_C_ALLOCATIONS=OFF` explicitly
disables it. Wrapping applies to linked references and cannot inspect allocations
inside dynamically linked runtime libraries. This evidence covers the exercised
paths; it does not prove every conceivable branch allocation-free.

Three deliberately failing sentinel modes verify the executable checks:

| Argument | Required process exit | Meaning |
| --- | --- | --- |
| `--fail-check` | 1 | An explicit failed check survives Release optimization. |
| `--allocate-probe` | 86 | An armed global C++ allocation is trapped. |
| `--malloc-probe` | 86, or 77 without wrapping | An armed direct C allocation is trapped, or unavailable. |

Run them as separate processes. The allocation traps exit immediately without
runtime cleanup or diagnostics. A nonzero sentinel exit is expected and must be
checked by the caller; running only these modes is not a normal consumer pass.

The registered `tusb32x_portable_consumer` CTest invokes the maintainer harness
`test/consumer/run_portable.py`. On Windows it verifies a restricted child PATH
without Git, Python, Doxygen or PlatformIO; only compiler runtime DLLs are copied
onto that PATH. Other hosts retain their normal PATH and do not claim tool absence.
The coordinator forwards parent compile/link flags, including configuration flags,
so ABI and sanitizer requirements follow the imported objects. When building
manually, supply the same required flags used to build the library. Temporary
trees are retained under the selected build tree for inspection.
