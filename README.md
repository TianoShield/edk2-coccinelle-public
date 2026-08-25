# edk2-coccinelle

This repository explores a semi-automatic, human-in-the-loop agentic workflow
for learning reusable [Coccinelle](https://coccinelle.gitlabpages.inria.fr/website/)
semantic patches (SmPL) from concrete edk2 code patches.

The current corpus covers:

- 20 historical edk2 CVEs, represented by 19 semantic patches (one patch covers
  CVE-2023-45232 and CVE-2023-45233).
- 11 collateral-evolution changes, represented by 13 semantic-patch
  implementations, including alternate variants for two changes.

## Practical impact

Semantic patches developed here generated collateral-evolution changes that
were reviewed and merged upstream into edk2:

- [#12571: Global: Replace manual alignment checks with helper macros](https://github.com/tianocore/edk2/pull/12571)
- [#12558: MdeModulePkg/ArmFfaLib: Use EFI_PAGES_TO_SIZE](https://github.com/tianocore/edk2/pull/12558)
- [#12557: MdeModulePkg,UefiPayloadPkg: Fix incorrect EfiPciWidth* enum literals](https://github.com/tianocore/edk2/pull/12557)
- [#12552: ArmPkg/Driver: Remove the stale mPartId and duplicate ArmFfaLibRxRelease](https://github.com/tianocore/edk2/pull/12552)

## Workflow

The agent and a human reviewer identify one reusable transformation in an edk2
commit, define its scope and safety constraints, implement the SmPL rule, and
validate it by replaying edk2 history and testing additional fixtures. See
[coccinelle-creation-workflow.md](coccinelle-creation-workflow.md) for the full
workflow.

## Repository layout

- `cocci-scripts/`: collateral-evolution semantic patches.
- `cocci-scripts/past-cves/`: semantic patches learned from published CVE fixes.
- `cocci-scripts/tests/`: manifest-driven replay and fixture tests.
- `cocci-candidate-assessment/`: agent assessments of candidate edk2 commits.

## Test

Requirements are Python 3.10+, `spatch`, and a local edk2 checkout (including
BaseTools' `uncrustify`). Install the Python utilities and validate the corpus:

```sh
python3 -m pip install -e '.[test]'
python3 cocci-scripts/tests/verify_manifest_schema.py
python3 cocci-scripts/tests/run.py --repo ../edk2
```

Pass `--manifest cocci-scripts/tests/manifests/<name>.json` to run one semantic
patch test set. The edk2 checkout can also be selected with `EDK2_REPO`.
