from __future__ import annotations

import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

from .errors import FatalError


@dataclass(frozen=True)
class UncrustifyConfig:
    bin_path: Path
    cfg_path: Path


def _uncrustify_config_for_repo(edk2_repo: Path) -> UncrustifyConfig:
    return UncrustifyConfig(
        bin_path=(
            edk2_repo
            / ".pytool"
            / "Plugin"
            / "UncrustifyCheck"
            / "tianocore-uncrustify-release_extdep"
            / "Linux-x86"
            / "uncrustify"
        ),
        cfg_path=(
            edk2_repo
            / ".pytool"
            / "Plugin"
            / "UncrustifyCheck"
            / "uncrustify.cfg"
        ),
    )


def get_uncrustify(edk2_repo: Path) -> UncrustifyConfig:
    cfg = _uncrustify_config_for_repo(edk2_repo)
    if not cfg.bin_path.is_file() or not os.access(cfg.bin_path, os.X_OK):
        raise FatalError(f"uncrustify binary not found/executable: {cfg.bin_path}")
    if not cfg.cfg_path.is_file():
        raise FatalError(f"uncrustify config not found: {cfg.cfg_path}")
    return cfg


def uncrustify_files(cfg: UncrustifyConfig, files: Iterable[Path]) -> None:
    file_list = [str(p) for p in files]
    if not file_list:
        return

    stdin = ("\n".join(file_list) + "\n").encode()
    cp = subprocess.run(
        [
            str(cfg.bin_path),
            "-c",
            str(cfg.cfg_path),
            "-F",
            "-",
            "--replace",
            "--no-backup",
            "--if-changed",
        ],
        input=stdin,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if cp.returncode != 0:
        raise FatalError(
            "uncrustify failed\n"
            f"  bin: {cfg.bin_path}\n"
            f"  cfg: {cfg.cfg_path}\n"
            f"  stderr:\n{cp.stderr.decode(errors='replace')}"
        )
