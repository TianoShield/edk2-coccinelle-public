from __future__ import annotations

import sys

from . import compile_commands, line_endings
from .errors import FatalError


def _print_help() -> None:
    print(
        "usage: python3 -m run_spatch <command> [args]\n"
        "\n"
        "commands:\n"
        "  compile-commands  run spatch over .c and .h files using compile_commands metadata\n"
        "  fix-line-endings  normalize stray LF CR LF byte sequences\n"
    )


def main(argv: list[str]) -> int:
    if not argv or argv[0] in {"-h", "--help"}:
        _print_help()
        return 0

    command, args = argv[0], argv[1:]
    try:
        if command == "compile-commands":
            return compile_commands.main(args)
        if command == "fix-line-endings":
            return line_endings.main(args)
    except FatalError as exc:
        sys.stderr.write(f"FATAL: {exc}\n")
        return 2
    except FileNotFoundError as exc:
        sys.stderr.write(f"error: {exc}\n")
        return 2

    sys.stderr.write(f"error: unknown command: {command}\n")
    _print_help()
    return 2
