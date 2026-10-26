# Portable values and validators consumer

This ordinary C++ application constructs a TUSB320LAI binding and a candidate
profile, then runs the two local validators. `main.cpp` and `profile.cpp` compile
and link against the public `TUSB32x` target. That target supplies C++17 and its
include directory; the consumer does not add either requirement itself.

The callbacks are explicitly fake and return `I2C_ERROR` if called. Validation
must leave their attempt counter at zero. There is no real device, initialization,
register access or profile application. A valid candidate does not establish
observed hardware settings or authorize an application power policy.

From the repository root, with CMake and a C++ compiler available:

```text
cmake -S examples/host/basic -B build/host-basic
cmake --build build/host-basic --config Release
```

With a single-config generator such as Ninja, also pass
`-DCMAKE_BUILD_TYPE=Release` in the configure command to select Release.

Run `build/host-basic/Release/tusb32x_host_basic.exe` with a Windows multi-config
generator, or `build/host-basic/tusb32x_host_basic` with a single-config generator
(append `.exe` on Windows). Expected output:

```text
Binding validation: OK; profile validation: OK; fake callback attempts: 0
```

Tests default OFF in the library; no test option is needed for this build. CMake
and the compiler are the only build dependencies. Python, Git, Doxygen, framework
SDKs and sibling projects are not required. This example proves portable public
linkage and local validation, not firmware integration or hardware qualification.
