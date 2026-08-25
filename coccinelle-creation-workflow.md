# Workflow for Creating or Refining a Coccinelle Semantic Patch

Create or refine a Coccinelle semantic patch by extracting one reusable transformation pattern from a specific commit. Replaying the commit is a validation technique, not the goal; the goal is a generic rule that rewrites the intended pattern and nothing else.

## Multi-step workflow

Follow this workflow in order. Do not skip a step unless it is clearly inapplicable. Each step has an exit condition. If the exit condition is not met, keep working within that step or return to the indicated earlier step.

### Summary

1. Inspect the target commit, identify the reusable transformation pattern, ask the user for clarification when needed, and create a concrete plan for the `.cocci` script and tests.
2. Implement the plan and make all tests pass
3. Simplify the `.cocci` script to the smallest readable form while preserving behavior.
4. Check success criteria and iterate if needed

### Step 1: Inspect, clarify, and plan before writing the rule

Before creating or editing any `.cocci` rule or test, inspect the target commit and define the intended transformation precisely.

#### 1. Inspect the commit
Read both the commit message and the full diff with whole-function context:

```sh
git show --function-context <commit>
```

Use this inspection to understand the semantic intent of the change, not just the textual diff.
Do not let comment-only or formatting-only changes define the selected pattern; record them as out-of-scope when they appear in the target diff.

#### 2. Choose exactly one transformation pattern

Identify one coherent transformation pattern to implement.
If the commit mixes multiple independent patterns, handle one pattern at a time, split unrelated patterns into separate rules or follow-up work, and do not encode unrelated edits in the current rule.
Describe the intended pattern in code terms.

Example: `replace allocation assertion with EFI_OUT_OF_RESOURCES return in EFI_STATUS functions`

#### 3. Classify the commit diff

Separate the commit diff into four categories.

##### A. In-scope transformation edits

Edits the `.cocci` rule is expected to reproduce. These are the actual before/after code changes that express the selected transformation pattern.

```diff
- ASSERT (Ptr != NULL);
+ if (Ptr == NULL) {
+   return EFI_OUT_OF_RESOURCES;
+ }
```

Ask: Would a correct rule intentionally produce this edit? If yes, it belongs here.

##### B. Out-of-scope commit edits

Edits that appear in the same commit but should not be reproduced by this rule.
These may be real code changes, but they belong to a different transformation pattern or are incidental cleanup.

Examples: comment changes, formatting-only changes, nearby unrelated renames, independent API migrations.

Ask: Would reproducing this edit require a different rule or a different intent? If yes, it belongs here.

##### C. Required safety context

Context that may not be edited, but must be matched or constrained so the transformation is correct.
This is semantic context needed to avoid unsafe or overbroad matches.

Examples: the return type of the enclosing function, the type of a struct or protocol pointer used in a member access, the argument position in a function call, whether the checked value comes from an allocation, or whether control flow makes insertion/replacement safe.

Ask: Could the rule become semantically incorrect if it ignored this context? If yes, it belongs here.

##### D. Incidental context to ignore

Code that appears near the target edit but is not semantically required for the transformation. This context should usually be replaced with dots (`...`).

Examples: nearby assignments, unrelated function calls or arguments, local variable use not related to the pattern, control flow that does not affect correctness, or neighboring code included only because of git diff context.

Ask: Did this code appear only because it was nearby in the diff? If yes, it belongs here.

#### 4. Define the intended abstraction level

Decide which parts of the transformation are fixed and which parts should be generalized.
Clarify:
- Which code fragments should become metavariables or dots?
- Which code fragments should stay concrete?
- Which surrounding context is semantically required?
- Which additional matching instances outside the commit should be considered valid?

A good rule should **generalize the intended pattern**, not blindly replay the commit.

#### 5. Identify ambiguity before implementation

Ask the user for clarification when the intended abstraction level is unclear.
If clarification is needed but unavailable, state the assumption that will be used before implementing the rule.

#### 6. Create an implementation and validation plan

Before editing files, create a short implementation and validation plan.
The plan must include: intended transformation pattern, explicit out-of-scope edits, safety constraints to encode in SmPL, proposed `.cocci` file name, proposed manifest name, git-backed replay cases covering the relevant commit hunks, synthetic fixture cases needed to test generality if any, commands to run for validation, and expected validation outcome.

### Example Plan

At the end of this step, produce a concise summary.

```text
Selected pattern:
Replace ASSERT(ptr != NULL) after allocation with an EFI_OUT_OF_RESOURCES return.

In-scope edits:
- ASSERT(ptr != NULL) converted to explicit NULL check.
- Replacement returns EFI_OUT_OF_RESOURCES.

Out-of-scope edits:
- Comment updates.
- Formatting changes.

Required safety context:
- Enclosing function must return EFI_STATUS.
- Checked expression must be the allocated pointer.
- Replacement must occur before the pointer is dereferenced.

Rule abstraction:
- Pointer expression becomes a metavariable.
- Return constant remains EFI_OUT_OF_RESOURCES.
- Allocation function may be concrete unless broader scope is requested.
- Arguments of allocation function become `...`.
- Statements unrelated to allocation become `...`.

Open ambiguity:
- Should this match all allocation APIs or only the allocation function used in the commit?

Validation plan:
- Add git-backed replay cases for the relevant commit hunks.
- Add a positive synthetic fixture for the generalized pattern.
- Add a negative synthetic fixture for non-EFI_STATUS functions.
- Commands to run for validation
```

**Exit condition:** The transformation pattern, scope, safety constraints, test strategy, and implementation plan are clear. If clarification was needed, the user has answered or the plan states the assumptions being used.

---

### Step 2: Implement the plan and make all tests pass

Implement the planned `.cocci` script and its tests.

1. Create or update the pinned test manifest.
   - Use `cocci-scripts/tests/manifests/manifest.schema.json` for the machine-readable format.
   - Use `cocci-scripts/tests/manifests/README.md` for the prose guide.
   - Use `gitRevision` in both `beforeSource` and `afterSource` whenever the expected result exists in edk2 history.
   - When a git-backed case uses `ranges`, include complete function bodies.
   - Add fixture-backed cases only for reduced or synthetic examples that test generalization beyond the original commit, or when the expected result does not exist in real edk2 history.
   - Cover every hunk in the target commit that belongs to the chosen transformation pattern.
   - Do not force unrelated hunks into the same rule.

2. Create or update the `.cocci` program.
   - Put the file under `cocci-scripts/`.
   - Start the file with a comment describing the intended transformation, scope limits, and safety constraints.
   - Implement the planned transformation, safety context, and abstractions.
   - Use standard Coccinelle isomorphisms rather than manually enumerating equivalent forms.
   - Prefer the smallest readable rule set that correctly captures the planned pattern.

3. Run the required checks.
   - If any manifest changed, validate the manifest schema.
   - If a `.cocci` file changed, run the corresponding manifest.
   - If a fixture changed, inspect the relevant normalized case diff.

4. Fix failures.
   - If an intended instance is missed, broaden the rule.
   - If the rule rewrites unrelated or semantically wrong code, tighten the rule.
   - If an extra rewrite is correct and belongs to the same pattern, fix or expand the tests.
   - Do not replace a git-backed replay case with fixture output merely to make an over-broad or over-restricted rule pass.

**Exit condition:** The planned `.cocci` script and test manifest exist, all relevant tests pass, and the result implements the planned transformation.

---

### Step 3: Simplify the `.cocci` script to the smallest readable form

Once all tests pass, simplify the semantic patch to the smallest readable form that preserves the intended behavior.
See `coccinelle-simplification-guide.md` for simplification guidance and examples.
Re-run all relevant tests after each simplification round.

**Exit condition:** The `.cocci` script is small and readable, no parts can be removed or merged without changing intended behavior, and all tests still pass.

---

### Step 4: Check success criteria and iterate if needed

Check the completed rule against the success criteria.

Success criteria:

- A `.cocci` file exists under `cocci-scripts/` for the target edk2 commit.
- A pinned manifest exists under `cocci-scripts/tests/manifests/` for the chosen transformation pattern.
- Coverage: all hunks in the target edk2 commit diff that are relevant to that pattern are covered by git-backed replay or justified synthetic fixture cases.
- Soundness: the semantic patch only rewrites the intended transformation pattern.
- Generality: the semantic patch is not overfit to the original diff hunks, and test cases demonstrate the rule’s generality beyond the original commit.
- Simplicity: the semantic patch is the smallest readable rule set that preserves intended behavior.
- All required checks pass.

If any criterion fails:

- Return to **Step 2** if the rule, manifest, fixtures, or tests need functional changes.
- Return to **Step 3** if the only remaining issue is concision or unnecessary complexity.
- Return to **Step 1** if the failure reveals ambiguity in the intended transformation pattern or scope.

**Exit condition:** All success criteria are met.
