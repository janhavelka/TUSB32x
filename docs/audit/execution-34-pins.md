# Prompt 34 pin and matrix audit

Checked official sources on 2026-10-28. This record verifies source identity and
availability; it does not report a firmware link, remote CI pass or hardware test.
The lead owns the selected workflow, SDK audit, builds and final result record.
The declared prompt 34 remote checkpoint is PENDING.

## Current checkout and peers

The audit began at TUSB32x `fc77aae9f81e908ffaa72f2f83781fcb9cba9750`, clean,
on `promptset/tusb32x-implementation-20261023`. Read parent/repository AGENTS,
COMMON, AGENT_WORKFLOW, prompt 34, current transport guides, platformio.ini and
idf_component.yml. Sibling reads were read-only; no fetch, branch change, build
or other Git mutation was performed by this reviewer.

| Peer | HEAD and dirty state | Exact sources and decision |
| --- | --- | --- |
| TMP1x2 | `bafc34067530eb340e55c42dae3fad4eb795b6e9`; clean | `AGENTS.md`, `.github/workflows/ci.yml`: adopt separate host/docs/Arduino/native jobs, finite timeouts, strict host warnings/sanitizers, S2/S3 images and read-only permissions. Reject copying its v5.3.2/v5.5.1/v6.0.1 rows as TUSB qualification; reject repeated source/package firmware gates and unverified historical action SHAs. |
| OPT4001 | `dab818e575fc25a0949d962bc0df600eb2ed5200`; clean | `AGENTS.md`, `.github/workflows/ci.yml`, `tools/check_ci_action_pins.py`: adopt full action SHAs and release comments. Its regex only verifies SHA shape; use actual YAML and the selected verified pin map for TUSB. Reject duplicating its guard/package commands under multiple jobs and using an ignored missing-log artifact as evidence that expected files exist. |

Both sibling workflows are comparison evidence, not proof that TUSB's adapter
works with their SDK versions. The manifest's `idf >=5.3.0` admits the portable
component; `docs/development.md` explicitly does not qualify the example adapter
on that floor. The actual adapter at audit start accepts only IDF 5.5.5.

## GitHub Actions

Each selected release was read from the official repository's `releases/latest`
endpoint, then resolved through `git/ref/tags/<release>` and an annotated tag's
object where applicable. Release pages and the pinned `action.yml` were inspected.

| Action | Verified release comment | Full commit |
| --- | --- | --- |
| [actions/checkout](https://github.com/actions/checkout/releases/tag/v7.0.1) | `# v7.0.1` | `3d3c42e5aac5ba805825da76410c181273ba90b1` |
| [actions/setup-python](https://github.com/actions/setup-python/releases/tag/v7.1.0) | `# v7.1.0` | `6d4fd22b20465ccc0f7354476588e8df7c9ca0e0` |
| [actions/upload-artifact](https://github.com/actions/upload-artifact/releases/tag/v7.0.2) | `# v7.0.2` | `cf430e030ddbb5b0abf93d22962f4752f3646cd9` |
| [actions/download-artifact](https://github.com/actions/download-artifact/releases/tag/v8.0.2) | `# v8.0.2` | `9000827ccba6bdab643e8b6fd33ac0654aef8333` |

All four run Node 24. Checkout/setup-python require runner 2.327.1 or later;
authenticated Git inside a Docker container action needs 2.329.0 or later.
Checkout v7 adds safer fork handling for privileged triggers. Setup-python v7.1
adds mirror inputs and dependency/security fixes. Both artifact releases improve
HTTP 429 retry handling and use the same artifact toolkit 6.3.1.

Use ordinary archived artifacts for named multi-file evidence. Upload v7's
`archive: false` accepts one file and uses its filename as artifact name, ignoring
`name`; avoid enabling that mode accidentally. Download v8 can consume v7 direct
uploads. Hidden files are excluded by default. Preserve the package tarball as a
file inside the artifact so upload's permission normalization does not change its
contents. These artifact majors target GitHub.com, not GHES.

## Host tools

| Tool | Pin/availability | Official source and qualification |
| --- | --- | --- |
| Python, Linux | 3.12.16 | [CPython release](https://www.python.org/downloads/release/python-31216/) is the 2026-10-25 security release. The [actions manifest](https://raw.githubusercontent.com/actions/python-versions/main/versions-manifest.json) includes Ubuntu 24.04 x64/arm64 under [3.12.16-40260402378](https://github.com/actions/python-versions/releases/tag/3.12.16-40260402378). It contains no Windows or macOS build for this source-only CPython release. |
| Python, Windows | No CI row selected | This checkpoint's host rows are Linux-only. The 3.12.16 manifest lacks Windows binaries; this does not add a Windows requirement or change supported local host builds. |
| PyYAML | 6.0.3 | [Official PyPI release](https://pypi.org/project/PyYAML/6.0.3/); requires Python >=3.8. |
| CMake Python distribution | 4.0.3 | [Official distribution release](https://pypi.org/project/cmake/4.0.3/); provides CMake 4.0.3, Python >=3.7, Linux/macOS/Windows wheels. |
| Ninja Python distribution | 1.13.0 | [Official distribution release](https://pypi.org/project/ninja/1.13.0/); Python >=3.8. Executable reports the distribution's `1.13.0.gd74ef.kitware.jobserver-pipe-1` suffix. PyPI version 1.13.1 does not exist (JSON endpoint returned 404). |
| GNU C++, Clang | GCC 14, Clang 18 on Ubuntu 24.04 | [Hosted image inventory](https://github.com/actions/runner-images/blob/main/images/ubuntu/Ubuntu2404-Readme.md), image 20261023.334.3, lists GCC 14.2.0 and Clang 18.1.3. Pin executable majors and record actual compiler/apt patch versions in runtime evidence; the hosted OS package set is mutable. |
| Doxygen, Ubuntu package | `1.9.8+ds-2ubuntu0.1` | [Official Ubuntu primary publication](https://api.launchpad.net/1.0/ubuntu/+archive/primary/+binarypub/245443974), Noble amd64 Updates, published 2026-06-15. The Security pocket has the same version. Exact apt installation can pin this version; record the installed version. |
| PlatformIO | 6.1.19 retained | [Official release history](https://docs.platformio.org/en/latest/core/history.html), 2026-02-04; [tag](https://github.com/platformio/platformio-core/releases/tag/v6.1.19) resolves to `15b8859aee4731fe4c5c6b06f74972535601eeb2`. Current latest is 6.2.0; retaining the established tested tool is deliberate, not a claim that 6.1.19 is latest. |

The declared workflow selects the official Doxygen 1.13.2 Linux binary:
[release](https://github.com/doxygen/doxygen/releases/tag/Release_1_13_2),
[asset](https://github.com/doxygen/doxygen/releases/download/Release_1_13_2/doxygen-1.13.2.linux.bin.tar.gz).
The reviewer streamed the official asset: 78,038,269 bytes, SHA-256
`f2c0a349403bc5b5ade3f501301e32b49ea31b3182666954bc398452fbc0dd1c`.
The release API supplies no digest; this hash was computed from the downloaded
bytes. Runtime compatibility of that binary was not tested by this reviewer.

## Arduino and native SDK sources

Keep the existing Arduino row pinned to [pioarduino 55.03.311](https://github.com/pioarduino/platform-espressif32/releases/tag/55.03.311),
commit `2a01b8848872983802e5f6f17d0b004df659b3c9`. Its official
[platform.json](https://raw.githubusercontent.com/pioarduino/platform-espressif32/55.03.311/platform.json)
directly names Espressif Arduino 3.3.11 core/libs assets and native IDF v5.5.5.
The manifest bytes have SHA-256
`cf752e458b9042e4b953f4dc7a0b6ddc1f9e87a02ff1f2ee67c4cfe5acc18f55`.
[Arduino 3.3.11](https://github.com/espressif/arduino-esp32/releases/tag/3.3.11)
resolves to `189089bb76e74978dc95abebefdad42b0d421ba9`; its release explicitly
uses IDF 5.5.5. Current latest pioarduino/Arduino are 55.03.312-1/3.3.12;
those are not substitutes for the existing audited Wire row.

The declared native matrix is 5.5.5 (oldest actually qualified adapter SDK) plus
6.1 (SDK version 6.1.0, current stable release), both on S2/S3. Qualification of
the new row requires the complete transport/lifetime audit, guards, host
regressions and real extracted images. Remote verification remains PENDING.
Do not call an added version guard or a sibling link support.
[Espressif release/version guidance](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32h2/versions.html)
requires checking framework compatibility and retesting migration changes.
The current stable release is [6.1](https://github.com/espressif/esp-idf/releases/tag/v6.1),
published 2026-08-27; 5.5.6 and 6.0.3 also exist. Historic 6.0.1 is not current.

| SDK | Official source commit | Docker Hub manifest index digest |
| --- | --- | --- |
| [v5.5.5](https://github.com/espressif/esp-idf/releases/tag/v5.5.5) | `b774170ff46c393eeb5e495ea37936038d3f4f4f` | `sha256:a9231d0697ab8f7517cc072e93b7c83e04907bfbfba80b6440d7dbbf90665cf2` |
| [v6.1](https://github.com/espressif/esp-idf/releases/tag/v6.1) | `fff9895c82d744c7237be8847347bdd1b07c6643` | `sha256:81893c71bb5e570088901f21def8684c25cd2a9020281bd01b843a7655edb18c` |

The official tag APIs were read directly:
[v5.5.5](https://hub.docker.com/v2/repositories/espressif/idf/tags/v5.5.5),
[v6.1](https://hub.docker.com/v2/repositories/espressif/idf/tags/v6.1).
The respective Linux amd64 child digests are
`sha256:6e2800a69f1c6521a5651da524f811e237d13e34cad369687916d0ad0bc4ef89`
and `sha256:f81b6e077e6af93c312181cd7d4e7936cfbafc5c7f77cbacba8fdd34994f218a`.
The index digests support both advertised Linux architectures; prefer the index
pin with an explicit runner architecture. These checks did not run either image.

The official `components/esp_driver_i2c/i2c_master.c` bytes were fetched afresh:

| Version/source | Bytes | SHA-256 |
| --- | --- | --- |
| [5.5.5 driver](https://github.com/espressif/esp-idf/blob/b774170ff46c393eeb5e495ea37936038d3f4f4f/components/esp_driver_i2c/i2c_master.c) | 69,303 | `884ee968512d8e375cd863641a325c4c8d98a21fa302b9cbcc5627f156dda790` |
| [6.1 driver](https://github.com/espressif/esp-idf/blob/fff9895c82d744c7237be8847347bdd1b07c6643/components/esp_driver_i2c/i2c_master.c) | 69,351 | `3ebaeee176e2ff731d15c69a7629c32c0339817afcd31e93da167f4f6b9d9ca0` |

The direct diff keeps the synchronous semaphore/command/event waits and retained
read handling, but changes non-DONE status from ESP_ERR_INVALID_STATE to
ESP_ERR_INVALID_RESPONSE. Preserve generic I2C_ERROR and full detail; the new
source comment does not prove a NACK phase. It changes clock/HAL macros, adds an
IRAM-safe receive-buffer check, and changes bus teardown/error paths. The complete
HAL/common-driver/interrupt and resource-lifetime comparison is recorded in
[the SDK audit](execution-34-sdk.md); real image links remain a separate gate.

## Declared checkpoint ownership

Five Linux host rows cover GCC 14/Clang 18 Debug/Release plus Clang 18 Debug with
ASan/UBSan. Python, documentation and package suites each have one Release owner.
The workflow checker needs PyYAML 6.0.3 and its negative fixtures run through the
existing `validate_runner`. The package job creates one identified archive;
four Arduino images and the four native SDK/target rows consume that archive,
without an identical source-checkout firmware matrix. The 76-file allowlist is
unchanged. Logs/JUnit/package/firmware evidence have seven-day retention. These
are declared gates; actual remote results and hardware qualification remain
separate evidence.
