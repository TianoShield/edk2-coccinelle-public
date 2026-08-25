// Generality + adversarial fixture for the DebugLib comma-flag rule.
//
//  - FlagParseGeneral   : the exact four-flag test with a differently spelled
//                         operand -> the comma disjunct is appended.
//  - FlagParseSubset    : only three flag characters -> left unchanged.
//  - FlagParseHasComma  : the comma flag is already present -> left unchanged
//                         (the rewrite must not duplicate it).

typedef unsigned short CHAR16;

VOID
FlagParseGeneral (
  IN CONST CHAR16  *Fmt
  )
{
  for ( ; *Fmt != '\0'; Fmt++) {
    if ((*Fmt == '.') || (*Fmt == '-') || (*Fmt == '+') || (*Fmt == ' ') || (*Fmt == ',')) {
      continue;
    }
  }
}

VOID
FlagParseSubset (
  IN CONST CHAR8  *Format
  )
{
  for ( ; *Format != '\0'; Format++) {
    if ((*Format == '.') || (*Format == '-') || (*Format == '+')) {
      continue;
    }
  }
}

VOID
FlagParseHasComma (
  IN CONST CHAR8  *Format
  )
{
  for ( ; *Format != '\0'; Format++) {
    if ((*Format == '.') || (*Format == '-') || (*Format == '+') || (*Format == ' ') || (*Format == ',')) {
      continue;
    }
  }
}
