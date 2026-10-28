#!/usr/bin/env python3
"""Exercise archive rejection before extraction with bounded synthetic fixtures.

Run ordinary unit tests with Python 3.10+; no build tool or hardware is used.
The explicit --consumer-origins mode runs three bounded builds with supplied host
tools, retains temporary evidence, and checks actual header provenance. No mode
uses a framework, network or hardware. Fixtures live in fresh temporary roots.
"""

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
import argparse
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import tarfile
import tempfile
import shutil
import subprocess
import sys
import unittest
from unittest import mock

import check_package as package


class ArchiveContracts(unittest.TestCase):
    """Check portable names, bounded archive parsing, and isolated extraction."""

    def setUp(self) -> None:
        """Create isolated input/output locations and an external sentinel."""
        self.temporary = tempfile.TemporaryDirectory(prefix="tusb32x-package-test-")
        self.root = Path(self.temporary.name).resolve()
        self.work = self.root / "work"
        self.work.mkdir()
        self.sentinel = self.root / "outside.txt"
        self.sentinel.write_bytes(b"outside file must survive\x00\xff")
        self.sequence = 0

    def tearDown(self) -> None:
        """Remove only this test's owned temporary directory."""
        self.temporary.cleanup()

    def test_extract_only_needs_no_compiler_and_claims_no_linkage(self) -> None:
        """CI reuses one archive safely without repeating its clean host consumer."""
        output = self.work / "evidence"
        with mock.patch.object(package, "consume") as consume, \
                mock.patch.object(package.shutil, "which", return_value=None), \
                redirect_stdout(io.StringIO()):
            result = package.main(["--test-archive", "--extract-only", "--out", str(output)])
        self.assertEqual(result, 0)
        consume.assert_not_called()
        report = json.loads((output / "package-report.json").read_text())
        self.assertEqual(report["consumer"], "NOT RUN")
        self.assertEqual(report["scope"], "extraction")
        checkout = Path(__file__).resolve().parents[1]
        self.assertEqual(report["files"], len(package.package_files(checkout)))

    def archive(self, entries: list[tuple[tarfile.TarInfo, bytes]]) -> Path:
        """Write a small real gzip tar, including deliberately hostile headers."""
        self.sequence += 1
        path = self.root / f"fixture-{self.sequence}.tar.gz"
        with tarfile.open(path, "w:gz", format=tarfile.PAX_FORMAT) as stream:
            for member, payload in entries:
                stream.addfile(member, io.BytesIO(payload) if member.isfile() else None)
        return path

    @staticmethod
    def regular(name: str, payload: bytes = b"content") -> tuple[tarfile.TarInfo, bytes]:
        """Construct a normal file without trusting a filesystem path."""
        member = tarfile.TarInfo(name)
        member.size = len(payload)
        return member, payload

    def reject(self, archive: Path, required: set[str], **bounds: int) -> None:
        """Require a deliberate error, intact caller files, and no extracted tree."""
        before = self.sentinel.read_bytes()
        with self.assertRaises(package.PackageError):
            package.extract_archive(
                archive, self.work, required, package.ArchiveLimits(**bounds)
            )
        self.assertEqual(self.sentinel.read_bytes(), before)
        self.assertFalse(list(self.work.rglob("renamed-tusb32x")))
        # Validation must finish before writing any archive payload, including
        # an innocent member that precedes the malformed one.
        self.assertFalse(list(self.work.rglob("a.txt")))

    def test_portable_names_preserve_spelling(self) -> None:
        """Accept actual project path forms without normalization."""
        for name in (
            "README.md", "include/TUSB32x/Version.h", "scripts/pio.cmd",
            "examples/01_basic_bringup_cli/main.cpp", "sdkconfig.defaults.esp32s3",
        ):
            with self.subTest(name=name):
                self.assertEqual(package.validate_name(name), tuple(name.split("/")))

    def test_nonportable_names_are_rejected_directly(self) -> None:
        """Reject path escapes, Windows aliases, and oversized path components."""
        # Direct validation ensures an exact-member allowlist cannot conceal a
        # missing path defense by independently rejecting an unexpected file.
        names = (
            "", ".", "..", "../outside.txt", "a/../../outside.txt", "/outside.txt",
            "//server/share/file", "C:/outside.txt", "C:outside.txt", "c:/x",
            "a\\b.txt", "\\\\server\\share\\file", "\\?\\C:\\file", "a:b",
            "a/b:stream", "a::$DATA", "a//b", "a/./b", "./a.txt", "a/../b",
            "a/", "a.", "a ", "dir./a", "dir /a", " file", "a\tfile",
            "a\nfile", "a\x00file", "a\x1ffile", "a\x7ffile", "a?b", "a*b",
            "a|b", 'a"b', "a<b", "a>b", "caf\u00e9.txt", "COM\u00b9.txt",
            "NUL", "nul.txt", "CON", "con.tar.gz", "PRN", "AUX.txt",
            "com1", "COM9.txt", "lpt1", "LPT9.TXT", "dir/CON/file",
            "CONIN$", "CONOUT$", "/".join(["a"] * 1000), "a" * 1000,
        )
        for name in names:
            with self.subTest(name=repr(name)):
                with self.assertRaises(package.PackageError):
                    package.validate_name(name)

    def test_valid_pax_archive_extracts_under_new_renamed_directory(self) -> None:
        """Preserve ordinary payloads and accept PlatformIO's fractional mtime."""
        member, payload = self.regular("include/TUSB32x/Version.h", b"header\x00\xff")
        member.pax_headers = {"mtime": "1793218195.5621753"}
        source = self.archive([self.regular("a.txt"), (member, payload)])
        library = package.extract_archive(
            source, self.work, {"a.txt", "include/TUSB32x/Version.h"}
        )
        self.assertEqual(library.name, "renamed-tusb32x")
        self.assertTrue(library.resolve().is_relative_to(self.work.resolve()))
        self.assertNotEqual(library.parent, self.work)
        self.assertEqual((library / "a.txt").read_bytes(), b"content")
        self.assertEqual((library / member.name).read_bytes(), payload)
        self.assertFalse((self.work / "a.txt").exists())

    def test_repeated_extraction_uses_fresh_roots(self) -> None:
        """A second invocation cannot overwrite an earlier extracted package."""
        source = self.archive([self.regular("a.txt")])
        first = package.extract_archive(source, self.work, {"a.txt"})
        (first / "a.txt").write_bytes(b"first owner content")
        second = package.extract_archive(source, self.work, {"a.txt"})
        self.assertNotEqual(first, second)
        self.assertEqual((first / "a.txt").read_bytes(), b"first owner content")
        self.assertEqual((second / "a.txt").read_bytes(), b"content")

    def test_exact_contents_are_required(self) -> None:
        """Reject omissions, additions, empty archives, and changed spelling."""
        cases = (
            ([self.regular("a.txt")], {"a.txt", "missing.h"}),
            ([self.regular("a.txt"), self.regular("unlisted.txt")], {"a.txt"}),
            ([], {"a.txt"}),
            ([self.regular("a.txt")], {"A.txt"}),
        )
        for entries, required in cases:
            with self.subTest(required=required, entries=[entry[0].name for entry in entries]):
                self.reject(self.archive(entries), required)

    def test_shared_example_file_omission_fails(self) -> None:
        """The external content contract includes required shared example inputs."""
        required = {"a.txt", "examples/common/ConsoleBuffer.h"}
        self.reject(self.archive([self.regular("a.txt")]), required)

    def test_unlisted_artifacts_fail(self) -> None:
        """An archive cannot add build, private, vendor, or local configuration files."""
        for name in (
            "build/output.bin", ".git/config", "docs/reference/chip.pdf",
            "examples/.pio/build/output.elf", "sdkconfig", "dependencies.lock",
            "__pycache__/module.pyc", "private-evidence/trace.log",
        ):
            with self.subTest(name=name):
                self.reject(self.archive([self.regular("a.txt"), self.regular(name)]), {"a.txt"})

    def test_traversal_is_rejected_before_first_file_write(self) -> None:
        """A late unsafe member prevents extraction of preceding innocent files."""
        for name in ("../outside.txt", "../../outside.txt", self.sentinel.as_posix()):
            with self.subTest(name=name):
                self.reject(self.archive([self.regular("a.txt"), self.regular(name)]), {"a.txt"})

    def test_duplicate_file_headers_fail(self) -> None:
        """Repeated names cannot silently replace an earlier payload."""
        source = self.archive([self.regular("a.txt"), self.regular("a.txt", b"replace")])
        self.reject(source, {"a.txt"})

    def test_case_collisions_include_implicit_parent_directories(self) -> None:
        """Case-insensitive filesystems cannot merge separately named directories."""
        for names in (("a.txt", "A.txt"), ("Dir/a.txt", "dir/b.txt"),
                      ("one/Dir/a.txt", "one/dir/b.txt")):
            with self.subTest(names=names):
                self.reject(self.archive([self.regular(name) for name in names]), set(names))

    def test_file_cannot_be_parent_directory_in_either_order(self) -> None:
        """Reject file/directory conflicts independently of archive ordering."""
        for names in (("a.txt", "a.txt/b.txt"), ("a.txt/b.txt", "a.txt"),
                      ("A.txt", "a.txt/b.txt")):
            with self.subTest(names=names):
                self.reject(self.archive([self.regular(name) for name in names]), set(names))

    def test_links_directories_and_special_members_fail(self) -> None:
        """Only regular files belong in the PlatformIO archive contract."""
        for kind in (
            tarfile.SYMTYPE, tarfile.LNKTYPE, tarfile.DIRTYPE, tarfile.CHRTYPE,
            tarfile.BLKTYPE, tarfile.FIFOTYPE, tarfile.CONTTYPE, b"V", b"Z",
        ):
            with self.subTest(kind=kind):
                member = tarfile.TarInfo("a.txt")
                member.type = kind
                member.linkname = str(self.sentinel)
                self.reject(self.archive([(member, b"")]), {"a.txt"})

    def test_pax_path_cannot_override_a_safe_header(self) -> None:
        """Validate the effective PAX path, including traversal overrides."""
        member, payload = self.regular("a.txt")
        member.pax_headers = {"path": "../outside.txt"}
        self.reject(self.archive([(member, payload)]), {"a.txt"})

    def test_extension_header_names_cannot_escape(self) -> None:
        """Unused metadata names still obey path rejection before any payload."""
        for name in ("../outside.txt", "/outside.txt", "C:/outside.txt", "a\\b", "PaxHeader"):
            with self.subTest(name=name):
                extension = tarfile.TarInfo(name)
                extension.type = tarfile.XHDTYPE
                metadata = b"11 mtime=1\n"
                extension.size = len(metadata)
                member, payload = self.regular("a.txt")
                raw = (extension.tobuf(format=tarfile.USTAR_FORMAT) + metadata
                       + bytes(512 - len(metadata)) + member.tobuf(format=tarfile.USTAR_FORMAT)
                       + payload + bytes(512 - len(payload)) + bytes(1024))
                source = self.root / "metadata-name.tar.gz"
                source.write_bytes(gzip.compress(raw))
                if name == "PaxHeader":
                    library = package.extract_archive(source, self.work, {"a.txt"})
                    self.assertEqual((library / "a.txt").read_bytes(), payload)
                else:
                    self.reject(source, {"a.txt"})

    def test_pax_size_is_subject_to_member_bounds(self) -> None:
        """An extension cannot enlarge a file beyond the ordinary byte ceiling."""
        member, payload = self.regular("a.txt")
        member.pax_headers = {"size": "1000000000"}
        self.reject(self.archive([(member, payload)]), {"a.txt"}, max_member_bytes=32)

    def test_sparse_headers_fail(self) -> None:
        """Sparse PAX semantics cannot bypass ordinary payload accounting."""
        member, payload = self.regular("a.txt")
        member.pax_headers = {"GNU.sparse.map": "0,7", "GNU.sparse.size": "7"}
        self.reject(self.archive([(member, payload)]), {"a.txt"})

    def test_individual_payload_bound(self) -> None:
        """Reject a single member exceeding its individual byte ceiling."""
        source = self.archive([self.regular("a.txt", b"1234")])
        self.reject(source, {"a.txt"}, max_member_bytes=3)

    def test_total_payload_bound(self) -> None:
        """Individually small files still share the total payload ceiling."""
        source = self.archive([self.regular("a.txt", b"123"), self.regular("b.txt", b"456")])
        self.reject(source, {"a.txt", "b.txt"}, max_total_bytes=5)

    def test_member_count_bound(self) -> None:
        """Reject a second member when only one has been allowed."""
        source = self.archive([self.regular("a.txt"), self.regular("b.txt")])
        self.reject(source, {"a.txt", "b.txt"}, max_members=1)

    def test_compressed_input_bound(self) -> None:
        """Bound compressed input bytes before tar parsing or extraction."""
        source = self.archive([self.regular("a.txt")])
        self.reject(source, {"a.txt"}, max_archive_bytes=source.stat().st_size - 1)

    def test_uncompressed_tar_bound_includes_header_overhead(self) -> None:
        """The decompressed bound includes padding and headers, not just files."""
        source = self.archive([self.regular("a.txt", b"x")])
        self.reject(source, {"a.txt"}, max_tar_bytes=512)

    def test_uncompressed_bound_applies_to_pax_metadata(self) -> None:
        """Large compressed metadata cannot evade the decompressed ceiling."""
        member, payload = self.regular("a.txt", b"x")
        member.pax_headers = {"comment": "x" * 8192}
        self.reject(self.archive([(member, payload)]), {"a.txt"}, max_tar_bytes=4096)

    def test_gzip_truncation_and_invalid_format_fail(self) -> None:
        """Malformed and incomplete gzip/tar inputs produce deliberate failures."""
        good = self.archive([self.regular("a.txt")]).read_bytes()
        for payload in (b"", b"not gzip", good[:10], good[:-8], gzip.compress(b"not tar")):
            with self.subTest(size=len(payload)):
                source = self.root / "broken.tar.gz"
                source.write_bytes(payload)
                self.reject(source, {"a.txt"})

    def test_corrupt_gzip_checksum_fails(self) -> None:
        """Check the gzip footer even after all apparent tar members are readable."""
        source = self.archive([self.regular("a.txt")])
        raw = bytearray(source.read_bytes())
        raw[-8] ^= 0xFF
        source.write_bytes(raw)
        self.reject(source, {"a.txt"})

    def test_invalid_deflate_is_a_deliberate_failure(self) -> None:
        """Normalize malformed compressed blocks before writing any payload."""
        source = self.root / "invalid-deflate.tar.gz"
        source.write_bytes(bytes.fromhex("1f8b080000000000000306000000000000000000"))
        self.reject(source, {"a.txt"})

    def test_truncated_file_data_fails(self) -> None:
        """A correct file header does not certify an incomplete payload."""
        member = tarfile.TarInfo("a.txt")
        member.size = 1000
        source = self.root / "truncated.tar.gz"
        source.write_bytes(gzip.compress(member.tobuf(format=tarfile.USTAR_FORMAT) + b"short"))
        self.reject(source, {"a.txt"})

    def test_tarfile_extraction_helpers_are_never_used(self) -> None:
        """Production extraction copies validated bytes without tar filesystem APIs."""
        source = self.archive([self.regular("a.txt")])
        with mock.patch.object(tarfile.TarFile, "extract", side_effect=AssertionError), \
                mock.patch.object(tarfile.TarFile, "extractall", side_effect=AssertionError):
            library = package.extract_archive(source, self.work, {"a.txt"})
        self.assertEqual((library / "a.txt").read_bytes(), b"content")

    def test_original_archive_change_cannot_replace_validated_payload(self) -> None:
        """Replacement of the caller's archive cannot alter the private snapshot."""
        source = self.archive([self.regular("a.txt", b"original payload")])
        original = source.read_bytes()
        replacement = self.archive([self.regular("a.txt", b"changed payload")]).read_bytes()
        original_next = tarfile.TarFile.next
        replaced = False

        def replace_after_header(stream: tarfile.TarFile) -> tarfile.TarInfo | None:
            """Replace the input after parsing has begun against captured bytes."""
            nonlocal replaced
            member = original_next(stream)
            if member is not None and member.isfile() and not replaced:
                source.write_bytes(replacement)
                replaced = True
            return member

        with mock.patch.object(tarfile.TarFile, "next", replace_after_header):
            library = package.extract_archive(source, self.work, {"a.txt"})
        self.assertTrue(replaced)
        self.assertEqual((library / "a.txt").read_bytes(), b"original payload")
        self.assertEqual(hashlib.sha256((library.parent / "archive.tar.gz").read_bytes()).digest(),
                         hashlib.sha256(original).digest())

    def test_invalid_limits_and_missing_parent_fail_without_payload(self) -> None:
        """Invalid checker configuration cannot create payloads or alter caller files."""
        source = self.archive([self.regular("a.txt")])
        for value in (0, -1, True):
            with self.subTest(value=value):
                self.reject(source, {"a.txt"}, max_members=value)
        with self.assertRaises(package.PackageError):
            package.extract_archive(source, self.root / "absent", {"a.txt"})

    def test_missing_build_tool_is_an_invocation_error(self) -> None:
        """A missing explicit tool returns argparse exit 2 before archive work."""
        with redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as caught:
            package.main(["--archive", str(self.root / "absent.tar.gz"),
                          "--out", str(self.work), "--cmake", "missing-tusb32x-tool-42"])
        self.assertEqual(caught.exception.code, 2)
        self.assertEqual(list(self.work.iterdir()), [])

    def test_archive_io_failure_has_check_exit_one(self) -> None:
        """Failed archive input remains a deliberate check failure without a build."""
        with mock.patch.object(package, "package_files", return_value=["a.txt"]), \
                redirect_stdout(io.StringIO()):
            result = package.main([
                "--archive", str(self.root / "absent.tar.gz"), "--out", str(self.work),
                "--cmake", sys.executable, "--compiler", sys.executable,
                "--make-program", sys.executable])
        self.assertEqual(result, 1)
        self.assertEqual(list(self.work.iterdir()), [])

    def test_hidden_members_after_tar_terminator_fail(self) -> None:
        """A second tar stream cannot hide forbidden content after valid padding."""
        first = gzip.decompress(self.archive([self.regular("a.txt")]).read_bytes())
        extra = gzip.decompress(self.archive([self.regular("hidden.txt")]).read_bytes())
        source = self.root / "concatenated.tar.gz"
        source.write_bytes(gzip.compress(first + extra))
        self.reject(source, {"a.txt"})

    def test_invalid_trailing_header_is_not_silently_ignored(self) -> None:
        """A malformed header between the last file and padding must fail."""
        member, payload = self.regular("a.txt", b"x")
        raw = (member.tobuf(format=tarfile.USTAR_FORMAT) + payload + bytes(511)
               + b"invalid header" + bytes(512 - len(b"invalid header")) + bytes(1024))
        source = self.root / "trailing-garbage.tar.gz"
        source.write_bytes(gzip.compress(raw))
        self.reject(source, {"a.txt"})

    def test_recursive_pax_headers_fail_deliberately(self) -> None:
        """Nested extensions cannot cause an uncaught Python recursion failure."""
        extension = tarfile.TarInfo("PaxHeader")
        extension.type = tarfile.XHDTYPE
        member, payload = self.regular("a.txt", b"x")
        raw = (extension.tobuf(format=tarfile.USTAR_FORMAT) * 1100
               + member.tobuf(format=tarfile.USTAR_FORMAT) + payload + bytes(511)
               + bytes(1024))
        source = self.root / "recursive-pax.tar.gz"
        source.write_bytes(gzip.compress(raw))
        self.reject(source, {"a.txt"})

    def test_pax_size_cannot_hide_an_extra_physical_member(self) -> None:
        """An extension must not turn an unlisted file header into allowed payload."""
        extension = tarfile.TarInfo("PaxHeader")
        extension.type = tarfile.XHDTYPE
        metadata = b"13 size=1024\n"
        extension.size = len(metadata)
        member = tarfile.TarInfo("a.txt")
        hidden, payload = self.regular("hidden.txt", b"x")
        # The physical a.txt is empty. An overridden size would consume the next
        # file's header and data, hiding hidden.txt from tarfile's yielded names.
        raw = (extension.tobuf(format=tarfile.USTAR_FORMAT) + metadata
               + bytes(512 - len(metadata)) + member.tobuf(format=tarfile.USTAR_FORMAT)
               + hidden.tobuf(format=tarfile.USTAR_FORMAT) + payload + bytes(511)
               + bytes(1024))
        source = self.root / "pax-hidden-member.tar.gz"
        source.write_bytes(gzip.compress(raw))
        self.reject(source, {"a.txt"})


class DependencyContracts(unittest.TestCase):
    """Check the compiler's Make dependency quoting without requiring a compiler."""

    def test_make_spaces_and_continuations(self) -> None:
        """A continued rule preserves escaped spaces as part of a header path."""
        self.assertEqual(package.make_dependencies(
            "obj.o: /root/source.cpp \\\n /root/with\\ space/header.h\n"),
            ["/root/source.cpp", "/root/with space/header.h"])

    def test_make_special_characters_and_drive_paths(self) -> None:
        """Drive colons, escaped hashes/backslashes and doubled dollars survive."""
        self.assertEqual(package.make_dependencies(
            "C:/build/obj.o: C:/src/main.cpp /a/\\#file.h /a/$$file.h /a/back\\\\slash.h"),
            ["C:/src/main.cpp", "/a/#file.h", "/a/$file.h", "/a/back\\slash.h"])

    def test_make_missing_rule_is_rejected(self) -> None:
        """Missing dependency syntax cannot become an empty successful check."""
        with self.assertRaises(package.PackageError):
            package.make_dependencies("not a dependency rule")

    def test_make_windows_native_source_path(self) -> None:
        """MinGW emits native backslashes in the source prerequisite, unescaped."""
        self.assertEqual(package.make_dependencies(
            r"obj.o: C:\work\src\main.cpp C:/toolchain/header.h"),
            [r"C:\work\src\main.cpp", "C:/toolchain/header.h"])


def consumer_origins(argv):
    """Run a real positive and two escaped-header negatives; return zero only on PASS.

    Copy trusted exports, checker and fixture into an owned temporary checkout.
    Each full checker invocation is bounded to 35 seconds (105 seconds total).
    Keep stdout/stderr and builds under the printed evidence root for inspection.
    Explicit host tools are required; unavailable tools are invocation errors.
    """
    parser = argparse.ArgumentParser(description=consumer_origins.__doc__)
    parser.add_argument("--cmake", default=shutil.which("cmake"))
    parser.add_argument("--compiler", default=shutil.which("g++") or shutil.which("cl"))
    parser.add_argument("--make-program", default=shutil.which("ninja"))
    parser.add_argument("--generator", default="Ninja")
    parser.add_argument("--config", choices=("Debug", "Release"), default="Release")
    args = parser.parse_args(argv)
    for key in ("cmake", "compiler", "make_program"):
        tool = getattr(args, key)
        resolved = shutil.which(tool) if tool else None
        if resolved is None:
            parser.error(f"missing required build tool: {key}: {tool}")
        setattr(args, key, os.path.abspath(resolved))
    checkout = Path(__file__).resolve().parents[1]
    evidence = Path(tempfile.mkdtemp(prefix="tusb32x-package-origins-")).resolve()
    copied = evidence / "trusted-copy"
    names = package.package_files(checkout) + [
        "tools/check_package.py", "test/package_consumer/CMakeLists.txt",
        "test/package_consumer/main.cpp"]
    try:
        for name in names:
            destination = copied / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(checkout / name, destination)
        source = copied / "src/TUSB32xOperations.cpp"
        original = source.read_text(encoding="utf-8")
        include = '#include "TUSB32xOwnerTime.h"'
        if original.count(include) != 1:
            raise package.PackageError("owner-time negative fixture include changed")
        sibling = evidence / "sibling" / "OwnerTime.h"
        sibling.parent.mkdir()
        shutil.copyfile(copied / "src/TUSB32xOwnerTime.h", sibling)
        for label, header in (("clean", None),
                              ("checkout-header", copied / "src/TUSB32xOwnerTime.h"),
                              ("sibling-header", sibling)):
            source.write_text(original if header is None else original.replace(
                include, f'#include "{header.as_posix()}"'), encoding="utf-8")
            command = [sys.executable, str(copied / "tools/check_package.py"),
                       "--test-archive", "--out", str(evidence / label)]
            for key in ("cmake", "compiler", "make_program", "generator", "config"):
                command.extend(["--" + key.replace("_", "-"), getattr(args, key)])
            result = subprocess.run(command, capture_output=True, text=True, timeout=35)
            output = result.stdout + result.stderr
            (evidence / f"{label}.log").write_text(output, encoding="utf-8")
            if header is None:
                if result.returncode != 0 or '"consumer": "PASS"' not in output:
                    raise package.PackageError(f"clean consumer failed: {output}")
            elif (result.returncode != 1 or
                  "compiler dependency outside clean sources/toolchain:" not in output):
                raise package.PackageError(f"escaped header was not rejected: {label}: {output}")
            print(f"PASS: {label}", flush=True)
        print(f"Compiler origin evidence: {evidence}")
        return 0
    except (package.PackageError, OSError, subprocess.SubprocessError) as error:
        print(f"FAIL: {error}\nCompiler origin evidence: {evidence}")
        return 1


if __name__ == "__main__":
    if "--consumer-origins" in sys.argv:
        arguments = sys.argv[1:]
        arguments.remove("--consumer-origins")
        raise SystemExit(consumer_origins(arguments))
    unittest.main()
