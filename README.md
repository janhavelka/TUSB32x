# TUSB32x

TUSB32x is a framework-neutral C++17 library in development, starting with
TI TUSB320LAI. It currently provides passive public values, constexpr Status
helpers, pure binding/profile validators and stable package version constants.
The library also contains tested [private transport/identity helpers](docs/transport.md).
The public device facade, managed operations and firmware adapters remain planned.
The development version is defined in library.json; it is not a release or
hardware-support claim.

`validateDriverConfig` checks callbacks, the explicit variant/address pair and
positive timeout. `validateProfile` checks value/variant semantics and distinguishes
invalid inputs from unsupported settings. Both are bounded, allocation-free and
perform zero callbacks or device mutation. TUSB322I write gates remain separate
from profile validity. See the [API guide](docs/api-contracts.md) and
[variant boundary](docs/variant-matrix.md).

## Consume the library

Use CMake 3.16+ and a C++17 compiler:

```cmake
add_subdirectory(path/to/TUSB32x)
target_link_libraries(app PRIVATE TUSB32x)
```

Tests default OFF. The target supplies its public include directory and C++17
requirement; ordinary consumers need no Git, Python, Doxygen, PlatformIO,
framework headers or sibling checkout. For example:

```cpp
#include <TUSB32x/Config.h>
#include <TUSB32x/Version.h>

const auto status = TUSB32x::validateProfile(
    TUSB32x::Variant::TUSB320LAI, TUSB32x::DeviceProfile{});
```

The [real host consumer](examples/host/basic/README.md) builds and runs multiple
translation units with CMake or pinned native PlatformIO. Its callbacks are
explicitly fake and never imply a physical device. If native IDF's
`idf_component_register` exists, the root CMake file registers the same real
sources as a component. Registration and dependency admission are not linked
firmware evidence.

## Develop from a source checkout

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
cmake -S . -B build/native -DTUSB32X_BUILD_TESTS=ON
cmake --build build/native --config Debug
ctest --test-dir build/native -C Debug --output-on-failure
```

Tests ON require Python 3.10+. For single-config generators also select
`-DCMAKE_BUILD_TYPE=Debug` at configure time. The registered checks cover C++
contracts, validators and isolated generator fixtures. Run `doxygen Doxyfile`
for strict current API documentation. On Windows use the existing VS Code Core
through `scripts/pio.cmd`; do not install another Core.

[Development metadata](docs/development.md) documents generator sync/check/bump/set,
exact exit codes, tool pins, package contents and evidence limits.
[Documentation](docs/README.md) and [examples](examples/README.md) index the
maintained guides. Repository implementation proceeds through the
[38 prompts](https://github.com/janhavelka/TUSB32x/tree/promptset/tusb32x-implementation-20261023/docs/prompts)
on one execution branch, with source/tests/docs, commits and verified sync for
each completed unit. Remote CI is confined to prompt 34 in this first execution.

Project code is MIT, owned by Thymos Solution s.r.o with Jan Havelka as maintainer.
Archived vendor references retain their original terms and are excluded from
normal library packages. Native tests prove software values and linkage;
Arduino/native-IDF firmware, full distribution qualification and physical
hardware evidence remain separate later work.
