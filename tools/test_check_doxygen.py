#!/usr/bin/env python3
"""Prove documentation guard failures with bounded, real Doxygen fixtures.

Requires TUSB32X_DOXYGEN or Doxygen on PATH; a missing tool fails this suite
(no skipped success). The validator's explicit executable is honored in fixtures.
Each fixture owns a fresh temporary root. No repository output, tool installation,
network or hardware is touched. Mocked process failures exercise timeout/invocation
handling only; documentation failures execute the installed Doxygen parser.
"""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest import mock

import check_doxygen as guard


def _required_doxygen() -> str:
    """Resolve the validator's selected parser, failing rather than falling back."""
    selected = os.environ.get("TUSB32X_DOXYGEN", "doxygen")
    executable = shutil.which(selected)
    if executable is None:
        raise RuntimeError(f"Doxygen is required to run documentation guard fixtures: {selected}")
    return executable


class DoxygenContracts(unittest.TestCase):
    """Check actual parser diagnostics, hidden APIs and generated boundaries."""

    @classmethod
    def setUpClass(cls) -> None:
        """Require the real parser rather than silently skipping negative tests."""
        cls.doxygen = _required_doxygen()

    def setUp(self) -> None:
        """Create a complete tiny public API, generated header and source helper."""
        self.temporary = tempfile.TemporaryDirectory(prefix="tusb32x-doxygen-test-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        version = json.loads((guard.ROOT / "library.json").read_text())["version"]
        self.write("library.json", json.dumps({"version": version}) + "\n")
        self.write("include/TUSB32x/Version.h",
                   (guard.ROOT / "include/TUSB32x/Version.h").read_text())
        self.write("Doxyfile", '\n'.join(f"{key} = {value}" for key, value in
                   guard._REQUIRED_CONFIG.items()) + '\nINPUT = include src examples\n'
                   'FILE_PATTERNS = *.h *.cpp\nRECURSIVE = YES\n'
                   'PROJECT_NAME = Fixture\nGENERATE_LATEX = NO\nQUIET = YES\n')
        self.write("include/TUSB32x/Api.h", '''/// @file Api.h
/// @brief Tiny public fixture with a visible type and function.
#pragma once
/// @brief Public fixture namespace.
namespace TUSB32x {
/// @brief Public value with a documented field.
struct PublicValue {
  int value = 0; ///< Fixture value, initially zero.
};
/// @brief Valid states.
enum class State {
  READY, ///< Ready state.
  CLOSED ///< Closed state.
};
/// @brief Return an owner-supplied input unchanged.
/// @param[in] value Input value.
/// @return The supplied value.
int ping(int value);
}
''')
        self.write("src/Helper.cpp", '''/// @file Helper.cpp
/// @brief Implementation fixture; private helpers stay out of public HTML.
namespace {
// Clip the bounded sample at zero before adding its fixed offset.
int privateHelper(int value) {
  if (value < 0) value = 0;
  return value + 2;
}
}
''')
        (self.root / "examples").mkdir()

    def write(self, relative: str, content: str) -> None:
        """Write UTF-8 fixture content with stable LF endings."""
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8", newline="\n")

    def replace(self, relative: str, before: str, after: str) -> None:
        """Replace exactly one known fixture substring; fail on stale setup."""
        path = self.root / relative
        text = path.read_text(encoding="utf-8")
        self.assertEqual(text.count(before), 1)
        self.write(relative, text.replace(before, after))

    def run_guard(self, expected: int = 1, extra: list[str] | None = None) -> str:
        """Run the guard with a real parser and return captured diagnostic text."""
        output = io.StringIO()
        with redirect_stdout(output), redirect_stderr(output):
            result = guard.main(["--root", str(self.root), "--doxygen", self.doxygen,
                                 *(extra or [])])
        self.assertEqual(result, expected, output.getvalue())
        return output.getvalue()

    def private_core(self, hidden: bool) -> None:
        """Write a documented private type/helper, optionally outside public extraction."""
        self.write("src/Private.h", '''/// @file Private.h
/// @brief Private core fixture; file and helper comments are checked independently.
#pragma once
''' + ("/// @cond TUSB32X_INTERNAL\n" if hidden else "") + '''namespace TUSB32x {
namespace detail {
/// @brief Private implementation value, not a supported public API.
struct PrivateValue {
  int value = 0; ///< Scratch value, initially zero.
};
/// @brief Keep scratch nonnegative before the fixed adjustment.
/// @param[in] value Local scratch input.
/// @return Nonnegative scratch plus the fixed adjustment.
int privateAdjustment(int value) {
  if (value < 0) value = 0;
  return value + 2;
}
}
}
''' + ("/// @endcond\n" if hidden else ""))

    def test_published_private_core_api_fails(self) -> None:
        """C++ namespace privacy needs a check beyond EXTRACT_PRIVATE=NO."""
        self.private_core(hidden=False)
        self.assertIn("Private core symbol published: TUSB32x::detail::PrivateValue",
                      self.run_guard())

    def test_hidden_private_core_keeps_file_documentation(self) -> None:
        """Private declarations stay absent while their source file remains checked."""
        self.private_core(hidden=True)
        self.run_guard(0)
        evidence = json.loads((self.root / "build/doxygen-check/latest.json").read_text())
        index = (Path(evidence["output"]) / "xml/index.xml").read_text(encoding="utf-8")
        self.assertNotIn("PrivateValue", index)
        self.assertNotIn("TUSB32x::detail", index)
        self.assertIn("Private.h", index)

    def test_hidden_private_helper_still_requires_comment(self) -> None:
        """Excluding a private symbol cannot conceal its missing invariant comment."""
        self.private_core(hidden=True)
        self.replace("src/Private.h",
                     "/// @brief Keep scratch nonnegative before the fixed adjustment.\n"
                     "/// @param[in] value Local scratch input.\n"
                     "/// @return Nonnegative scratch plus the fixed adjustment.\n", "")
        self.assertIn("Missing source helper comment: src/Private.h", self.run_guard())

    def test_documented_fixture_passes_and_retains_actual_xml(self) -> None:
        """The positive control must generate visible XML/HTML, not just tokens."""
        self.run_guard(0)
        evidence = json.loads((self.root / "build/doxygen-check/latest.json").read_text())
        self.assertEqual(evidence["errors"], [])
        self.assertTrue((Path(evidence["output"]) / "xml/index.xml").is_file())
        self.assertTrue((Path(evidence["output"]) / "html/index.html").is_file())

    def test_missing_public_type_comment_fails_actual_doxygen(self) -> None:
        """An undocumented declaration must fail despite being present in XML."""
        self.replace("include/TUSB32x/Api.h",
                     "/// @brief Public value with a documented field.\n", "")
        self.assertIn("not documented", self.run_guard())

    def test_wrong_parameter_name_fails_actual_doxygen(self) -> None:
        """Doxygen verifies documentation names against the real signature."""
        self.replace("include/TUSB32x/Api.h", "@param[in] value", "@param[in] misspelled")
        self.assertIn("misspelled", self.run_guard())

    def test_missing_enum_value_comment_fails_actual_doxygen(self) -> None:
        """A documented enum cannot hide a missing value description."""
        self.replace("include/TUSB32x/Api.h", "CLOSED ///< Closed state.", "CLOSED")
        self.assertIn("CLOSED", self.run_guard())

    def test_broken_internal_reference_fails_actual_doxygen(self) -> None:
        """An unresolved explicit ref is a real parser warning."""
        self.replace("include/TUSB32x/Api.h", "Tiny public fixture",
                     r"See @ref MissingInternalType. Tiny public fixture")
        self.assertIn("MissingInternalType", self.run_guard())

    def test_hidden_public_type_fails_even_without_parser_warning(self) -> None:
        """A silent @cond block is still an omitted public API."""
        self.replace("include/TUSB32x/Api.h", "struct PublicValue {",
                     "/// @cond HIDDEN\nstruct PublicValue {")
        self.replace("include/TUSB32x/Api.h", "};\n/// @brief Valid states.",
                     "};\n/// @endcond\n/// @brief Valid states.")
        output = self.run_guard()
        self.assertIn("Public type absent from XML", output)
        self.assertIn("hiding directive", output)

    def test_excluded_public_function_fails(self) -> None:
        """Configuration cannot remove a callable public declaration."""
        with (self.root / "Doxyfile").open("a", encoding="utf-8") as stream:
            stream.write("EXCLUDE_SYMBOLS = TUSB32x::ping\n")
        self.assertIn("EXCLUDE_SYMBOLS", self.run_guard())

    def test_private_public_function_cannot_disappear(self) -> None:
        """Doxygen access commands cannot conceal a callable public function."""
        self.replace("include/TUSB32x/Api.h",
                     "/// @brief Return an owner-supplied input unchanged.",
                     "/// @private\n/// @brief Return an owner-supplied input unchanged.")
        self.assertIn("Public documentation hiding directive", self.run_guard())

    def test_protected_public_function_cannot_disappear(self) -> None:
        """Protected access commands are also incompatible with public headers."""
        self.replace("include/TUSB32x/Api.h",
                     "/// @brief Return an owner-supplied input unchanged.",
                     "/// @protected\n/// @brief Return an owner-supplied input unchanged.")
        self.assertIn("Public documentation hiding directive", self.run_guard())

    def test_alias_cannot_hide_public_function(self) -> None:
        """Alias expansion cannot change a public declaration's documentation access."""
        self.replace("include/TUSB32x/Api.h",
                     "/// @brief Return an owner-supplied input unchanged.",
                     "/// @conceal\n/// @brief Return an owner-supplied input unchanged.")
        configuration = (self.root / "Doxyfile").read_text(encoding="utf-8")
        for directive in ("private", "protected"):
            for assignment in ("=", "+="):
                with self.subTest(directive=directive, assignment=assignment):
                    self.write("Doxyfile", configuration
                               + f'ALIASES {assignment} "conceal=@{directive}"\n')
                    self.assertIn("through ALIASES", self.run_guard())

    def test_appended_excluded_function_cannot_disappear(self) -> None:
        """Doxyfile append assignments have the same exclusion effect as equals."""
        with (self.root / "Doxyfile").open("a", encoding="utf-8") as stream:
            stream.write("EXCLUDE_SYMBOLS += TUSB32x::ping\n")
        self.assertIn("EXCLUDE_SYMBOLS", self.run_guard())

    def test_predefined_macro_cannot_hide_public_function(self) -> None:
        """Conditional preprocessing cannot make a public function disappear."""
        self.replace("include/TUSB32x/Api.h", "int ping(int value);",
                     "#ifndef DOXYGEN\nint ping(int value);\n#endif")
        with (self.root / "Doxyfile").open("a", encoding="utf-8") as stream:
            stream.write("PREDEFINED = DOXYGEN\n")
        output = self.run_guard()
        self.assertIn("PREDEFINED", output)
        self.assertIn("Unsupported conditional public API documentation", output)

    def test_undefined_macro_cannot_hide_public_function(self) -> None:
        """An otherwise unconfigured conditional declaration is also unsupported."""
        self.replace("include/TUSB32x/Api.h", "int ping(int value);",
                     "#ifdef NEVER_DEFINED\nint ping(int value);\n#endif")
        self.assertIn("Unsupported conditional public API documentation", self.run_guard())

    def test_whole_file_include_guard_remains_supported(self) -> None:
        """Ordinary include guards remain valid portable public header practice."""
        self.replace("include/TUSB32x/Api.h", "#pragma once",
                     "#ifndef TUSB32X_FIXTURE_API_H\n#define TUSB32X_FIXTURE_API_H")
        with (self.root / "include/TUSB32x/Api.h").open("a", encoding="utf-8") as stream:
            stream.write("#endif\n")
        self.run_guard(0)

    def test_source_helper_comment_omission_fails(self) -> None:
        """A private nontrivial helper needs its short invariant comment."""
        self.replace("src/Helper.cpp",
                     "// Clip the bounded sample at zero before adding its fixed offset.\n", "")
        self.assertIn("Missing source helper comment", self.run_guard())

    def test_one_line_branching_helper_still_needs_comment(self) -> None:
        """Formatting a branch onto one line does not turn it into an accessor."""
        self.replace("src/Helper.cpp",
                     "// Clip the bounded sample at zero before adding its fixed offset.\n", "")
        self.replace("src/Helper.cpp", "int privateHelper(int value) {\n"
                     "  if (value < 0) value = 0;\n  return value + 2;\n}",
                     "int privateHelper(int value) { if (value < 0) value = 0; return value + 2; }")
        self.assertIn("Missing source helper comment", self.run_guard())

    def test_pointer_and_reference_helpers_need_comments(self) -> None:
        """Return pointer/reference placement cannot bypass the helper comment check."""
        source = (self.root / "src/Helper.cpp").read_text(encoding="utf-8")
        source = source.replace(
            "// Clip the bounded sample at zero before adding its fixed offset.\n", "")
        for signature, result in (("int *privateHelper(int value)", "nullptr"),
                                  ("int &privateHelper(int& value)", "value"),
                                  ("int*privateHelper(int value)", "nullptr"),
                                  ("int&privateHelper(int& value)", "value")):
            with self.subTest(signature=signature):
                self.write("src/Helper.cpp", source.replace(
                    "int privateHelper(int value)", signature).replace("value + 2", result))
                self.assertIn("Missing source helper comment", self.run_guard())

    def test_trailing_return_helper_needs_comment(self) -> None:
        """A trailing return type still defines a nontrivial private helper."""
        self.replace("src/Helper.cpp",
                     "// Clip the bounded sample at zero before adding its fixed offset.\n", "")
        self.replace("src/Helper.cpp", "int privateHelper(int value)",
                     "auto privateHelper(int value) -> int")
        self.assertIn("Missing source helper comment", self.run_guard())

    def test_documented_helper_declarator_variants_pass(self) -> None:
        """Supported return declarators remain legal with their adjacent comment."""
        source = (self.root / "src/Helper.cpp").read_text(encoding="utf-8")
        for signature, result in (("int *privateHelper(int value)", "nullptr"),
                                  ("int &privateHelper(int& value)", "value"),
                                  ("int*privateHelper(int value)", "nullptr"),
                                  ("int&privateHelper(int& value)", "value"),
                                  ("auto privateHelper(int value) -> int", "value + 2")):
            with self.subTest(signature=signature):
                self.write("src/Helper.cpp", source.replace(
                    "int privateHelper(int value)", signature).replace("value + 2", result))
                self.run_guard(0)

    def test_digit_separators_cannot_hide_undocumented_helper(self) -> None:
        """C++17 numeric apostrophes cannot mask the helper between two literals."""
        self.replace("src/Helper.cpp",
                     "// Clip the bounded sample at zero before adding its fixed offset.\n",
                     "constexpr int LOWER = 1'000;\n")
        self.replace("src/Helper.cpp", "  return value + 2;\n}\n}",
                     "  return value + 2;\n}\nconstexpr int UPPER = 2'000;\n}")
        self.assertIn("Missing source helper comment", self.run_guard())

    def test_documented_unrelated_name_cannot_hide_missing_helper_comment(self) -> None:
        """Exact XML file/line ownership prevents same-name documentation reuse."""
        self.replace("src/Helper.cpp",
                     "// Clip the bounded sample at zero before adding its fixed offset.\n", "")
        self.replace("src/Helper.cpp", "privateHelper", "ping")
        self.assertIn("Missing source helper comment", self.run_guard())

    def test_source_file_documentation_omission_fails(self) -> None:
        """Source documentation is checked even when anonymous helpers are hidden."""
        self.replace("src/Helper.cpp", "/// @file Helper.cpp\n", "")
        self.assertIn("Missing @file/@brief", self.run_guard())

    def test_generated_header_comments_omission_fails(self) -> None:
        """Generated constants are held to public documentation requirements."""
        self.replace("include/TUSB32x/Version.h",
                     "/// @brief Major SemVer component, in the uint32 range; "
                     "no I2C or allocation.\n", "")
        self.assertIn("VERSION_MAJOR", self.run_guard())

    def test_generated_header_exclusion_fails(self) -> None:
        """Generated files cannot disappear behind a successful Doxygen run."""
        with (self.root / "Doxyfile").open("a", encoding="utf-8") as stream:
            stream.write("EXCLUDE = include/TUSB32x/Version.h\n")
        self.assertIn("Documentation input missing: include/TUSB32x/Version.h", self.run_guard())

    def test_generated_marker_omission_fails(self) -> None:
        """Public generated comments must identify their actual producer."""
        self.replace("include/TUSB32x/Version.h",
                     "Generated by scripts/generate_version.py from library.json. "
                     "Do not edit manually.", "Hand maintained.")
        self.assertIn("producer marker missing", self.run_guard())

    def test_extract_all_cannot_suppress_missing_docs(self) -> None:
        """EXTRACT_ALL is explicitly rejected instead of making warnings vanish."""
        self.replace("Doxyfile", "EXTRACT_ALL = NO", "EXTRACT_ALL = YES")
        self.assertIn("EXTRACT_ALL = NO", self.run_guard())

    def test_stale_output_cannot_mask_new_omission(self) -> None:
        """Every generation gets its own fresh output directory."""
        self.run_guard(0)
        old = json.loads((self.root / "build/doxygen-check/latest.json").read_text())["output"]
        self.replace("Doxyfile", "INPUT = include src examples", "INPUT = src examples")
        self.assertIn("Documentation input missing", self.run_guard())
        new = json.loads((self.root / "build/doxygen-check/latest.json").read_text())["output"]
        self.assertNotEqual(old, new)
        self.assertTrue((Path(old) / "html/index.html").is_file())

    def test_explicit_environment_tool_works_without_path(self) -> None:
        """Fixture discovery and guard execution share the validator's executable."""
        output = io.StringIO()
        with mock.patch.dict(os.environ, {"TUSB32X_DOXYGEN": self.doxygen, "PATH": ""}):
            self.assertEqual(_required_doxygen(), self.doxygen)
            with redirect_stdout(output), redirect_stderr(output):
                result = guard.main(["--root", str(self.root)])
        self.assertEqual(result, 0, output.getvalue())

    def test_missing_environment_override_does_not_fall_back(self) -> None:
        """An invalid explicit parser remains incomplete even if PATH has Doxygen."""
        selected = str(self.root / "missing-selected-parser")
        with mock.patch.dict(os.environ, {"TUSB32X_DOXYGEN": selected}):
            with self.assertRaisesRegex(RuntimeError, "missing-selected-parser"):
                _required_doxygen()
            output = io.StringIO()
            with redirect_stdout(output), redirect_stderr(output):
                result = guard.main(["--root", str(self.root)])
        self.assertEqual(result, 2, output.getvalue())

    def test_missing_executable_is_incomplete(self) -> None:
        """An unavailable required tool must return nonzero, never SKIP/PASS."""
        self.assertIn("Doxygen required", self.run_guard(
            2, ["--doxygen", str(self.root / "absent-doxygen")]))

    def test_timeout_is_bounded_failure(self) -> None:
        """The subprocess receives a finite timeout and never uses a shell."""
        with mock.patch.object(guard.subprocess, "run", side_effect=
                               subprocess.TimeoutExpired(["doxygen"], 120)) as run:
            self.assertIn("exceeded 120 seconds", self.run_guard())
        self.assertEqual(run.call_args.kwargs["timeout"], 120)
        self.assertIsInstance(run.call_args.args[0], list)
        self.assertNotIn("shell", run.call_args.kwargs)
        evidence = json.loads((self.root / "build/doxygen-check/latest.json").read_text())
        self.assertTrue((Path(evidence["output"]) / "stdout.log").is_file())
        self.assertIn("exceeded 120", evidence["errors"][0])

    def test_unstartable_executable_is_incomplete(self) -> None:
        """OS invocation failures are distinguished from documentation errors."""
        with mock.patch.object(guard.subprocess, "run", side_effect=OSError("cannot execute")):
            self.assertIn("cannot execute", self.run_guard(2))

    def test_missing_local_html_anchor_fails(self) -> None:
        """Generated link checks verify real anchor existence."""
        html = self.root / "links"
        html.mkdir()
        (html / "index.html").write_text('<a href="guide.html#absent">Guide</a>', encoding="utf-8")
        (html / "guide.html").write_text('<a id="present"></a>', encoding="utf-8")
        self.assertIn("Missing local anchor: index.html: guide.html#absent",
                      guard.check_html(html)[1])


if __name__ == "__main__":
    unittest.main()
