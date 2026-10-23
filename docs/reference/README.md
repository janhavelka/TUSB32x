# Reference sources

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
| [TUSB321AI SLLSER2A](source/TI-TUSB321AI-SLLSER2A.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb321ai.pdf) | Exact source for the separate GPIO-only part called TUSB321A in family summaries |
| [TUSB322I SLLSEO5D](source/TI-TUSB322I-SLLSEO5D.pdf) | [TI datasheet](https://www.ti.com/lit/ds/symlink/tusb322i.pdf) | Separately gated I2C/VCONN extension |
| [Application issues SLAAE24](source/TI-TUSB32x-Application-Issues-SLAAE24.pdf) | [TI application note](https://www.ti.com/lit/an/slaae24/slaae24.pdf) | Family distinctions, startup/power and interrupt guidance |
| [Selection guide SDAA284](source/TI-CC-Selection-Guide-SDAA284.pdf) | [TI selection guide](https://www.ti.com/lit/pdf/sdaa284) | Current portfolio context and comparison |

Read the [technical audit](../audit/ti-family.md) for source locations, uncertainty
and conflicts. Exact device pin/electrical/register tables take precedence over
general portfolio tables; retain and explain conflicting wording. Recheck official
documentation/errata during implementation instead of assuming this snapshot is
permanently current.

## Prompt 2 current-source review

Checked 2026-10-23. All seven existing PDFs matched their manifest byte counts
and SHA-256, and fresh downloads from their official URLs were byte-identical.
Added the separate TUSB321AI SLLSER2A May 2017 datasheet (855642 bytes), without
replacing any prior revision. All eight entries now retain a per-file rights/
terms description and source-verification UTC timestamp. The original PDF
notices remain intact; the concise manifest description does not replace them.
No reference is an input to production compilation. Ordinary packages must
exclude the payload; executable package enforcement is due in prompts 8/32.

| Official product/documentation page checked | Current linked source or material observation |
| --- | --- |
| [TUSB320](https://www.ti.com/product/TUSB320) | SLLSEN9F, March 2022; shared exact datasheet also covers TUSB320I. Separate TUSB320I product-page retrieval failed in this session; the shared PDF was successfully verified |
| [TUSB320LI](https://www.ti.com/product/TUSB320LI), [TUSB320HI](https://www.ti.com/product/TUSB320HI) | SLLSEP2D, May 2017; both pages discourage LI/HI DFP/DRP configurations in their alternate-product description |
| [TUSB320LAI](https://www.ti.com/product/TUSB320LAI), [TUSB320HAI](https://www.ti.com/product/TUSB320HAI) | SLLSEQ8D, May 2017; exact part polarity differs |
| [TUSB322I](https://www.ti.com/product/TUSB322I) | SLLSEO5D, March 2021 in the PDF. Catalog listing date June 2017 is not the PDF revision date |
| [TUSB321](https://www.ti.com/product/TUSB321), [TUSB321AI](https://www.ti.com/product/TUSB321AI) | SLLSEO6C, August 2018 and SLLSER2A, May 2017; separate GPIO-only documents |
| [Application issues](https://www.ti.com/lit/an/slaae24/slaae24.pdf), [selection guide](https://www.ti.com/lit/pdf/sdaa284) | SLAAE24 May 2021 and SDAA284 March 2026; summary-table conflicts remain visible in the variant guide |

The inspected technical-document lists and targeted TI errata searches exposed
no separate relevant silicon errata. This is not an exhaustive absence claim.
Product pages, summaries and support replies supplement exact datasheets; their
dates do not prove a newer register contract. Recheck them before dependent work.
The [variant matrix](../variant-matrix.md) records exact facts and unresolved
claims; [feature coverage](../feature-coverage.md) maps them to later checks.

## Public implementation provenance

[public-implementations.json](public-implementations.json) pins 13 source files
from six repositories: eight TI Linux driver/helper/binding/board files and five
community driver/emulator/patch files. Each entry records repository owner, file
attribution and license evidence, full commit/path/permalink, Git blob SHA,
completed retrieval UTC, byte size and independently calculated SHA-256.

Actual source files were inspected through the GitHub connector, then raw bytes
were downloaded outside the repository and checked against the recorded blob
IDs. The manifest archives metadata only; it does not vendor, execute, relicense
or physically qualify these implementations. Supplemental source and license
links remain in the audit reports; they are not all entries in this manifest.

Read the [TI-hosted source audit](../audit/ti-public-implementations.md),
[community audit](../audit/community-public-implementations.md) and
[additional TI evidence](../audit/additional-ti-evidence.md) for observations,
confirmed code defects, document conflicts and search limitations. The
[shared chip evidence](../prompts/CHIP_EVIDENCE.md) makes these findings usable
by the numbered prompts while requiring exact-source verification and correction.
