// Recognize the printf-style comma (`,`) thousands-separator flag in the
// DebugLib format-string parsers that build a BASE_LIST from a VA_LIST.
//
//   if ((*Format == '.') || (*Format == '-') || (*Format == '+') || (*Format == ' '))
// ->if ((*Format == '.') || (*Format == '-') || (*Format == '+') || (*Format == ' ') || (*Format == ','))
//
// The "flag characters are omitted" test in DebugPrintMarker /
// VaListToBaseList skips over the flag/width run of a `%` conversion but did
// not list the comma flag, so a format such as "%,ld" desynchronized the
// argument walk and corrupted the packed BASE_LIST.  Appending the comma
// disjunct aligns these parsers with BasePrintLib.
//
// Scope / safety:
//   - The rule is anchored on the exact four-character disjunction
//     '.' / '-' / '+' / ' ', which is the printf flag-parsing idiom; a bare
//     "add a ',' comparison" rule keyed on any single character would be
//     meaningless.
//   - The disjunction must be the ENTIRE `if` condition (not merely a
//     sub-expression).  Without the `if` anchor the four-disjunct chain also
//     appears as the left operand of an already-fixed five-disjunct condition,
//     and the rule would append a duplicate `,` there.  Anchoring keeps the
//     rewrite idempotent and leaves a shorter subset chain (e.g. missing the
//     space flag) or an already-comma chain untouched.
//   - The comparison operand is a metavariable, so the rule also covers other
//     DebugLib implementations regardless of how the format cursor is spelled;
//     the sibling `switch`-based parser in DxePrintLibPrint2Protocol and the
//     comma-emitting code in BasePrintLib are a different shape and are not
//     matched.
// https://github.com/tianocore/edk2/commit/4d0946c082dfaf1556a2ecb0fd496d27acb8cecd
// https://github.com/tianocore/edk2/commit/a7c119589fe2c5829011e3bbe717285f2ff2ab28

@comma_flag@
expression E;
statement S;
@@
  if (
     (E == '.') || (E == '-') || (E == '+') || (E == ' ')
+    || (E == ',')
     )
  S
