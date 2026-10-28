# Native ESP-IDF component link fixture

This independent project links the repository's native core component and the
actual example adapter. Its `app_main` creates no bus or device; default adapter
contexts deliberately reject locally. Core binding is gated on successful
adapter validation, and `end()` runs before the context expires. Every adapter
entry point is linked. No CLI or Arduino source
is compiled. The fixture does not flash hardware or qualify electrical behavior,
transaction timing or application scheduling overhead.

From the repository root, use the installed VS Code-managed PlatformIO Core:

```powershell
scripts\pio.cmd run -d test/idf_sdk -e idf_s2 -e idf_s3
```

The platform is pinned to pioarduino `55.03.311`, whose native framework package
is ESP-IDF `5.5.5` (`framework-espidf` `3.50505`). Each environment has a separate
`.pio/build/idf_s2` or `.pio/build/idf_s3` directory and generated `sdkconfig.idf_s2`
or `sdkconfig.idf_s3`. Common and target-specific defaults are tracked; generated
SDK configuration, firmware, component-manager lock and build files are ignored.
Successful output includes each environment's `firmware.elf` and `firmware.bin`.
The root component path and name are derived from the checkout rather than an
absolute workstation path or a fixed checkout name.

In an activated ESP-IDF 5.5.5 shell, an equivalent native build uses separate
target directories and configurations:

```sh
cd test/idf_sdk
idf.py -B build-s2 -DIDF_TARGET=esp32s2 -DSDKCONFIG=sdkconfig.esp32s2 build
idf.py -B build-s3 -DIDF_TARGET=esp32s3 -DSDKCONFIG=sdkconfig.esp32s3 build
```

PlatformIO's commands above are the Windows validation path. An SDK-header shim
or a registration-only CMake check is not a substitute for these firmware links.
The tracked defaults disable SDK application logging, dynamic power management
and I2C debug logging as required by the adapter's bounded SDK contract.
The native adapter uses the SDK's standard warning policy. Host recording-backend
tests compile the actual adapter with the repository's strict warning flags; the
root core component also retains its strict flags. Applying those flags to the
native `main` target makes the pinned PlatformIO builder propagate them into
unrelated SDK components, whose libc wrappers and NVS code fail those diagnostics.
This fixture therefore retains the SDK policy for its main component.
Read the [adapter's integration guide](../../docs/idf-transport.md) before
constructing an application context; the link fixture's invalid contexts are
intentionally unusable for transfers.
