#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Optional

from testlib import FatalError, apply_spatch, diff_u_w_b, get_uncrustify, git_show_file, resolve_edk2_repo, uncrustify_files


class ManifestError(FatalError):
    pass


class CaseFailure(RuntimeError):
    pass


def _iter_manifest_paths(manifests_dir: Path) -> list[Path]:
    return sorted(path for path in manifests_dir.glob("*.json") if path.name != "manifest.schema.json")


@dataclass(frozen=True)
class LineRange:
    start_line: int
    end_line: int


@dataclass(frozen=True)
class SourceSpec:
    git_revision: Optional[str] = None
    fixture_path: Optional[Path] = None
    ranges: tuple[LineRange, ...] = ()


@dataclass(frozen=True)
class Case:
    name: str
    before: SourceSpec
    after: SourceSpec


@dataclass(frozen=True)
class Manifest:
    id: str
    cocci_file: Path
    spatch_options: tuple[str, ...]
    cases: list[Case]
    path: Path


def _load_manifest(path: Path) -> dict[str, Any]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as e:
        raise ManifestError(f"failed to parse manifest {path}: {e}") from e
    if not isinstance(data, dict):
        raise ManifestError(f"manifest root must be an object: {path}")
    return data


def _parse_case(obj: dict[str, Any], *, manifest_path: Path, repo_root: Path) -> Case:
    def req_str(key: str) -> str:
        v = obj.get(key)
        if not isinstance(v, str) or not v:
            raise ManifestError(f"{manifest_path}: case missing/invalid string field: {key}")
        return v

    name = req_str("name")

    def parse_ranges(raw: Any, *, field_name: str) -> tuple[LineRange, ...]:
        if raw is None:
            return ()
        if not isinstance(raw, list) or not raw:
            raise ManifestError(f"{manifest_path}: case {name}: {field_name} must be a non-empty list")

        ranges: list[LineRange] = []
        for idx, entry in enumerate(raw, start=1):
            if not isinstance(entry, dict):
                raise ManifestError(f"{manifest_path}: case {name}: {field_name}[{idx}] must be an object")

            start_line = entry.get("startLine")
            end_line = entry.get("endLine")
            if not isinstance(start_line, int) or isinstance(start_line, bool) or start_line <= 0:
                raise ManifestError(f"{manifest_path}: case {name}: {field_name}[{idx}].startLine must be a positive integer")
            if not isinstance(end_line, int) or isinstance(end_line, bool) or end_line <= 0:
                raise ManifestError(f"{manifest_path}: case {name}: {field_name}[{idx}].endLine must be a positive integer")
            if end_line < start_line:
                raise ManifestError(f"{manifest_path}: case {name}: {field_name}[{idx}] has endLine before startLine")

            ranges.append(LineRange(start_line=start_line, end_line=end_line))

        sorted_ranges = sorted(ranges, key=lambda line_range: (line_range.start_line, line_range.end_line))
        for prev_range, next_range in zip(sorted_ranges, sorted_ranges[1:]):
            if next_range.start_line <= prev_range.end_line:
                raise ManifestError(f"{manifest_path}: case {name}: {field_name} must be non-overlapping")

        return tuple(ranges)

    def resolve_fixture_path(raw: str, *, field_name: str) -> Path:
        resolved = Path(raw)
        if not resolved.is_absolute():
            resolved = (repo_root / resolved).resolve()
        if not resolved.is_file():
            raise ManifestError(f"{manifest_path}: case {name}: {field_name} not found: {resolved}")
        return resolved

    def parse_source_spec(key: str) -> SourceSpec:
        raw = obj.get(key)
        if not isinstance(raw, dict):
            raise ManifestError(f"{manifest_path}: case {name}: {key} must be an object")

        git_revision = raw.get("gitRevision")
        fixture_path_raw = raw.get("fixturePath")
        if (git_revision is None) == (fixture_path_raw is None):
            raise ManifestError(
                f"{manifest_path}: case {name}: {key} must specify exactly one of gitRevision or fixturePath"
            )

        if git_revision is not None:
            if not isinstance(git_revision, str) or not git_revision:
                raise ManifestError(f"{manifest_path}: case {name}: {key}.gitRevision must be a non-empty string")
            rev, sep, relpath = git_revision.partition(":")
            if not sep or not rev or not relpath:
                raise ManifestError(
                    f"{manifest_path}: case {name}: {key}.gitRevision must have the form <rev>:<path>"
                )
            fixture_path = None
        else:
            if not isinstance(fixture_path_raw, str) or not fixture_path_raw:
                raise ManifestError(f"{manifest_path}: case {name}: {key}.fixturePath must be a non-empty string")
            git_revision = None
            fixture_path = resolve_fixture_path(fixture_path_raw, field_name=f"{key}.fixturePath")

        ranges = parse_ranges(raw.get("ranges"), field_name=f"{key}.ranges")
        return SourceSpec(git_revision=git_revision, fixture_path=fixture_path, ranges=ranges)

    return Case(name=name, before=parse_source_spec("beforeSource"), after=parse_source_spec("afterSource"))


def _write_bytes(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def _slice_source(*, source: bytes, ranges: tuple[LineRange, ...], source_desc: str) -> bytes:
    if not ranges:
        return source

    lines = source.splitlines(keepends=True)
    total_lines = len(lines)
    chunks: list[bytes] = []
    for line_range in ranges:
        if line_range.end_line > total_lines:
            raise ManifestError(
                f"{source_desc}: requested lines {line_range.start_line}-{line_range.end_line}, "
                f"but file has only {total_lines} lines"
            )

        chunk = b"".join(lines[line_range.start_line - 1 : line_range.end_line])
        if chunk and not chunk.endswith(b"\n"):
            chunk += b"\n"
        chunks.append(chunk)

    return b"\n".join(chunks)


def _load_source(
    *,
    edk2_repo: Path,
    source_spec: SourceSpec,
    source_desc: str,
) -> bytes:
    if source_spec.git_revision is not None:
        rev, _, path = source_spec.git_revision.partition(":")
        source = git_show_file(edk2_repo, rev, path)
    elif source_spec.fixture_path is not None:
        source = source_spec.fixture_path.read_bytes()
    else:
        raise ManifestError(f"{source_desc}: source spec is missing both gitRevision and fixturePath")

    return _slice_source(source=source, ranges=source_spec.ranges, source_desc=source_desc)


def _run_case(
    *,
    case: Case,
    manifest_id: str,
    edk2_repo: Path,
    cocci_file: Path,
    spatch_options: tuple[str, ...],
    uncrustify_cfg,
    tmpdir: Path,
) -> None:
    safe_name = case.name.replace("/", "_")
    before = tmpdir / f"{manifest_id}.{safe_name}.before.c"
    expected = tmpdir / f"{manifest_id}.{safe_name}.expected.c"
    actual = tmpdir / f"{manifest_id}.{safe_name}.actual.c"
    log = tmpdir / f"{manifest_id}.{safe_name}.spatch.log"

    _write_bytes(
        before,
        _load_source(
            edk2_repo=edk2_repo,
            source_spec=case.before,
            source_desc=f"{manifest_id}:{case.name}: before source",
        ),
    )
    _write_bytes(
        expected,
        _load_source(
            edk2_repo=edk2_repo,
            source_spec=case.after,
            source_desc=f"{manifest_id}:{case.name}: after source",
        ),
    )
    actual.write_bytes(before.read_bytes())

    apply_spatch(
        cocci_file=cocci_file,
        target_file=actual,
        log_file=log,
        spatch_options=spatch_options,
    )
    uncrustify_files(uncrustify_cfg, [expected, actual])

    same, diff = diff_u_w_b(expected, actual)
    if not same:
        raise CaseFailure(diff)


def _load_and_validate_manifest(*, manifest_path: Path, repo_root: Path) -> Manifest:
    manifest = _load_manifest(manifest_path)
    manifest_id = manifest.get("id")
    if not isinstance(manifest_id, str) or not manifest_id:
        raise ManifestError(f"{manifest_path}: missing/invalid id")

    cocci_rel = manifest.get("cocciFile")
    if not isinstance(cocci_rel, str) or not cocci_rel:
        raise ManifestError(f"{manifest_path}: missing/invalid cocciFile")
    cocci_file = (repo_root / cocci_rel).resolve()
    if not cocci_file.is_file():
        raise ManifestError(f"{manifest_path}: cocci_file not found: {cocci_file}")

    spatch_options_obj = manifest.get("spatchOptions", [])
    if not isinstance(spatch_options_obj, list) or not all(
        isinstance(option, str) and option for option in spatch_options_obj
    ):
        raise ManifestError(f"{manifest_path}: spatchOptions must be a list of non-empty strings")
    spatch_options = tuple(spatch_options_obj)

    cases_obj = manifest.get("cases")
    if not isinstance(cases_obj, list) or not cases_obj:
        raise ManifestError(f"{manifest_path}: missing/invalid cases list")

    cases: list[Case] = []
    for raw_case in cases_obj:
        if not isinstance(raw_case, dict):
            raise ManifestError(f"{manifest_path}: all cases must be objects")
        cases.append(_parse_case(raw_case, manifest_path=manifest_path, repo_root=repo_root))

    return Manifest(
        id=manifest_id,
        cocci_file=cocci_file,
        spatch_options=spatch_options,
        cases=cases,
        path=manifest_path,
    )


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", help="path to edk2 git repo (or set EDK2_REPO)")
    ap.add_argument("--manifest", action="append", help="manifest file to run (repeatable)")
    ap.add_argument("--keep-tmp", action="store_true", help="do not delete temp directory")
    args = ap.parse_args(argv)

    script_dir = Path(__file__).resolve().parent
    repo_root = (script_dir / ".." / "..").resolve()

    manifests: list[Path] = []
    if args.manifest:
        manifests = [Path(m).resolve() for m in args.manifest]
    else:
        manifests = _iter_manifest_paths(script_dir / "manifests")
    if not manifests:
        raise ManifestError("no manifests found (pass --manifest or add files under cocci-scripts/tests/manifests/)")

    # Fatal: stop immediately if a manifest can't be parsed/validated.
    parsed_manifests = [_load_and_validate_manifest(manifest_path=m, repo_root=repo_root) for m in manifests]

    edk2_repo = resolve_edk2_repo(args.repo, script_root=script_dir)
    uncrustify_cfg = get_uncrustify(edk2_repo)

    if args.keep_tmp:
        tmpdir = Path(tempfile.mkdtemp(prefix="edk2-cocci-tests-"))
        tmp_cm = None
    else:
        tmp_cm = tempfile.TemporaryDirectory(prefix="edk2-cocci-tests-")
        tmpdir = Path(tmp_cm.name)

    total_cases = 0
    failed_cases = 0
    try:
        for manifest in parsed_manifests:
            for case in manifest.cases:
                total_cases += 1
                case_id = f"{manifest.id}:{case.name}"
                try:
                    _run_case(
                        case=case,
                        manifest_id=manifest.id,
                        edk2_repo=edk2_repo,
                        cocci_file=manifest.cocci_file,
                        spatch_options=manifest.spatch_options,
                        uncrustify_cfg=uncrustify_cfg,
                        tmpdir=tmpdir,
                    )
                    print(f"ok: {case_id}")
                except CaseFailure as e:
                    failed_cases += 1
                    print(f"FAIL: {case_id}")
                    sys.stderr.write(f"FAIL: {case_id}\n{e}\n")
                except FatalError as e:
                    sys.stderr.write(f"FATAL: {case_id}: {e}\n")
                    return 2

        if failed_cases:
            passed_cases = total_cases - failed_cases
            sys.stderr.write(
                f"summary: {total_cases} cases, {passed_cases} passed, {failed_cases} failed\n"
            )
            return 1
        return 0
    finally:
        if tmp_cm is not None:
            tmp_cm.cleanup()
        elif args.keep_tmp:
            print(f"kept temp dir: {tmpdir}")


if __name__ == "__main__":
    try:
        raise SystemExit(main(sys.argv[1:]))
    except ManifestError as e:
        sys.stderr.write(f"FATAL: {e}\n")
        raise SystemExit(2)
    except FatalError as e:
        sys.stderr.write(f"FATAL: {e}\n")
        raise SystemExit(2)
    except CaseFailure as e:
        sys.stderr.write(f"FAIL: {e}\n")
        raise SystemExit(1)
