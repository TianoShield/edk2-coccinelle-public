# Repository Guidelines

## Project Structure & Module Organization
`cocci-scripts/` contains the semantic patches (`*.cocci`) that generalize edk2 code changes. Test infrastructure lives in `cocci-scripts/tests/`: `run.py` executes manifest-driven tests, `testlib.py` holds shared helpers, `verify_manifest_schema.py` validates manifest JSON, and `show_manifest_case_diff.py` prints before/after diffs for one case. Manifest files live in `cocci-scripts/tests/manifests/`; use `cocci-scripts/tests/manifests/manifest.schema.json` for the machine-readable format and `cocci-scripts/tests/manifests/README.md` for the prose guide. Checked-in expected snippets live under `cocci-scripts/tests/fixtures/<commit>/`. Root-level shell helpers such as `check-commit-touches-c.sh` support commit triage.
Must follow `coccinelle-creation-workflow.md` when creating or refining a coccinelle semantic patch.

## Build, Test, and Development Commands
- `python3 cocci-scripts/tests/verify_manifest_schema.py`: validate all manifests against `manifest.schema.json`.
- `python3 cocci-scripts/tests/run.py --repo ../edk2 --manifest cocci-scripts/tests/manifests/94a50ef5.json`: run one manifest against a local edk2 checkout.
- `python3 cocci-scripts/tests/show_manifest_case_diff.py --repo ../edk2 --manifest cocci-scripts/tests/manifests/300dada916.json --case fixture-bootmanager-menu`: inspect the normalized before/after diff for a single case.
- `python3 -m py_compile cocci-scripts/tests/*.py`: quick syntax check for test utilities.

All test commands expect a usable edk2 tree at `../edk2` or via `EDK2_REPO`.

## Coding Style & Naming Conventions
Use 4-space indentation in Python and preserve existing edk2/Coccinelle formatting conventions. Keep JSON manifest keys in camelCase (`cocciFile`, `beforeSource`, `gitRevision`). Name manifest cases descriptively, for example `replay-commit-ppi` or `fixture-bootmanager-menu`.

## Testing Guidelines
Every semantic patch change should add or update manifest coverage. Prefer replaying real edk2 history with `gitRevision` in both `beforeSource` and `afterSource`. Use `fixturePath` only when the expected output is intentionally synthetic, reduced, or not available in real edk2 history. Keep fixture cases focused on generalization beyond the original commit, not on masking replay mismatches.

## Commit & Pull Request Guidelines
Follow the recent subject style: scoped, imperative summaries such as `tests: add manifest case diff helper` or `cocci: tighten 94a50ef5 IS_ALIGNED matching`. Keep commits narrow: schema changes, manifest refactors, and `.cocci` behavior changes should be separable when possible. Commit messages should include a body that describes the substantive behavior changes, not just restate the subject. Pull requests should explain the target commit or pattern, list the manifests exercised, and call out any fixture regeneration.

## Coccinelle Documentation
- Coccinelle repository is at `COCCINELLE_ROOT=/evaldisk/ming/tianoshield/coccinelle/`.
- The Coccinelle manual is at `$COCCINELLE_ROOT/docs/manual/`.
- The SmPL grammar is at `$COCCINELLE_ROOT/docs/manual/cocci_syntax.tex`.
- Standard isomorphisms are at `$COCCINELLE_ROOT/standard.iso`.
- Command line options of spatch are at `$COCCINELLE_ROOT/docs/manual/spatch_options.tex`.
- Use `spatch --debug` when matching behavior is unclear.
