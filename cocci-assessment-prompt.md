## Overview

Assess a range of edk2 commits as Coccinelle semantic patch candidates. Results go in
`cocci-candidate-assessment/<full-hash>.json`, one file per commit. After all commits are
assessed, run `combine-cocci-candidate-assessment.sh` to produce a combined report sorted
in `git log` order.

## Step 1 — Build the work list

Collect the commits to assess (e.g. `git -C <edk2> log --format='%H %s' --since=<date>`).
Skip any commit that already has a result file.

## Step 2 — C-file gate

For each missing commit, run `check-commit-touches-c.sh <commit>`:
- Exit 0 (`touches_c=true`): commit touches `.c` or `.h` files → proceed to Step 3.
- Exit 1 (`touches_c=false`): no C files touched → write a `suitable: false` result
  immediately with reason "touches_c=false" and skip to the next commit. No subagent needed.

## Step 3 — Subagent assessment (C-touching commits)

Batch C-touching commits and launch subagents in parallel (≤20 concurrent). Each subagent:

1. Receives the full commit hash, subject, `git show <hash>` diff, and the subjects of the
   ±3 adjacent commits in the git log sequence (for series detection).
2. Embeds the full text of `cocci-candidate-criteria.md` in its prompt.
3. Assesses the commit and writes the result JSON file.

**Adjacent-commit grouping:** when neighboring commits share the same change pattern (same
collateral evolution applied across different packages), treat the whole group as one
conceptual commit when scoring repetitiveness. A commit that is 1-of-N in such a series
counts as highly repetitive even if its individual diff is small.

## Step 4 — Combine results

Run `./combine-cocci-candidate-assessment.sh` to merge all per-commit JSON files into
`cocci-candidate-assessment/combined.json`, sorted in `git log` order.

## JSON schema

```json
{
  "commit": "<40-char hash>",
  "short_commit": "<first 10 chars>",
  "subject": "<commit subject line>",
  "suitable": true,
  "reasons": ["reason 1", "reason 2"],
  "candidate_pattern": "Description of the SmPL-expressible transformation",
  "confidence": "high | medium | low"
}
```

`candidate_pattern` is `null` when `suitable` is `false`.
