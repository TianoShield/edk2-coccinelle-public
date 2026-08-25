## Semantic Patches

Coccinelle is a program matching and transformation engine which provides the language SmPL (Semantic Patch Language) for specifying desired matches and transformations in C code.

---

## What Makes a Good Coccinelle Candidate?
- Must be C codebase.
- Well-defined before/after pattern. The transformation can be expressed as a clear match-and-replace rule. 
- High Repetitiveness and scale. The same structural change appears multiple times -- across files or repeatedly within one file. The repeated edits should mostly follow the same shape with little variation.
- API migration and collateral evolution. Renaming calls, adding arguments, switching to helper macros, or replacing old access patterns are good fits.

---

## What It's *Not* Great For
- Non-C code.
- One-off Fixes: change exists in exactly one function in one file and is specific to that context.
- Large refactors with only a small Coccinelle-friendly subpattern.
- Changes that need case-by-case reasoning, e.g. conditionals that vary per call site or edits that differ meaningfully from instance to instance.
- Pure style and formatting changes.
- Include-guard replacement and similar whole-file preprocessor structural changes. Replacing `#ifndef GUARD / #define GUARD ... #endif` with `#pragma once` requires matching a coordinated pair at the top and bottom of a file as a unit — SmPL has no construct for this. Use sed/awk/Python instead.

---

In short: Coccinelle is ideal for systematic, structural C code transformations — the kind where the change can be fully described as a pattern rather than decided case by case.
