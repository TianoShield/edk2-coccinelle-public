from __future__ import annotations

import argparse
import re
from pathlib import Path


_PATTERN = re.compile(rb"(?<!\r)\n\r\n")


def normalize_file(path: Path, *, dry_run: bool = False) -> int:
    data = path.read_bytes()
    new_data, count = _PATTERN.subn(b"\r\n", data)
    if count and not dry_run:
        path.write_bytes(new_data)
    return count


def _iter_targets(path: Path) -> list[Path]:
    if path.is_file():
        return [path]
    if path.is_dir():
        return sorted((child for child in path.rglob("*") if child.is_file()), key=lambda p: str(p))
    raise FileNotFoundError(f"file or directory not found: {path}")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        prog="python3 -m run_spatch fix-line-endings",
        description="Replace stray LF CR LF sequences with CR LF.",
    )
    parser.add_argument(
        "targets",
        nargs="+",
        help="files to normalize, or directory trees containing files to normalize",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="report the number of replacements without modifying the file",
    )
    args = parser.parse_args(argv)

    paths: list[Path] = []
    for target in args.targets:
        paths.extend(_iter_targets(Path(target)))

    for path in paths:
        count = normalize_file(path, dry_run=args.dry_run)
        if args.dry_run:
            print(f"{path}: {count} replacement(s) would be made")
        else:
            print(f"{path}: {count} replacement(s) made")
    return 0
