# Development metadata and version tools

`library.json` is version truth. The current `0.1.0-dev.1` marks an incomplete
driver: values, Status helpers, local validators, public decoding and private
transport/identity/codecs/composition are implemented; device operations,
adapters and firmware qualification remain planned. Version text
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
belongs to later example tooling.

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
3.10+ for the registered tusb32x_version_sync suite alongside the three C++ suites.
Windows additionally registers tusb32x_pio_launcher; its temporary CMD fixtures
need no Core installation. Run them directly with `python tools/test_pio_launcher.py -v`.
See [native checks](https://github.com/janhavelka/TUSB32x/blob/promptset/tusb32x-implementation-20261023/test/README.md) in a source checkout.

If `idf_component_register` exists, CMake registers the same explicit sources and
public include/C++17 requirements as a native IDF component. Otherwise it creates
the ordinary host target TUSB32x. The manifest's `idf >=5.3.0` is a dependency
admission floor adopted from current TMP1x2/24Cxx and checked against the
[official IDF 5.3.2 C++ guide](https://github.com/espressif/esp-idf/blob/v5.3.2/docs/en/api-guides/cplusplus.rst)
and [component registration](https://github.com/espressif/esp-idf/blob/v5.3.2/tools/cmake/component.cmake).
It is not evidence of linked native IDF firmware. No Arduino, driver peripheral,
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
documents. No board pin, USB flag or future firmware environment is introduced.

Run `doxygen Doxyfile` for public documentation under build/doxygen. The current
configuration extracts documented public API and enables missing-comment,
parameter, enum and documentation errors as failures. Source/example comments
are included. The complete omission checker and its negative fixtures remain
prompt 33 work; a warning-free generation alone does not prove visibility.

## Package and ownership boundaries

The PlatformIO export list contains public headers, five real C++ sources and their
private protocol/codec/time headers, CMake,
manifests, license, selected guides and the real host consumer. Reference/vendor
artifacts, prompts, audits, progress/validation machinery, tests, scripts and
generated build output are excluded from ordinary archives. Maintainer commands
require the source checkout. Links to repository-only evidence use source-host
URLs; links between shipped guides remain relative.
The IDF manifest excludes all files and explicitly re-includes the same consumer
set: IDF include rules are re-inclusions, as its
[manifest reference](https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#files)
explains. Archive selection checks at this stage are separate from prompt 32's
full package/firmware consumer qualification.

Thymos Solution s.r.o and maintainer Jan Havelka follow current sibling manifests
and MIT terms. CODEOWNERS uses the evidenced @janhavelka account from the current
repository origin and peer ownership file. Vendor artifacts retain their
separate original terms and are neither relicensed nor compiled/packaged as core.
