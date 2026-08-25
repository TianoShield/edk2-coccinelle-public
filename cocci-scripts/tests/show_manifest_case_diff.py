#!/usr/bin/env python3

from __future__ import annotations

import argparse
import sys
import tempfile
from pathlib import Path

from run import ManifestError, _load_and_validate_manifest, _load_source, _write_bytes
from testlib import FatalError, diff_u_w_b, get_uncrustify, resolve_edk2_repo, uncrustify_files


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", help="path to edk2 git repo (or set EDK2_REPO)")
    parser.add_argument("--manifest", required=True, help="manifest JSON file")
    parser.add_argument("--case", required=True, help="case name in the manifest")
    args = parser.parse_args(argv)

    script_dir = Path(__file__).resolve().parent
    repo_root = (script_dir / ".." / "..").resolve()
    manifest_path = Path(args.manifest).resolve()

    manifest = _load_and_validate_manifest(manifest_path=manifest_path, repo_root=repo_root)
    case = next((entry for entry in manifest.cases if entry.name == args.case), None)
    if case is None:
        raise ManifestError(f"{manifest_path}: case not found: {args.case}")

    edk2_repo = resolve_edk2_repo(args.repo, script_root=script_dir)
    uncrustify_cfg = get_uncrustify(edk2_repo)

    with tempfile.TemporaryDirectory(prefix="edk2-cocci-case-diff-") as tmp:
        tmpdir = Path(tmp)
        safe_name = case.name.replace("/", "_")
        before_path = tmpdir / f"{manifest.id}.{safe_name}.before.c"
        after_path = tmpdir / f"{manifest.id}.{safe_name}.after.c"

        _write_bytes(
            before_path,
            _load_source(
                edk2_repo=edk2_repo,
                source_spec=case.before,
                source_desc=f"{manifest.id}:{case.name}: before source",
            ),
        )
        _write_bytes(
            after_path,
            _load_source(
                edk2_repo=edk2_repo,
                source_spec=case.after,
                source_desc=f"{manifest.id}:{case.name}: after source",
            ),
        )

        # Match the runner's normalization so the diff highlights semantic changes.
        uncrustify_files(uncrustify_cfg, [before_path, after_path])

        same, diff = diff_u_w_b(before_path, after_path)
        if same:
            print(f"no diff: {manifest.id}:{case.name}")
            return 0

        print(diff, end="")
        return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except FatalError as exc:
        sys.stderr.write(f"FATAL: {exc}\n")
        raise SystemExit(2)
