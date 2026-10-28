# Simulated ordinary C++ owner

This runnable C++17 application uses public callbacks and the `TUSB32x` CMake
target. It has an explicitly **SIMULATED** TUSB320LAI register image and synthetic
monotonic milliseconds. It requires no physical chip, Arduino, ESP-IDF or sibling
project. `main.cpp` owns the driver and transport; `profile.cpp` constructs a
candidate in a separate translation unit with its own direct public include.
The target supplies the C++17 requirement and public include path.

The application binds without traffic, admits read-only initialization, and polls
with a ceiling of one terminal callback per turn. `IN_PROGRESS` means admission
succeeded. It takes the exact instance-local token and checks the delivered
`OperationResult.status` and `hardwareEffect` separately from delivery status.
It then explicitly applies the candidate's Try.SNK preference, obtains a read-only
port snapshot, and copies the cache by value. A detached port is a valid observation.
Advertisement categories are not measured load or authorization to supply power.

Next, an application-simulated external reset changes the local image and calls
`invalidateDeviceState`. A successful unrelated raw identity read leaves
configuration UNKNOWN and the historical port invalidated. The previous caller
copy stays unchanged. The application independently chooses `startReconcile`,
which proves the different actual profile without replaying desired settings.
Its result delivery returns OK while its payload reports CONFIGURATION_MISMATCH
and effect NONE. A separate complete refresh renews port evidence. Finally,
`end()` unbinds without traffic while the borrowed context remains alive.

From the repository root, with CMake and a C++17 compiler available:

```text
cmake -S examples/host/basic -B build/host-basic -DTUSB32X_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build/host-basic --config Release
```

With a single-config generator such as Ninja, run
`build/host-basic/tusb32x_host_basic` (append `.exe` on Windows). With a Windows
multi-config generator, run `build/host-basic/Release/tusb32x_host_basic.exe`.
For Debug, use a separate build directory, set `CMAKE_BUILD_TYPE=Debug`, and pass
`--config Debug`. Always-active checks return exit code 1 on any failed contract;
Release does not disable them. Successful output is:

```text
SIMULATED TUSB320LAI owner; synthetic milliseconds; package 0.1.0-dev.1
Bind: OK detail=0 msg=OK
Admission: IN_PROGRESS detail=0 msg=IN_PROGRESS
Poll outcome: OK detail=0 msg=OK
Result delivery: OK detail=0 msg=OK
Operation payload: OK detail=0 msg=OK
Effect=NONE; callbacks=6
Admission: IN_PROGRESS detail=0 msg=IN_PROGRESS
Poll outcome: OK detail=0 msg=OK
Result delivery: OK detail=0 msg=OK
Operation payload: OK detail=0 msg=OK
Effect=CONFIRMED; callbacks=7
Admission: IN_PROGRESS detail=0 msg=IN_PROGRESS
Poll outcome: OK detail=0 msg=OK
Result delivery: OK detail=0 msg=OK
Operation payload: OK detail=0 msg=OK
Effect=NONE; callbacks=3
Cached SIMULATED port: detached; sequence=1; acquired=132 ms
External reset invalidation: OK detail=0 msg=OK
Unrelated raw identity read: OK detail=0 msg=OK
After raw success: configuration=UNKNOWN; cached port invalidated
Admission: IN_PROGRESS detail=0 msg=IN_PROGRESS
Poll outcome: CONFIGURATION_MISMATCH detail=0 msg=CONFIGURATION_MISMATCH
Result delivery: OK detail=0 msg=OK
Operation payload: CONFIGURATION_MISMATCH detail=0 msg=CONFIGURATION_MISMATCH
Effect=NONE; callbacks=6
Admission: IN_PROGRESS detail=0 msg=IN_PROGRESS
Poll outcome: OK detail=0 msg=OK
Result delivery: OK detail=0 msg=OK
Operation payload: OK detail=0 msg=OK
Effect=NONE; callbacks=3
End: OK detail=0 msg=OK
SIMULATED owner checks passed
```

The package version comes from generated Version.h; it identifies no flashed
firmware. The example prints the full backend `Status.detail` and static message
for every reported outcome. For the managed Windows PlatformIO native environment:

```text
scripts\pio.cmd run -d examples/host/basic -e native -t exec
```

This pins `platformio/native@1.2.1` and uses the existing Core/host compiler. No
version generator runs during either consumer build. See
[development tools](../../../docs/development.md).

Each operation has an immutable 100 synthetic millisecond deadline and a maximum
of 64 owner turns. Each callback completes within one synthetic millisecond,
including immediate gate admission and all simulated phases; it handles at most
a two-byte write or a one-byte pointer and eight receive bytes. Unsupported
frames fail with a terminal I2C_* status. Buffers are borrowed only until return;
there is no retained buffer, asynchronous work, retry or bus recovery. The gate
is released before every callback returns. The gate only detects accidental
overlap in this single-threaded example; it is not a mutex usable by threads.

A production owner must serialize its own instance access and the physical bus
across every bus user, including the STOP between pointer selection and read.
Keep one configuration owner per physical target: a bus lock alone cannot
protect another owner's cached settings. Do not hold a bus lock between polls.
An ISR only signals the owner; it performs no driver calls, bus work or logging.
Production turns return to unrelated work and sample fresh monotonic uint64
milliseconds after callback return. Status-only callbacks report no completion
time; the next fresh owner sample is essential. A real backend must prove its
aggregate timeout, including lock acquisition, both phases, SDK internal waits
and cleanup. This in-memory example establishes no such physical bound.

The context is constructed before the driver, remains alive through explicit
`end()`, and outlives the driver on failure exits too. Bind, admission, local
invalidation, cached getters, result take and end are bus-silent. End does not
reset the device or undo effects. Cancellation and failed writes also require
independent effect inspection and an application-selected recovery policy.
For original TUSB320/I source-capable startup after VDD return, the application
establishes stable supply and I2C readiness, obtains identity, and explicitly
chooses `startRecoverDeadBattery` when required. Keep that register procedure
inside the core; the LAI example does not request a base-only recovery.

Tests default OFF. Explicit tests-ON builds reuse the registered host executable
and expose the same tests at this project root rather than creating a duplicate target.
CMake and the compiler are the only tests-OFF consumer build dependencies;
Git, Python, Doxygen and framework SDKs are unnecessary. This demonstrates portable
public API ownership, linkage and simulated state/effect behavior. Physical
Type-C, timing, IRQ, VBUS and board qualification require separate evidence. See
[owner integration](../../../docs/integration.md) for the generic mapping and
registered two-device owner fixture.
