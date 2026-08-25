#!/usr/bin/env python3

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path
from typing import Iterable, Optional

from run_spatch.errors import FatalError
from run_spatch.line_endings import normalize_file
from run_spatch.uncrustify import UncrustifyConfig, get_uncrustify, uncrustify_files


def _which_or_error(tool: str) -> str:
    path = shutil.which(tool)
    if not path:
        raise FatalError(f"required tool not found in PATH: {tool}")
    return path


def resolve_edk2_repo(repo_arg: Optional[str], *, script_root: Path) -> Path:
    env_repo = os.environ.get("EDK2_REPO")
    if env_repo:
        repo = Path(env_repo)
    elif repo_arg:
        repo = Path(repo_arg)
    else:
        repo = (script_root / ".." / ".." / ".." / "edk2").resolve()

    repo = repo.resolve()
    if not (repo / ".git").exists():
        raise FatalError(f"not a git repo: {repo}")
    return repo


def run(cmd: list[str], *, cwd: Optional[Path] = None, stdin: Optional[bytes] = None) -> subprocess.CompletedProcess:
    return subprocess.run(
        cmd,
        cwd=str(cwd) if cwd else None,
        input=stdin,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )


def git_show_file(edk2_repo: Path, revspec: str, relpath: str) -> bytes:
    _which_or_error("git")
    cp = run(["git", "-C", str(edk2_repo), "show", f"{revspec}:{relpath}"])
    if cp.returncode != 0:
        raise FatalError(
            "git show failed\n"
            f"  repo: {edk2_repo}\n"
            f"  spec: {revspec}:{relpath}\n"
            f"  stderr:\n{cp.stderr.decode(errors='replace')}"
        )
    return cp.stdout


def apply_spatch(
    *,
    cocci_file: Path,
    target_file: Path,
    log_file: Path,
    spatch_options: Optional[Iterable[str]] = None,
) -> None:
    _which_or_error("spatch")
    parsing_hacks = (Path(__file__).resolve().parent.parent / "parsing_hacks.h").resolve()
    cmd = [
        "spatch",
        "--in-place",
        "--macro-file-builtins",
        str(parsing_hacks),
        *(spatch_options or ()),
        "--sp-file",
        str(cocci_file),
        str(target_file),
    ]
    before = target_file.read_bytes()
    cp = run(
        cmd
    )
    if cp.returncode == 0:
        after = target_file.read_bytes()
        if after != before:
            normalize_file(target_file)
    log_file.write_bytes(
        b"CMD: " + " ".join(cmd).encode()
        + b"\n\nSTDOUT:\n"
        + cp.stdout
        + b"\n\nSTDERR:\n"
        + cp.stderr
    )
    if cp.returncode != 0:
        raise FatalError(f"spatch failed (see log): {log_file}")


def diff_u_w_b(a: Path, b: Path) -> tuple[bool, str]:
    _which_or_error("diff")
    cp = run(["diff", "-u", "-w", "-B", str(a), str(b)])
    if cp.returncode == 0:
        return True, ""
    if cp.returncode == 1:
        return False, cp.stdout.decode(errors="replace")
    raise FatalError(f"diff failed:\n{cp.stderr.decode(errors='replace')}")
