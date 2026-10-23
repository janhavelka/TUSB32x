# TI source archive

Official reference documents retrieved on **2026-10-23** for TUSB32x library
planning. These files are research evidence, not library source or proof of
hardware behavior. Texas Instruments retains its original rights and notices;
the project's planned MIT license does not relicense these PDFs.

[source/provenance.json](source/provenance.json) records the official URL,
document/revision date, retrieval timestamp, byte count and SHA-256 of each exact
download. Keep an old revision's provenance if refreshing the archive. Exclude
PDFs/reference payloads from normal library packages and compilation.

| Local artifact | Official source | Use |
| --- | --- | --- |
| [TUSB320LAI/HAI SLLSEQ8D](source/TI-TUSB320LAI-HAI-SLLSEQ8D.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb320lai.pdf) | User's primary device, third-generation register and timing contract |
| [TUSB320LI/HI SLLSEP2D](source/TI-TUSB320LI-HI-SLLSEP2D.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb320li.pdf) | Second-generation capabilities and revision |
| [TUSB320/I SLLSEN9F](source/TI-TUSB320-I-SLLSEN9F.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb320.pdf) | Original addresses and restricted control map |
| [TUSB321 SLLSEO6C](source/TI-TUSB321-SLLSEO6C.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb321.pdf) | GPIO-only family boundary |
| [TUSB322I SLLSEO5D](source/TI-TUSB322I-SLLSEO5D.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb322i.pdf) | Separately gated I2C/VCONN extension |
| [Application issues SLAAE24](source/TI-TUSB32x-Application-Issues-SLAAE24.pdf) | [TI application note](https://www.ti.com/lit/an/slaae24/slaae24.pdf) | Family distinctions, startup/power and interrupt guidance |
| [Selection guide SDAA284](source/TI-CC-Selection-Guide-SDAA284.pdf) | [TI selection guide](https://www.ti.com/lit/pdf/sdaa284) | Current portfolio context and comparison |

Read the [technical audit](../audit/ti-family.md) for source locations, uncertainty
and conflicts. Exact device pin/electrical/register tables take precedence over
general portfolio tables; retain and explain conflicting wording. Recheck official
documentation/errata during implementation instead of assuming this snapshot is
permanently current.
