#!/usr/bin/env python3
"""Exercise real static guards against isolated checkout mutations.

Run with Python 3.10+; no compiler, framework, network or hardware is used. Fixtures
copy the literal package inputs into temporary directories and never mutate the
checkout. Every CLI child has a finite timeout. Unittest returns nonzero if a
broken fixture passes, or a legal source/comment is rejected. Compiled/runtime
portability and allocation tests remain owned by the existing portable consumer.
"""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

import check_contracts as contracts


ROOT = Path(__file__).resolve().parents[1]


class ContractGuardTests(unittest.TestCase):
    """Require each boundary to reject a concrete malformed source or manifest."""

    def setUp(self):
        """Copy the current exported inputs into one disposable fixture."""
        self.temporary = tempfile.TemporaryDirectory(prefix="tusb32x-contract-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        manifest = json.loads((ROOT / "library.json").read_text(encoding="utf-8"))
        for name in manifest["export"]["include"]:
            target = self.root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, target)

    def append(self, name: str, text: str):
        """Append deliberate fixture code without editing the actual checkout."""
        with (self.root / name).open("a", encoding="utf-8", newline="\n") as stream:
            stream.write("\n" + text + "\n")

    def replace(self, name: str, before: str, after: str):
        """Require an existing fixture token before replacing its first occurrence."""
        path = self.root / name
        text = path.read_text(encoding="utf-8")
        self.assertIn(before, text)
        path.write_text(text.replace(before, after, 1), encoding="utf-8", newline="\n")

    def manifest(self, mutate):
        """Apply a metadata mutation to the fixture's manifest only."""
        path = self.root / "library.json"
        data = json.loads(path.read_text(encoding="utf-8"))
        mutate(data)
        path.write_text(json.dumps(data), encoding="utf-8", newline="\n")

    def reject(self, expected: str):
        """Check a specific diagnostic so an unrelated failure cannot pass a case."""
        errors = contracts.check(self.root)
        self.assertTrue(any(expected in error for error in errors), errors)

    def test_unmodified_fixture_passes(self):
        """Accept the maintained source and metadata contract before mutations."""
        self.assertEqual(contracts.check(self.root), [])

    def test_framework_header_rejected(self):
        """Reject a framework include added to ordinary core implementation."""
        self.append("src/TUSB32x.cpp", "#include <Arduino.h>")
        self.reject("unapproved or missing include")

    def test_framework_calls_rejected_in_each_portable_tree(self):
        """Enforce the same framework boundary in public, private and shared example code."""
        for name in ("include/TUSB32x/Config.h", "src/TUSB32x.cpp",
                     "examples/common/ExampleOwner.cpp"):
            with self.subTest(name=name):
                self.append(name, "void violation() { Wire.begin(); }")
                self.reject(f"framework API in portable source: {name}")

    def test_platform_clock_and_wait_rejected(self):
        """Detect platform clock access and blocking waits in separate fresh mutations."""
        for call in ("millis()", "delay(5)", "clock_gettime(0, nullptr)",
                     "std::chrono::steady_clock::now()", "std::this_thread::sleep_for(1)"):
            with self.subTest(call=call):
                original = self.root / "src/TUSB32x.cpp"
                before = original.read_bytes()
                self.append("src/TUSB32x.cpp", f"void violation() {{ {call}; }}")
                self.reject("platform time/wait")
                original.write_bytes(before)

    def test_heap_allocations_and_deallocation_rejected(self):
        """Detect allocation syntax, allocating containers and C allocation/deallocation calls."""
        for expression in ("new int", "new (place) int", "malloc(8)", "calloc(2, 8)",
                           "realloc(p, 8)", "free(p)", "delete p", "std::vector<int> data"):
            with self.subTest(expression=expression):
                original = self.root / "src/TUSB32x.cpp"
                before = original.read_bytes()
                self.append("src/TUSB32x.cpp", f"void violation() {{ {expression}; }}")
                self.reject("allocation")
                original.write_bytes(before)

    def test_exception_control_flow_rejected(self):
        """Reject exception-based control flow in core code."""
        self.append("src/TUSB32x.cpp", "void violation() { throw 1; }")
        self.reject("exception control flow")

    def test_core_output_rejected(self):
        """Reject core-owned output even when its message is an ordinary string literal."""
        self.append("src/TUSB32x.cpp", 'void violation() { printf("bad"); }')
        self.reject("logging/output in core")

    def test_comments_literals_and_deleted_copy_do_not_trigger(self):
        """Keep comments, raw strings and deleted ownership operations legal."""
        self.append("src/TUSB32x.cpp", '''
// malloc(4); Wire.begin(); delay(2); throw 1;
/* #include <Arduino.h>
   printf("bad"); */
constexpr char TEXT[] = "// not a comment: new int; delay(2);";
constexpr char RAW[] = R"sample("; malloc(4); Wire.begin(); //)sample";
struct Owner { Owner(const Owner&) = delete; };
''')
        self.assertEqual(contracts.check(self.root), [])

    def test_comment_markers_in_literals_cannot_hide_following_code(self):
        """Keep a quoted comment marker from hiding a later allocation on that line."""
        self.append("src/TUSB32x.cpp", 'const char* text = "//"; void bad() { malloc(1); }')
        self.reject("allocation")

    def test_digit_separators_cannot_hide_following_allocation(self):
        """Keep C++17 numeric apostrophes distinct from character literal quotes."""
        path = self.root / "src/TUSB32x.cpp"
        original = path.read_bytes()
        for number in ("1'000", "0xFF'FFu", "0b1010'0101ULL", "1'234.5e-6",
                       "0x1'F.Ap+2", ".1'25e+3", "1'000_count"):
            with self.subTest(number=number):
                path.write_bytes(original)
                text = (f"constexpr auto before = {number};\n"
                        "int* allocation() { return new int(1); }\n"
                        f"constexpr auto after = {number};")
                self.append("src/TUSB32x.cpp", text)
                self.reject("allocation")
                for keep_strings in (False, True):
                    self.assertEqual(contracts.masked(text, keep_strings), text)

    def test_numeric_literals_preserve_real_character_and_string_masking(self):
        """Numeric handling cannot expose forbidden words contained in literals."""
        text = ("auto number = 1'000; auto quote = '\\''; auto wide = L'x'; "
                'auto text = "1\'000 new int"; // 2\'000 malloc(4)\n'
                'auto raw = R"(3\'000 new int)";')
        code = contracts.masked(text)
        self.assertIn("1'000", code)
        self.assertNotIn("new", code)
        self.assertNotIn("malloc", code)
        self.assertEqual(len(code), len(text))

    def test_spliced_call_cannot_hide_allocation(self):
        """Apply C++ line splicing before checking forbidden function names."""
        self.append("src/TUSB32x.cpp", "void bad() { mal\\\nloc(1); }")
        self.reject("allocation")

    def test_commented_include_not_treated_as_dependency(self):
        """Ignore an include example inside a block comment."""
        self.append("src/TUSB32x.cpp", '/*\n#include "not-a-file.h"\n*/')
        self.assertEqual(contracts.check(self.root), [])

    def test_include_text_in_raw_string_is_not_a_dependency(self):
        """Ignore include-looking physical lines that are actually raw string contents."""
        self.append("src/TUSB32x.cpp", 'const char* raw = R"(\n#include <Arduino.h>\n)";')
        self.assertEqual(contracts.check(self.root), [])

    def test_unknown_system_and_missing_local_headers_rejected(self):
        """Require approved system headers and resolvable local header dependencies."""
        self.append("src/TUSB32x.cpp", '#include <thread>\n#include "missing.h"')
        errors = contracts.check(self.root)
        self.assertTrue(any("thread" in error for error in errors))
        self.assertTrue(any("missing.h" in error for error in errors))

    def test_include_hidden_behind_macro_rejected(self):
        """Reject nonliteral includes that would escape static dependency inspection."""
        self.append("src/TUSB32x.cpp", '#define HEADER "Arduino.h"\n#include HEADER')
        self.reject("include must use a literal header name")

    def test_nonstandard_include_directive_rejected(self):
        """Reject compiler-specific include extensions outside the supported C++17 subset."""
        self.append("src/TUSB32x.cpp", '#include_next <Arduino.h>')
        self.reject("include must use a literal header name")

    def test_public_header_cannot_depend_on_private_header(self):
        """Reject a relative path from public declarations into private implementation."""
        self.append("include/TUSB32x/Config.h", '#include "../../src/TUSB32xProtocol.h"')
        self.reject("unapproved or missing include")

    def test_core_cannot_depend_on_common_example(self):
        """Prevent example helpers from becoming a transitive core dependency."""
        self.append("src/TUSB32x.cpp", '#include "../examples/common/ExampleOwner.h"')
        self.reject("unapproved or missing include")

    def test_non_header_include_cannot_escape_source_scan(self):
        """Reject hidden implementation text stored under an unscanned include suffix."""
        (self.root / "src/hidden.inc").write_text("void bad() { millis(); }", encoding="utf-8")
        self.append("src/TUSB32x.cpp", '#include "hidden.inc"')
        self.reject("unapproved or missing include")

    def test_missing_include_guard_rejected(self):
        """Require repeated-inclusion protection on each scanned header."""
        self.replace("include/TUSB32x/Config.h", "#pragma once", "")
        self.reject("missing include guard")

    def test_ordinary_include_guard_accepted(self):
        """Accept a matching conventional guard as an alternative to pragma once."""
        self.replace("include/TUSB32x/Config.h", "#pragma once",
                     "#ifndef TUSB32X_CONFIG_H\n#define TUSB32X_CONFIG_H")
        self.append("include/TUSB32x/Config.h", "#endif")
        self.assertEqual(contracts.check(self.root), [])

    def test_public_namespace_pollution_rejected(self):
        """Keep public headers from importing a namespace into their consumers."""
        self.append("include/TUSB32x/Config.h", "using namespace std;")
        self.reject("using namespace in public header")

    def test_public_implementation_source_rejected(self):
        """Reject implementation translation units misplaced under the public include tree."""
        (self.root / "include/bad.cpp").write_text("int bad;", encoding="utf-8")
        self.reject("implementation source in public include tree")

    def test_metadata_name_license_and_ownership_rejected(self):
        """Detect changes to maintained package identity, license and ownership metadata."""
        self.manifest(lambda data: data.update(name="Wrong", license="unknown", authors=[]))
        self.reject("metadata name/license")
        self.reject("metadata authors/maintainer")

    def test_missing_public_header_metadata_rejected(self):
        """Require package header metadata to cover every real public header."""
        self.manifest(lambda data: data["headers"].pop())
        self.reject("headers must list every public header")

    def test_cmake_missing_and_duplicate_source_rejected(self):
        """Reject an explicit source list containing a duplicate instead of the missing source."""
        self.replace("CMakeLists.txt", "src/TUSB32x.cpp ", "src/TUSB32xProtocol.cpp ")
        self.reject("CMake TUSB32X_SOURCES")

    def test_new_unregistered_source_rejected(self):
        """Require each added core source in both build registrations and package exports."""
        (self.root / "src/extra.cpp").write_text("int extra;", encoding="utf-8")
        self.reject("CMake TUSB32X_SOURCES")
        self.reject("PlatformIO srcFilter")
        self.reject("package export omits core sources")

    def test_build_targets_must_consume_exact_shared_sources(self):
        """Reject framework sources added directly to the ordinary core library target."""
        self.replace("CMakeLists.txt", "STATIC ${TUSB32X_SOURCES}",
                     "STATIC ${TUSB32X_SOURCES} examples/arduino/Tusb32xWire.cpp")
        self.reject("CMake and IDF must consume only")

    def test_idf_cannot_substitute_different_sources(self):
        """Require native component registration to use the same explicit core sources."""
        self.replace("CMakeLists.txt", "SRCS ${TUSB32X_SOURCES}", "SRCS src/other.cpp")
        self.reject("CMake and IDF must consume only")

    def test_cmake_glob_and_append_rejected(self):
        """Reject dynamically expanded supplementary core source lists."""
        self.append("CMakeLists.txt", "file(GLOB OTHER src/*.cpp)\n"
                    "list(APPEND TUSB32X_SOURCES ${OTHER})")
        self.reject("must not use globbing or supplementary lists")

    def test_cmake_source_removal_rejected(self):
        """Detect later mutations that remove sources from the inspected shared variable."""
        self.append("CMakeLists.txt", "list(REMOVE_AT TUSB32X_SOURCES 0)")
        self.reject("one definition and two consumers")

    def test_platformio_wildcard_source_filter_rejected(self):
        """Keep PlatformIO source registration literal and complete."""
        self.manifest(lambda data: data["build"].update(srcFilter="+<*>"))
        self.reject("PlatformIO srcFilter")

    def test_platformio_source_tree_redirect_rejected(self):
        """Reject a manifest redirect from core sources into example code."""
        self.manifest(lambda data: data["build"].update(srcDir="examples"))
        self.reject("PlatformIO includeDir/srcDir")

    def test_export_parity_reuses_package_guard(self):
        """Exercise the existing package guard through mismatched IDF export metadata."""
        self.replace("idf_component.yml", '    - "LICENSE"', '    - "other.txt"')
        self.reject("IDF and PlatformIO content contracts differ")

    def test_malformed_metadata_fails_cleanly(self):
        """Return violations for invalid JSON and unsupported metadata shapes."""
        for text in ("{invalid", "[]", '{"headers": null}', '{"build": 7}'):
            with self.subTest(text=text):
                (self.root / "library.json").write_text(text, encoding="utf-8")
                self.assertTrue(contracts.check(self.root))

    def test_missing_source_tree_cannot_report_empty_success(self):
        """Treat a missing source directory as a failure rather than an empty successful scan."""
        # Rename within the owned fixture so cleanup remains bounded to its root.
        (self.root / "src").rename(self.root / "removed-src")
        self.reject("missing source directory: src")

    def test_cli_exit_codes_and_diagnostics(self):
        """Verify process exits for passing, violated, missing-root and invalid-option inputs."""
        command = [sys.executable, str(ROOT / "tools/check_contracts.py")]
        passed = subprocess.run(command + ["--root", str(self.root)], capture_output=True,
                                text=True, timeout=15, check=False)
        self.assertEqual(passed.returncode, 0, passed.stderr)
        self.assertIn("PASS:", passed.stdout)
        self.append("src/TUSB32x.cpp", "void bad() { delay(1); }")
        failed = subprocess.run(command + ["--root", str(self.root)], capture_output=True,
                                text=True, timeout=15, check=False)
        self.assertEqual(failed.returncode, 1, failed.stderr)
        self.assertIn("platform time/wait", failed.stderr)
        missing = subprocess.run(command + ["--root", str(self.root / "absent")],
                                 capture_output=True, text=True, timeout=15, check=False)
        self.assertEqual(missing.returncode, 2, missing.stderr)
        invalid = subprocess.run(command + ["--unknown"], capture_output=True,
                                 text=True, timeout=15, check=False)
        self.assertEqual(invalid.returncode, 2)

    def test_unreadable_inputs_report_incomplete(self):
        """Report a missing required metadata file as INCOMPLETE with exit code two."""
        (self.root / "library.json").unlink()
        output = io.StringIO()
        with redirect_stdout(output), redirect_stderr(output):
            self.assertEqual(contracts.main(["--root", str(self.root)]), 2)
        self.assertIn("INCOMPLETE:", output.getvalue())


if __name__ == "__main__":
    unittest.main()
