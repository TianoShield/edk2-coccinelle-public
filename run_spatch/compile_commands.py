from __future__ import annotations

import argparse
import configparser
import json
import os
import shlex
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

from .errors import FatalError
from .line_endings import normalize_file
from .uncrustify import get_uncrustify, uncrustify_files


@dataclass(frozen=True)
class CompileCommand:
    directory: Path
    file: Path
    raw_file: str
    include_dirs: tuple[str, ...]


def _load_json(path: Path) -> object:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:  # pragma: no cover - error path
        raise FatalError(f"failed to parse JSON {path}: {exc}") from exc


def _iter_command_tokens(entry: dict[str, object], *, database_path: Path) -> list[str]:
    arguments = entry.get("arguments")
    if isinstance(arguments, list):
        if not all(isinstance(arg, str) for arg in arguments):
            raise FatalError(f"{database_path}: compile command arguments must be strings")
        return [arg for arg in arguments if arg]

    command = entry.get("command")
    if isinstance(command, str):
        try:
            return shlex.split(command)
        except ValueError as exc:  # pragma: no cover - error path
            raise FatalError(f"{database_path}: failed to parse command string: {exc}") from exc

    raise FatalError(f"{database_path}: compile command must contain arguments or command")


def _extract_include_dirs(tokens: Sequence[str]) -> list[str]:
    includes: list[str] = []
    idx = 0
    while idx < len(tokens):
        token = tokens[idx]
        if token == "-I":
            idx += 1
            if idx >= len(tokens):
                raise FatalError("found -I without a directory argument")
            includes.append(tokens[idx])
        elif token.startswith("-I") and token != "-I":
            includes.append(token[2:])
        idx += 1
    return includes


def _resolve_file_path(*, directory: Path, file_path: str) -> Path:
    path = Path(file_path)
    if not path.is_absolute():
        path = directory / path
    return path.resolve()


def _unique_in_order(values: Iterable[str]) -> list[str]:
    seen: set[str] = set()
    ordered: list[str] = []
    for value in values:
        if value not in seen:
            seen.add(value)
            ordered.append(value)
    return ordered


def _load_compile_commands(path: Path) -> list[CompileCommand]:
    raw = _load_json(path)
    if not isinstance(raw, list):
        raise FatalError(f"{path}: compile_commands.json root must be a list")

    commands: list[CompileCommand] = []

    for idx, entry in enumerate(raw, start=1):
        if not isinstance(entry, dict):
            raise FatalError(f"{path}: compile command #{idx} must be an object")

        directory_raw = entry.get("directory")
        file_raw = entry.get("file")
        if not isinstance(directory_raw, str) or not directory_raw:
            raise FatalError(f"{path}: compile command #{idx} missing/invalid directory")
        if not isinstance(file_raw, str) or not file_raw:
            raise FatalError(f"{path}: compile command #{idx} missing/invalid file")

        directory = Path(directory_raw)
        if not directory.is_absolute():
            directory = (path.parent / directory).resolve()
        else:
            directory = directory.resolve()

        file_path = _resolve_file_path(directory=directory, file_path=file_raw)
        tokens = _iter_command_tokens(entry, database_path=path)
        include_dirs = tuple(_unique_in_order(_extract_include_dirs(tokens)))

        commands.append(
            CompileCommand(
                directory=directory,
                file=file_path,
                raw_file=file_raw,
                include_dirs=include_dirs,
            )
        )

    return commands


def _is_relative_to(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
    except ValueError:
        return False
    return True


def _load_submodule_dirs(edk2_repo: Path) -> tuple[Path, ...]:
    gitmodules = edk2_repo / ".gitmodules"
    if not gitmodules.is_file():
        return ()

    parser = configparser.ConfigParser()
    parser.read(gitmodules, encoding="utf-8")

    paths: list[Path] = []
    for section in parser.sections():
        if not section.startswith("submodule "):
            continue
        if not parser.has_option(section, "path"):
            continue
        paths.append((edk2_repo / parser.get(section, "path")).resolve())
    return tuple(sorted(paths))


def _is_in_submodule(path: Path, submodule_dirs: Sequence[Path]) -> bool:
    return any(_is_relative_to(path, submodule_dir) for submodule_dir in submodule_dirs)


def _iter_source_files(root: Path, *, submodule_dirs: Sequence[Path]) -> Iterable[Path]:
    root = root.resolve()
    if _is_in_submodule(root, submodule_dirs):
        return

    for dir_path_str, dir_names, file_names in os.walk(root):
        dir_path = Path(dir_path_str)
        dir_names[:] = [
            name
            for name in sorted(dir_names)
            if not _is_in_submodule((dir_path / name).resolve(), submodule_dirs)
        ]

        for name in sorted(file_names):
            path = dir_path / name
            if path.suffix.lower() in {".c", ".h"} and path.is_file():
                yield path.resolve()


def _build_spatch_command(
    *,
    file_path: Path,
    include_dirs: Sequence[str],
    use_compile_includes: bool,
    sp_file: Path,
    macro_file_builtins: Path | None,
    spatch_args: Sequence[str],
) -> list[str]:
    cmd = ["spatch", "--in-place", "--smpl-spacing"]
    if use_compile_includes:
        for include_dir in include_dirs:
            cmd.extend(["-I", include_dir])
        cmd.extend(["--recursive-includes", "--include-headers-for-types"])
    cmd.extend(spatch_args)
    if macro_file_builtins is not None:
        cmd.extend(["--macro-file-builtins", str(macro_file_builtins)])
    cmd.extend(["--sp-file", str(sp_file)])
    cmd.append(str(file_path))
    return cmd


def _run_spatch_command(*, file_path: Path, cmd: Sequence[str]) -> tuple[int, bool]:
    before = file_path.read_bytes()
    cp = subprocess.run(list(cmd), check=False)
    changed = False
    if cp.returncode == 0:
        after = file_path.read_bytes()
        if after != before:
            changed = True
            normalize_file(file_path)
    return cp.returncode, changed


def main(argv: list[str]) -> int:
    if "--" in argv:
        split_at = argv.index("--")
        argv, spatch_args = argv[:split_at], argv[split_at + 1 :]
    else:
        spatch_args = []

    parser = argparse.ArgumentParser(
        prog="python3 -m run_spatch compile-commands",
        description=(
            "Run spatch on .c and .h files under a directory, using "
            "compile_commands.json entries for include-aware files."
        ),
        epilog="Any arguments after -- are passed through to spatch.",
    )
    parser.add_argument("compile_commands", help="path to compile_commands.json")
    parser.add_argument(
        "--dir",
        required=True,
        help="directory recursively scanned for .c and .h files",
    )
    parser.add_argument(
        "--sp-file",
        required=True,
        help="path to the cocci script used by spatch",
    )
    parser.add_argument(
        "--macro-file-builtins",
        help="path to the builtins macro file passed to spatch",
    )
    parser.add_argument(
        "--edk2-repo",
        required=True,
        help="path to edk2 repo used to locate uncrustify",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="print the spatch commands without executing them or running uncrustify",
    )
    args = parser.parse_args(argv)

    compile_commands_path = Path(args.compile_commands).resolve()
    source_dir = Path(args.dir).expanduser().resolve()
    sp_file = Path(args.sp_file).expanduser().resolve()
    macro_file_builtins = (
        Path(args.macro_file_builtins).expanduser().resolve()
        if args.macro_file_builtins
        else None
    )
    edk2_repo = Path(args.edk2_repo).expanduser().resolve()
    if not args.dry_run and shutil.which("spatch") is None:
        raise FatalError("required tool not found in PATH: spatch")

    if not source_dir.is_dir():
        raise FatalError(f"--dir does not name a directory: {source_dir}")

    submodule_dirs = _load_submodule_dirs(edk2_repo)
    source_files = list(_iter_source_files(source_dir, submodule_dirs=submodule_dirs))
    selected_file_set = set(source_files)
    commands = _load_compile_commands(compile_commands_path)

    seen_files: dict[Path, CompileCommand] = {}
    for command in commands:
        if command.file not in selected_file_set:
            continue
        previous = seen_files.get(command.file)
        if previous is not None:
            sys.stderr.write(
                "warning: duplicate compile command skipped: "
                f"{command.file} (duplicate raw file {command.raw_file!r}; "
                f"first raw file {previous.raw_file!r})\n"
            )
            continue
        seen_files[command.file] = command

    jobs = []
    for file_path in source_files:
        command = seen_files.get(file_path)
        jobs.append(
            (
                file_path,
                _build_spatch_command(
                    file_path=file_path,
                    include_dirs=command.include_dirs if command is not None else (),
                    use_compile_includes=command is not None,
                    sp_file=sp_file,
                    macro_file_builtins=macro_file_builtins,
                    spatch_args=spatch_args,
                ),
            )
        )

    if not jobs:
        return 0

    if args.dry_run:
        for _, spatch_cmd in jobs:
            print(shlex.join(spatch_cmd))
        return 0

    max_workers = min(len(jobs), max(2, os.cpu_count() or 2))
    failures: list[Path] = []
    changed_files: list[Path] = []
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        future_to_command = {
            executor.submit(_run_spatch_command, file_path=file_path, cmd=spatch_cmd): file_path
            for file_path, spatch_cmd in jobs
        }
        for future in as_completed(future_to_command):
            file_path = future_to_command[future]
            try:
                returncode, changed = future.result()
            except Exception as exc:  # pragma: no cover - subprocess launch failure
                raise FatalError(f"spatch failed for {file_path}: {exc}") from exc
            if returncode != 0:
                failures.append(file_path)
            elif changed:
                changed_files.append(file_path)

    if failures:
        failed = "\n".join(f"  {path}" for path in failures)
        raise FatalError(f"spatch failed for one or more files:\n{failed}")

    if changed_files:
        uncrustify_cfg = get_uncrustify(edk2_repo)
        uncrustify_files(uncrustify_cfg, sorted(changed_files))

    return 0
