"""Validate a trusted local package, extract safely, and run its isolated C++ consumer.

Archive parsing is bounded and never uses tarfile's filesystem extraction. Builds
execute source code and therefore require a package made from this trusted checkout;
path validation does not sandbox arbitrary CMake. All subprocesses use argument
lists and finite timeouts. Temporary evidence is retained for inspection.
"""

import argparse
from dataclasses import dataclass
import gzip
import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import tempfile
from urllib.parse import unquote, urlsplit
import zlib


class PackageError(RuntimeError):
    """An unsafe/incomplete archive or failed consumer check; CLI exit is 1."""


@dataclass(frozen=True)
class ArchiveLimits:
    """Positive byte/member ceilings, including hidden tar extension metadata."""

    max_members: int = 512
    max_member_bytes: int = 2 * 1024 * 1024
    max_total_bytes: int = 16 * 1024 * 1024
    max_tar_bytes: int = 32 * 1024 * 1024
    max_archive_bytes: int = 8 * 1024 * 1024


def validate_name(name):
    """Return portable ASCII path parts; reject aliases before any filesystem use."""
    if not isinstance(name, str) or not name or len(name) > 240:
        raise PackageError("empty or overlong archive path")
    parts = name.split("/")
    if len(parts) > 16:
        raise PackageError(f"too many path segments: {name!r}")
    for part in parts:
        if (len(part) > 100 or not re.fullmatch(r"[A-Za-z0-9_.-]+", part)
                or part in (".", "..") or part.endswith(".")):
            raise PackageError(f"nonportable archive path: {name!r}")
        stem = part.split(".", 1)[0].upper()
        if stem in {"CON", "PRN", "AUX", "NUL"} or re.fullmatch(r"(?:COM|LPT)[1-9]", stem):
            raise PackageError(f"Windows device path: {name!r}")
    return tuple(parts)


def check_tar_headers(snapshot, limits):
    """Bound physical headers before tarfile processes extension-header recursion.

    Accept regular POSIX files with at most one per-file PAX mtime header. This is the
    format emitted by the managed packer; links, global/GNU extensions and sparse
    formats are unnecessary for this distribution. End padding must be all zero,
    so a concatenated archive cannot hide files from the exact-content check.
    """
    with snapshot.open("rb") as source:
        files = 0
        extension_pending = False
        while True:
            header = source.read(512)
            if len(header) != 512:
                raise PackageError("truncated tar header")
            if header == bytes(512):
                if extension_pending or source.read(512) != bytes(512):
                    raise PackageError("invalid tar end marker")
                while True:
                    padding = source.read(65536)
                    if not padding:
                        return
                    if any(padding):
                        raise PackageError("nonzero content after tar end marker")
            member = tarfile.TarInfo.frombuf(header, "ascii", "strict")
            if member.size < 0 or member.size > limits.max_member_bytes:
                raise PackageError("physical tar member byte limit exceeded")
            if member.type == tarfile.XHDTYPE:
                # Python's packer uses this pseudo-name; it never becomes a path.
                if member.name != "././@PaxHeader":
                    validate_name(member.name)
                if extension_pending:
                    raise PackageError("consecutive tar extension headers are forbidden")
                extension_pending = True
                metadata = source.read(member.size)
                length, _, record = metadata.partition(b" ")
                if (length != str(len(metadata)).encode("ascii") or not re.fullmatch(
                        rb"mtime=-?[0-9]+(?:\.[0-9]+)?\n", record)):
                    raise PackageError("only a single PAX mtime record is supported")
                source.seek((-member.size) % 512, 1)
                continue
            elif member.type in (tarfile.REGTYPE, tarfile.AREGTYPE):
                files += 1
                if files > limits.max_members:
                    raise PackageError("physical tar member count limit exceeded")
                extension_pending = False
            else:
                raise PackageError("only regular files and per-file PAX metadata are supported")
            source.seek(((member.size + 511) // 512) * 512, 1)


def extract_archive(archive, work_dir, required_files, limits=ArchiveLimits()):
    """Scan a bounded immutable gzip/tar snapshot, then extract exact regular files.

    work_dir is an existing trusted parent. A fresh private workspace holds the
    snapshot and renamed library; no existing child is overwritten or removed.
    Failures leave bounded evidence there and never create an extraction tree
    before the complete name/type/content scan succeeds. Return the renamed root.
    """
    if any(type(value) is not int or value < 1 for value in vars(limits).values()):
        raise PackageError("archive limits must be positive integers")
    required = set(required_files)
    for name in required:
        validate_name(name)
    if not required:
        raise PackageError("an exact nonempty content contract is required")
    try:
        parent = Path(work_dir).resolve(strict=True)
        if not parent.is_dir():
            raise PackageError("extraction parent is not a directory")
        # Read once: later replacement of the supplied archive cannot change extraction.
        with Path(archive).open("rb") as source:
            packed = source.read(limits.max_archive_bytes + 1)
        if len(packed) > limits.max_archive_bytes:
            raise PackageError("compressed archive byte limit exceeded")
        workspace = Path(tempfile.mkdtemp(prefix="package-", dir=parent)).resolve()
        (workspace / "archive.tar.gz").write_bytes(packed)
        snapshot = workspace / "archive.tar"
        total = 0
        with gzip.GzipFile(fileobj=io.BytesIO(packed)) as source, snapshot.open("xb") as dest:
            while True:
                block = source.read(min(65536, limits.max_tar_bytes - total + 1))
                if not block:
                    break
                total += len(block)
                if total > limits.max_tar_bytes:
                    raise PackageError("uncompressed tar byte limit exceeded")
                dest.write(block)
        if total < 1024 or total % 512:
            raise PackageError("truncated tar archive")
        check_tar_headers(snapshot, limits)
        with tarfile.open(snapshot, "r:") as package:
            members = []
            prefixes = {}
            names = set()
            total_bytes = 0
            for member in package:
                if len(members) >= limits.max_members:
                    raise PackageError("archive member count limit exceeded")
                parts = validate_name(member.name)
                if not member.isreg() or member.issparse() or member.linkname:
                    raise PackageError(f"not an ordinary regular file: {member.name}")
                if set(member.pax_headers) - {"mtime"}:
                    raise PackageError("semantic PAX overrides are forbidden")
                if member.size < 0 or member.size > limits.max_member_bytes:
                    raise PackageError(f"member byte limit exceeded: {member.name}")
                total_bytes += member.size
                if total_bytes > limits.max_total_bytes:
                    raise PackageError("total member byte limit exceeded")
                if member.name in names:
                    raise PackageError(f"duplicate archive member: {member.name}")
                names.add(member.name)
                for index in range(1, len(parts) + 1):
                    prefix = "/".join(parts[:index])
                    key = prefix.casefold()
                    kind = "file" if index == len(parts) else "directory"
                    previous = prefixes.get(key)
                    if previous and previous != (prefix, kind):
                        raise PackageError(f"case or file/directory collision: {prefix}")
                    prefixes[key] = (prefix, kind)
                with package.extractfile(member) as source:
                    remaining = member.size
                    while remaining:
                        block = source.read(min(65536, remaining))
                        if not block:
                            raise PackageError(f"truncated file: {member.name}")
                        remaining -= len(block)
                members.append(member)
            if names != required:
                missing = sorted(required - names)
                extra = sorted(names - required)
                raise PackageError(
                    f"package content mismatch; missing={missing}; forbidden={extra}")
            staging = workspace / "extracted"
            staging.mkdir()
            for member in members:
                destination = staging.joinpath(*validate_name(member.name))
                destination.parent.mkdir(parents=True, exist_ok=True)
                # No archive-created links, metadata, permissions or timestamps are applied.
                with package.extractfile(member) as source, destination.open("xb") as dest:
                    shutil.copyfileobj(source, dest, 65536)
            renamed = workspace / "renamed-tusb32x"
            staging.rename(renamed)
            return renamed
    except (OSError, EOFError, tarfile.TarError, ValueError, RecursionError, zlib.error) as error:
        raise PackageError(f"cannot validate/extract archive: {error}") from error


def package_files(root):
    """Read the trusted checkout's literal export contract and matching IDF list."""
    manifest = json.loads((root / "library.json").read_text(encoding="utf-8"))
    names = manifest["export"]["include"]
    if len(names) != len(set(names)):
        raise PackageError("duplicate export include")
    for name in names:
        validate_name(name)
        path = root / name
        if path.is_symlink() or not path.is_file() or not path.resolve().is_relative_to(root):
            raise PackageError(f"export is not a checkout regular file: {name}")
    component = (root / "idf_component.yml").read_text(encoding="utf-8")
    includes = component.split("  include:\n", 1)[1]
    if re.findall(r'^    - "([^"]+)"$', includes, re.MULTILINE) != names:
        raise PackageError("IDF and PlatformIO content contracts differ")
    # These inputs cannot disappear silently when editing either packaging manifest.
    critical = {
        "LICENSE", "library.json", "CMakeLists.txt", "idf_component.yml",
        "include/TUSB32x/Version.h", "include/TUSB32x/TUSB32x.h",
        "include/TUSB32x/Status.h", "include/TUSB32x/Config.h",
        "include/TUSB32x/CommandTable.h", "examples/common/ConsoleBuffer.h",
        "examples/common/Tusb32xCli.h", "examples/common/Tusb32xCli.cpp",
        "examples/common/Tusb32xCliOutput.cpp", "examples/common/ExampleOwner.h",
        "examples/common/ExampleOwner.cpp", "examples/common/OutputMailbox.h",
        "examples/arduino/BoardConfig.h", "examples/esp_idf/BoardConfig.h",
        "examples/esp_idf/basic/CMakeLists.txt", "examples/esp_idf/basic/main/CMakeLists.txt",
        "examples/esp_idf/basic/main/main.cpp", "examples/esp_idf/basic/sdkconfig.defaults",
        "scripts/arduino_identity.py", "scripts/idf_identity.py", "scripts/pio.cmd",
        "platformio.ini",
    }
    if not critical <= set(names):
        raise PackageError(f"missing required export inputs: {sorted(critical - set(names))}")
    forbidden = {"build", ".pio", ".git", ".cache", "__pycache__", "reference", "audit",
                 "prompts", "test", "tools", "managed_components"}
    for name in names:
        if (forbidden.intersection(name.split("/")) or name.endswith(
                (".pdf", ".bin", ".elf", ".log", ".lock", ".pyc", ".exe", ".dll",
                 ".o", ".obj", ".a", ".lib", ".map"))
                or name.split("/")[-1] in ("sdkconfig", "sdkconfig.old")):
            raise PackageError(f"forbidden export input: {name}")
    return names


def markdown_links(root, names):
    """Check local inline/reference Markdown links and fragments within a given tree."""
    count = 0
    for name in names:
        if not name.endswith(".md"):
            continue
        path = root / name
        text = path.read_text(encoding="utf-8")
        links = re.findall(r"!?\[[^\]]*\]\(([^)]+)\)", text)
        links += re.findall(r"^\s*\[[^\]]+\]:\s*(\S+)", text, re.MULTILINE)
        for link in links:
            link = link.split(' "', 1)[0].strip("<>")
            parsed = urlsplit(link)
            if parsed.scheme or parsed.netloc:
                continue
            target = (path.parent / unquote(parsed.path)).resolve() if parsed.path else path
            if not target.is_relative_to(root) or not target.exists():
                raise PackageError(f"broken/escaping Markdown link: {name}: {link}")
            if parsed.fragment and target.suffix == ".md":
                headings = re.findall(r"^#{1,6}\s+(.+?)\s*#*$",
                                      target.read_text(encoding="utf-8"), re.MULTILINE)
                anchors = set()
                occurrences = {}
                for heading in headings:
                    slug = re.sub(r"[^\w -]", "", heading.lower()).replace(" ", "-")
                    index = occurrences.get(slug, 0)
                    occurrences[slug] = index + 1
                    anchors.add(slug if not index else f"{slug}-{index}")
                anchors.update(re.findall(r'<a\s+(?:name|id)=["\x27]([^"\x27]+)',
                                          target.read_text(encoding="utf-8")))
                if unquote(parsed.fragment) not in anchors:
                    raise PackageError(f"missing Markdown fragment: {name}: {link}")
            count += 1
    return count


def run(command, environment):
    """Run one argument-list child with a 60-second bound, preserving its output."""
    command = [str(item) for item in command]
    print(subprocess.list2cmdline(command), flush=True)
    subprocess.run(command, env=environment, check=True, timeout=60)


def make_dependencies(text):
    """Decode one compiler Make depfile, including escaped spaces, hashes and dollars.

    Return prerequisite paths, excluding the object target. Only the single-rule
    depfiles emitted by the consumer's GNU/Clang CMake builds are accepted.
    """
    text = text.replace("\\\n", "")
    separator = re.search(r":\s", text)
    if separator is None:
        raise PackageError("missing compiler dependency rule")
    text = text[separator.end():]
    paths = []
    token = ""
    index = 0
    while index < len(text):
        char = text[index]
        if (char == "\\" and index + 1 < len(text)
                and (text[index + 1].isspace() or text[index + 1] in "\\#:")):
            index += 1
            token += text[index]
        elif char == "$" and text[index:index + 2] == "$$":
            token += "$"
            index += 1
        elif char.isspace():
            if token:
                paths.append(token)
                token = ""
        else:
            token += char
        index += 1
    if token:
        paths.append(token)
    return paths


def check_dependencies(root, fixture, build, args, environment, compile_count):
    """Reject actual compiler inputs outside the copied sources or compiler/SDK roots.

    Read Ninja's dependency database or GNU/Clang Make depfiles after the build.
    Require one nonempty dependency record per compiled source; unsupported or
    missing dependency evidence fails rather than certifying header provenance.
    The Ninja query has the same 60-second subprocess bound as the build.
    """
    if args.generator in ("Ninja", "Ninja Multi-Config"):
        result = subprocess.run(
            [args.make_program, "-C", str(build), "-t", "deps"],
            env=environment, text=True, capture_output=True, check=True, timeout=60)
        records = []
        for line in result.stdout.splitlines():
            if line.startswith("    "):
                if not records:
                    raise PackageError("unexpected Ninja dependency output")
                records[-1].append(line[4:])
            elif line:
                if not re.fullmatch(r".+: #deps [0-9]+, deps mtime [0-9]+ \(VALID\)", line):
                    raise PackageError(f"invalid Ninja dependency record: {line}")
                records.append([])
    elif args.generator in ("Unix Makefiles", "MinGW Makefiles"):
        depfiles = sorted(build.rglob("*.o.d")) + sorted(build.rglob("*.obj.d"))
        records = [make_dependencies(path.read_text(encoding="utf-8")) for path in depfiles]
    else:
        raise PackageError(f"compiler dependency inspection unsupported: {args.generator}")
    if len(records) != compile_count or any(not record for record in records):
        raise PackageError("missing or incomplete compiler dependency records")
    approved = [root.resolve(), fixture.resolve()]
    for directory in (build / "package-toolchain-includes.txt").read_text(
            encoding="utf-8").splitlines():
        path = Path(directory)
        if not path.is_absolute():
            raise PackageError("nonabsolute compiler include root")
        approved.append(path.resolve())
    dependencies = set()
    for record in records:
        for name in record:
            path = Path(name)
            path = (path if path.is_absolute() else build / path).resolve(strict=True)
            if not any(path.is_relative_to(directory) for directory in approved):
                raise PackageError(f"compiler dependency outside clean sources/toolchain: {path}")
            dependencies.add(str(path))
    (build / "package-compiler-dependencies.txt").write_text(
        "\n".join(sorted(dependencies)) + "\n", encoding="utf-8")


def consume(root, checkout, args):
    """Copy only the public consumer fixture, build/run, and verify extracted origins."""
    workspace = root.parent
    fixture = workspace / "consumer"
    fixture.mkdir()
    for name in ("CMakeLists.txt", "main.cpp"):
        shutil.copyfile(checkout / "test/package_consumer" / name, fixture / name)
    environment = os.environ.copy()
    # Do not carry checkout/tool injection through CMake's search environment.
    for key in ("CMAKE_PREFIX_PATH", "CMAKE_MODULE_PATH", "CMAKE_TOOLCHAIN_FILE", "CPATH",
                "CPLUS_INCLUDE_PATH", "C_INCLUDE_PATH", "LIBRARY_PATH", "PYTHONPATH",
                "CXXFLAGS", "CPPFLAGS", "LDFLAGS", "CL", "_CL_"):
        environment.pop(key, None)
    if os.name == "nt":
        system = Path(os.environ["SystemRoot"])
        runtime = workspace / "runtime"
        runtime.mkdir()
        for pattern in ("libgcc*.dll", "libstdc++*.dll", "libwinpthread*.dll",
                        "libmcfgthread*.dll", "libssp*.dll"):
            for dll in Path(args.compiler).parent.glob(pattern):
                shutil.copyfile(dll, runtime / dll.name)
        environment["PATH"] = os.pathsep.join(map(str, (system / "System32", system, runtime)))
    else:
        # Build tools are explicit. POSIX compiler helpers still need system tools.
        tool_dir = workspace / "tool-path"
        tool_dir.mkdir()
        for name in ("as", "ld", "ar", "ranlib", "make", "sh"):
            tool = shutil.which(name)
            if tool:
                (tool_dir / name).symlink_to(tool)
        environment["PATH"] = str(tool_dir)
    for tool in ("git", "python", "python3", "doxygen", "pio"):
        if shutil.which(tool, path=environment["PATH"]):
            raise PackageError(f"maintainer tool leaked into consumer PATH: {tool}")
    build = workspace / "consumer-build"
    multi_config = ("Multi-Config" in args.generator or args.generator.startswith("Visual Studio")
                    or args.generator == "Xcode")
    configuration = [] if multi_config else [f"-DCMAKE_BUILD_TYPE={args.config}"]
    run([args.cmake, "-S", fixture, "-B", build, "-G", args.generator,
         f"-DTUSB32X_PACKAGE_ROOT={root}", f"-DCMAKE_CXX_COMPILER={args.compiler}",
         f"-DCMAKE_MAKE_PROGRAM={args.make_program}", *configuration,
         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_CXX_STANDARD=11"], environment)
    run([args.cmake, "--build", build, "--config", args.config], environment)
    cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
    if "TUSB32X_BUILD_TESTS:BOOL=OFF" not in cache or re.search(
            r"(?:Python|PYTHON|DOXYGEN|GIT_EXECUTABLE|GTest|Catch2).*?=", cache):
        raise PackageError("consumer discovered a maintainer/test dependency")
    origin = (build / "package-consumer-origin.txt").read_text(encoding="utf-8")
    compiles = (build / "compile_commands.json").read_text(encoding="utf-8")
    for value in (origin, compiles):
        if str(checkout).replace("\\", "/").casefold() in value.replace("\\", "/").casefold():
            raise PackageError("consumer used source-checkout paths")
    entries = json.loads(compiles)
    for entry in entries:
        if not Path(entry["file"]).resolve().is_relative_to(workspace):
            raise PackageError("compiled source outside clean workspace")
    check_dependencies(root, fixture, build, args, environment,
                       len({entry["file"] for entry in entries}))
    filename = "tusb32x_package_consumer" + (".exe" if os.name == "nt" else "")
    executable = build / filename
    if not executable.exists():
        executable = build / args.config / filename
    run([executable], environment)
    print("PASS: tests-OFF consumer compiled, linked and ran without maintainer tools")


def main(argv=None):
    """Validate content/links/consumer; return 0 PASS, 1 failed check, 2 invocation."""
    parser = argparse.ArgumentParser(description=__doc__)
    selection = parser.add_mutually_exclusive_group(required=True)
    selection.add_argument("--archive", type=Path, help="trusted locally packed .tar.gz")
    selection.add_argument("--test-archive", action="store_true",
                           help="create a bounded test tar from the explicit export list")
    parser.add_argument("--out", type=Path, required=True,
                        help="artifact parent (created if absent)")
    parser.add_argument("--cmake", default=shutil.which("cmake"))
    parser.add_argument("--compiler", default=shutil.which("g++") or shutil.which("cl"))
    parser.add_argument("--make-program", default=shutil.which("ninja"))
    parser.add_argument("--generator", default="Ninja")
    parser.add_argument("--config", choices=("Debug", "Release"), default="Release")
    args = parser.parse_args(argv)
    if not all((args.cmake, args.compiler, args.make_program)):
        parser.error("CMake, a C++ compiler and the generator's build tool are required")
    for key in ("cmake", "compiler", "make_program"):
        executable = shutil.which(str(getattr(args, key)))
        if executable is None:
            parser.error(f"missing build tool: {getattr(args, key)}")
        setattr(args, key, str(Path(executable).resolve()))
    checkout = Path(__file__).resolve().parents[1]
    try:
        names = package_files(checkout)
        args.out.mkdir(parents=True, exist_ok=True)
        if args.test_archive:
            generated = Path(tempfile.mkdtemp(prefix="test-archive-", dir=args.out))
            args.archive = generated / "TUSB32x.tar.gz"
            with tarfile.open(args.archive, "w:gz", format=tarfile.PAX_FORMAT) as package:
                for name in names:
                    package.add(checkout / name, arcname=name, recursive=False)
        # CTest artifacts usually live under the checkout. Put compilation elsewhere
        # so a checkout path in the compiler's input is always an observable error.
        extraction_parent = args.out.resolve()
        if extraction_parent.is_relative_to(checkout):
            extraction_parent = Path(tempfile.gettempdir()).resolve()
        if extraction_parent.is_relative_to(checkout):
            raise PackageError("temporary directory must be outside the source checkout")
        root = extract_archive(args.archive, extraction_parent, names)
        for name in names:
            if (root / name).read_bytes() != (checkout / name).read_bytes():
                raise PackageError(f"archive differs from trusted checkout: {name}")
        links = markdown_links(root, names)
        source_links = markdown_links(checkout, names)
        consume(root, checkout, args)
        digest = hashlib.sha256((root.parent / "archive.tar.gz").read_bytes()).hexdigest()
        report = {"archive": str(args.archive.resolve()), "sha256": digest,
                  "validated_snapshot": str(root.parent / "archive.tar.gz"),
                  "extracted": str(root), "files": len(names), "package_links": links,
                  "source_links": source_links, "consumer": "PASS"}
        (root.parent / "package-report.json").write_text(
            json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0
    except (PackageError, OSError, ValueError, KeyError, IndexError,
            subprocess.SubprocessError) as error:
        print(f"FAIL: {error}")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
