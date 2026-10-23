# Preparation validation

Date: **2026-10-23**. These results validate the audit/promptset artifacts only.
There is no driver implementation in this checkout to compile or hardware-test.

## Completed checks

| Check | Result | Scope |
| --- | --- | --- |
| Prompt sequence | PASS | Fifteen files, numbered 00 through 14, all listed in the promptset index |
| Shared contract | PASS | Every numbered prompt imports COMMON.md and requires renewed sibling auditing |
| Local Markdown links | PASS | All local Markdown targets resolve in this checkout |
| Text integrity | PASS | UTF-8 readable; no replacement characters; fenced blocks balanced |
| Vendor archive | PASS | Seven PDFs parse, 195 pages total; each byte count and SHA-256 matches provenance.json |
| Repository snapshots | PASS | Seventeen I2C libraries and FieldCore-node; sampled HEAD/dirty state unchanged at final comparison |
| Independent convention review | COMPLETE | Public layout, ownership, API/status, health, examples/CLI, tests, runners, tooling and package coverage |
| Independent consumer/tooling review | COMPLETE | FieldCore independence, generic consumers, callback budgets/timeouts, diagnostics and evidence verdicts |
| Independent TI review | COMPLETE | Family boundaries, addresses/identity, masks, W1C/reset, timing and documented ambiguities |

Mechanical checks used Python's pathlib/re/json/hashlib/subprocess, installed
PyMuPDF for PDF parsing, and read-only `git rev-parse` / `git status --porcelain`
against the sibling snapshots. No persistent test framework or driver test was
added for these documentation-only checks.

## Review corrections incorporated

- Defined a poll transfer budget as terminal callback attempts; a STOP-separated
  register access may contain two bus transactions. Adapter transaction counts
  and callback/managed-health counts are separate, with one aggregate timeout.
- Health is neutral diagnostics plus optional passive projection; an OFFLINE label
  cannot suppress an explicitly requested legal operation or schedule retries.
- Hardware waits start from completion/fresh time evidence; callbacks cannot
  consume a deadline unnoticed or shorten a required delay.
- Interrupt acknowledgement preserves observed/trusted applied RW settings,
  never unapplied staged settings. Explicit regression case required.
- Python serial runners specify DTR/RTS/open/reconnect behavior and invalidate
  evidence after a detected reboot; avoiding a reset command is not a guarantee.
- TUSB322I power effects are documented separately from application-owned external
  power policy; no false guarantee that role operations cannot affect VCONN.
- Repeated START and interrupt-clear timing are not invented TI requirements.
  Startup/current-tracking/access-wording conflicts remain visible.

## Not performed or claimed

- TUSB32x implementation, native tests, Arduino/native-IDF firmware builds,
  package compilation, CI or physical hardware tests.
- Sibling builds/tests, modifications, upstream synchronization, serial probing,
  flashing, publication, commits or pushes.

Those checks/actions belong to execution of the implementation promptset and its
actual environment/authorization. The local folder initially had no Git metadata;
the deliverable is the files in this checkout.
