# Development metadata and version tools

`library.json` is version truth. The current `0.1.0-dev.1` marks an incomplete
library: portable values, validators, decoding, cooperative operations, the shared
CLI and Arduino S2/S3 examples are implemented. The native IDF adapter has
independent S2/S3 component and complete native CLI application links. Physical
qualification remains separate work. Version text
identifies the package, never the firmware flashed on a board.

## Version workflow

Use Python 3.10 or newer in a source checkout:

```text
python scripts/generate_version.py sync
python scripts/generate_version.py check
python scripts/generate_version.py bump patch
python scripts/generate_version.py set 0.2.0-dev.1
python tools/test_version_sync.py -v
```

Run bump/set only for an intentional version change. No release tag or registry
publication is implied. `sync` derives Version.h, CMake's full and numeric-core
versions, the component version and Doxygen PROJECT_NUMBER. Never hand-edit the
generated header. Equal bytes are left untouched, including mtimes. The script
uses its own location, not the working directory, Git or an enclosing repository.
Dirty examples do not affect the stable package header. Firmware build identity
is generated separately by `scripts/arduino_identity.py` for Arduino builds.
It records the actual checkout HEAD/dirty state, UTC build time and environment;
missing Git or an unrelated parent repository produces unknown provenance.

All commands require the exact arguments shown; `bump` also accepts minor/major.
Strict ASCII SemVer permits prerelease/build suffixes; numeric core and numeric
prerelease identifiers have no leading zeros. Core fields must fit uint32_t.
Bump increments the chosen core field, zeros lower fields and drops suffixes.
Set accepts a full version. No Git, clock, network or subprocess is consulted.

Exit 0 means success. `check` returns 1 and lists relative paths for drift without
writing. Invalid arguments, metadata, missing/ambiguous template fields and I/O
errors return 2. Alternate quoting, comments or command layout cannot hide a
second managed declaration. Missing version truth is an error, never an invented 0.0.0.
All inputs/templates are checked before writes; an I/O failure during writes can
leave a partial sync. Repair the failure and rerun the intended command.

## Builds and tool choices

Plain host builds need CMake 3.16+ and a C++17 toolchain. Tests default OFF and
consumers never discover Python, Git, Doxygen or PlatformIO. Tests ON find Python
3.10+ for registered tool/contract suites alongside the C++ suites. The Arduino
metadata fixture also uses Git in isolated temporary repositories; the mailbox
concurrency fixture links the host thread library. These are tests-ON dependencies.
Windows additionally registers tusb32x_pio_launcher; its temporary CMD fixtures
need no Core installation. Run them directly with `python tools/test_pio_launcher.py -v`.
See [native checks](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/README.md) in a source checkout.

If `idf_component_register` exists, CMake registers the same explicit sources and
public include/C++17 requirements as a native IDF component. Otherwise it creates
the ordinary host target TUSB32x. The manifest's `idf >=5.3.0` is a dependency
admission floor adopted from current TMP1x2/24Cxx and checked against the
[official IDF 5.3.2 C++ guide](https://github.com/espressif/esp-idf/blob/v5.3.2/docs/en/api-guides/cplusplus.rst)
and [component registration](https://github.com/espressif/esp-idf/blob/v5.3.2/tools/cmake/component.cmake).
The [native adapter fixtures](idf-transport.md) now link against the narrower
audited IDF 5.5.5 configuration on S2/S3; the admission floor does not qualify other
SDK versions. No Arduino, driver peripheral,
RTOS or consumer-product dependency is required by this component.

On Windows, use the existing VS Code-managed Core through `scripts/pio.cmd`:

```text
scripts\pio.cmd --version
scripts\pio.cmd run -d examples/host/basic -e native -t exec
scripts\pio.cmd pkg pack . --output build/TUSB32x.tar.gz
```

The launcher invokes `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`, passes
arguments and returns its exit code, including when the caller's environment
defines an ERRORLEVEL variable. A missing executable reports its exact path
and exits 1. It installs nothing. Managed Core 6.1.19 matches current peer workflow
pins. The real host consumer uses `platformio/native@1.2.1`, whose
[official manifest](https://github.com/platformio/platform-native/blob/v1.2.1/platform.json)
requires Core 6. Native uses the application's installed host toolchain, as the
[PlatformIO guide](https://docs.platformio.org/en/latest/platforms/native.html)
documents. The root firmware environments and their explicit board/USB flags are
described in [Arduino examples](arduino-examples.md); they do not affect the
ordinary CMake consumer.

Run `doxygen Doxyfile` for public documentation under build/doxygen. The current
configuration extracts documented public API and enables missing-comment,
parameter, enum and documentation errors as failures. Source/example comments
are included. The complete omission checker and its negative fixtures remain
prompt 33 work; a warning-free generation alone does not prove visibility.

## Package and ownership boundaries

The PlatformIO export list contains public headers, five real C++ sources and their
private protocol/codec/time headers, common CLI/owner/mailbox code, the Arduino
adapter and two applications, the native IDF adapter/application, CMake, manifests, license,
selected guides and the real host consumer. The root PlatformIO configuration and its two required
example build scripts and native incremental-identity script are shipped. Reference/vendor artifacts, prompts, audits,
progress/validation machinery, tests, other maintainer scripts and generated
build output are excluded from ordinary archives. Maintainer commands
require the source checkout. Links to repository-only evidence use source-host
URLs; links between shipped guides remain relative.
The IDF manifest excludes all files and explicitly re-includes the same consumer
set: IDF include rules are re-inclusions, as its
[manifest reference](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#files)
explains. Both manifests enumerate the same 76 literal filenames, including each
public header. The checker rejects any missing or extra member; it compares every
payload with this trusted checkout before executing build files.

### Archive and clean consumers

Run these maintainer commands from a source checkout (the checker is deliberately
excluded from the distribution):

```text
python tools/test_check_package.py -v
scripts\pio.cmd pkg pack . --output build/TUSB32x.tar.gz
python tools/check_package.py --archive build/TUSB32x.tar.gz --out build/package-check
```

The checker requires CMake, Ninja and a C++ compiler on the maintainer host; pass
`--cmake`, `--compiler`, `--make-program` or `--generator` for explicit tool paths.
It returns 0 after archive, link and executable checks pass, 1 on failed checks,
and 2 for invocation errors or missing build tools. Every child has a finite
60-second timeout. Its JSON report records the archive SHA-256, fresh extracted
path and counts. Evidence remains in a fresh private workspace; when `--out`
is inside the checkout, compilation uses the system temporary directory and
reports that external path. No unrelated tree is moved or deleted. These commands do not publish an archive or run hardware.

Before extraction, portable ASCII names (240 characters, 16 segments, 100 per
segment), case/prefix collisions, Windows aliases, duplicate headers and all
nonregular members are rejected. Limits are 512 files, 2 MiB per file, 16 MiB
payload, 32 MiB decompressed tar including extension headers and 8 MiB compressed.
The accepted format is gzip POSIX tar with regular files and optional per-file
mtime-only PAX metadata, matching the managed packer. Semantic path/size overrides,
global/GNU extensions and nonzero content after the terminator are rejected.
The complete scan uses a private immutable snapshot; no archive link or permission
is installed. Exact files and local Markdown links are checked in both trees.
This validates distribution mechanics, not authenticity or a sandbox for arbitrary
CMake: only packages made from this trusted checkout are built.

The copied standalone fixture uses `add_subdirectory()` and target `TUSB32x` in a
renamed extraction, directly includes every public header and runs simulated
cooperative initialization with exact tokens and effects. It checks source/include
origins, tests OFF, compile commands and a child PATH without Git, Python,
Doxygen or PlatformIO. The checker itself uses Python; consuming core does not.
There is no install/export or `find_package(TUSB32x)` contract.

CTest registers `tusb32x_package_safety` and `tusb32x_package_consumer`. The latter
creates a real test tar from the literal content list, then extracts, compiles,
links and executes; ordinary host tests do not require PlatformIO. Separately,
the actual managed-Core archive is checked with the command above. Extracted
Arduino builds use the four root environments. Native builds use the archived
`examples/esp_idf/basic` and separate absolute build/SDKCONFIG paths for S2/S3;
see its [commands](../examples/esp_idf/basic/README.md). Firmware identity remains
example-only: an extracted tree reports unknown Git/tree instead of borrowing
an enclosing repository. See repository-only
[validation evidence](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/docs/validation.md)
for actual hashes, tools, paths, commands and linked images. Firmware links do
not establish USB console input/output, I2C framing or electrical Type-C behavior.

| Checked package use | Tool/SDK row | Evidence boundary |
| --- | --- | --- |
| Plain add_subdirectory consumer | CMake 4.0.1, Ninja 1.12.1, GNU 15.1.0 | Renamed extraction compiles, links and runs without maintainer tools |
| Arduino basic and two-device examples, S2/S3 | Managed Core 6.1.19; pioarduino 55.03.311; Arduino 3.3.11; IDF 5.5.5 | Four extracted image links; hardware NOT RUN |
| Native CLI, S2/S3 | ESP-IDF 5.5.5; Xtensa GCC 14.2.0 esp-14.2.0_20260121 | Two genuine extracted image links; no Arduino; hardware NOT RUN |

Other SDK/toolchain rows await the full supported matrix at prompt 34; manifest
admission floors and framework names do not qualify them.

Thymos Solution s.r.o and maintainer Jan Havelka follow current sibling manifests
and MIT terms. CODEOWNERS uses the evidenced @janhavelka account from the current
repository origin and peer ownership file. Vendor artifacts retain their
separate original terms and are neither relicensed nor compiled/packaged as core.
