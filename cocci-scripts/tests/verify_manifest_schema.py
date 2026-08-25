#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Iterable

import jsonschema


def _load_json(path: Path) -> object:
    return json.loads(path.read_text(encoding="utf-8"))


def _iter_manifests(manifests_dir: Path) -> Iterable[Path]:
    return sorted(path for path in manifests_dir.glob("*.json") if path.name != "manifest.schema.json")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--schema",
        default="cocci-scripts/tests/manifests/manifest.schema.json",
        help="path to the manifest JSON Schema",
    )
    parser.add_argument(
        "--manifests-dir",
        default="cocci-scripts/tests/manifests",
        help="directory containing manifest JSON files",
    )
    args = parser.parse_args(argv)

    schema_path = Path(args.schema).resolve()
    manifests_dir = Path(args.manifests_dir).resolve()
    schema = _load_json(schema_path)
    validator = jsonschema.Draft202012Validator(schema)

    unexpected_failures: list[str] = []

    for manifest_path in _iter_manifests(manifests_dir):
        errors = sorted(validator.iter_errors(_load_json(manifest_path)), key=lambda err: list(err.path))
        manifest_name = manifest_path.name
        if errors:
            print(f"FAIL: {manifest_name}")
            print(f"  {errors[0].message}")
            unexpected_failures.append(manifest_name)
        else:
            print(f"ok: {manifest_name}")

    if unexpected_failures:
        print("schema failures:")
        for name in unexpected_failures:
            print(f"  {name}")

    return 1 if unexpected_failures else 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
