# Manifest Format

`manifest.schema.json` defines the JSON format used by `cocci-scripts/tests/run.py`.

Each manifest describes one semantic patch test set:

- `id`: stable identifier for the manifest, typically the commit id being generalized.
- `cocciFile`: path to the `.cocci` file, relative to the repository root.
- `spatchOptions`: optional list of additional `spatch` command-line options for this manifest.
- `cases`: non-empty list of test cases.

Each case has:

- `name`: human-readable case name.
- `beforeSource`: source material that will be written to a temporary file, transformed by `spatch`, then normalized with `uncrustify`.
- `afterSource`: expected source material, also normalized with `uncrustify` before comparison.

## Source Objects

`beforeSource` and `afterSource` use the same shape. A source object must contain exactly one of:

- `gitRevision`: a string of the form `<git-revision>:<path>`.
- `fixturePath`: path to a checked-in fixture file, relative to the repository root unless already absolute.

It may also contain:

- `ranges`: non-empty list of line ranges to extract from the chosen source.

## Ranges

Each range object has:

- `startLine`: 1-based inclusive start line.
- `endLine`: 1-based inclusive end line.

When `ranges` is omitted, the entire source is used.

When `ranges` is present, `run.py`:

- extracts each requested slice in the order written in the manifest,
- requires the ranges to be non-overlapping,
- joins the extracted slices with a blank line.

## Minimal Example

```json
{
  "id": "94a50ef5",
  "cocciFile": "cocci-scripts/94a50ef5-is-aligned.cocci",
  "cases": [
    {
      "name": "replay-full-file",
      "beforeSource": {
        "gitRevision": "94a50ef5:UefiCpuPkg/PiSmmCpuDxeSmm/SmmCpuMemoryManagement.c"
      },
      "afterSource": {
        "gitRevision": "94a50ef5b1:UefiCpuPkg/PiSmmCpuDxeSmm/SmmCpuMemoryManagement.c"
      }
    },
    {
      "name": "fixture-expected-with-slice",
      "beforeSource": {
        "gitRevision": "b7a715f7c03c45c6b4575bf88596bfd79658b8ce:UefiCpuPkg/PiSmmCpuDxeSmm/SmmProfile.c",
        "ranges": [
          {
            "startLine": 419,
            "endLine": 590
          }
        ]
      },
      "afterSource": {
        "fixturePath": "cocci-scripts/tests/fixtures/94a50ef5/SmmProfile.after.c"
      }
    }
  ]
}
```

## Validation

To validate all manifests against the schema:

```bash
python3 cocci-scripts/tests/verify_manifest_schema.py
```
