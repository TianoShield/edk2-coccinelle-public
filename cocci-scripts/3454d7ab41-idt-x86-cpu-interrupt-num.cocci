// Size x86 IDTs with the architectural X86_CPU_INTERRUPT_NUM (256,
// defined by 3ed9aceeb8 in MdeModulePkg CpuExceptionHandlerLib.h)
// instead of local per-module entry-count constants, generalizing the
// 256-IDT-descriptor series 3454d7ab41 (UefiCpuPkg SEC/PEI/SMM),
// 4512733ac8 (MdeModulePkg DxeIpl IA32), and 3bfe4fadff (OvmfPkg
// SEC/SMM).
//
//   #define CPU_INTERRUPT_NUM    256    (deleted)
//   #define CPU_EXCEPTION_NUM    32     (deleted)
//   #define SEC_IDT_ENTRY_COUNT  34     (deleted)
//   #define IDT_ENTRY_COUNT      32     (deleted)
//   every use of those names            -> X86_CPU_INTERRUPT_NUM
//   sizeof (IA32_IDT_GATE_DESCRIPTOR) * 32
//                                       -> sizeof (...) * X86_CPU_INTERRUPT_NUM
//
// Scope / safety:
//   - Renaming is anchored on the four exact constant names, in array
//     sizes, sizeof products, loop bounds, comparisons, initializers,
//     and call arguments alike.  No value-based rewrite of a bare
//     32/34/256 happens anywhere else.
//   - The one bare-literal rewrite is pinned to exactly 32 and to
//     multiplication with sizeof (IA32_IDT_GATE_DESCRIPTOR), the IDT
//     size idiom the series converts in SmmRelocationLib, CpuS3, and
//     PiSmmCpuCommon.  Deliberately different IDT sizes survive: the
//     DebugAgent 33-entry tables (sizeof (...) * 33), the S3 context
//     0x100 spelling, and descriptor *offsets* (3 * sizeof (...),
//     14 * sizeof (...)) are all left alone.
//   - The defines are matched with their exact original values, so a
//     hypothetical same-named constant with a different value is not
//     silently deleted.
//   - Out of scope (by design): the .nasm vector-table changes, comment
//     text updates, the #include <Library/CpuExceptionHandlerLib.h>
//     addition that provides the macro (4512733ac8 in DxeIpl.h), and
//     the InitializeCpuExceptionHandlers (NULL) call 3bfe4fadff adds in
//     OvmfPkg SEC.  Converted files must reach CpuExceptionHandlerLib.h
//     through their includes.
//   - Run with
//       --macro-file-builtins cocci-scripts/parsing_hacks.h
//       --include cocci-scripts/BaseLib.h --include-headers-for-types
//     (the manifest passes the last two via spatchOptions; see
//     cocci-scripts/BaseLib.h for why the pristine edk2 header cannot
//     be used directly).  Without the CONST macro hint
//     (OvmfPkg/Sec/SecMain.c) or the BaseLib.h SWITCH_STACK_ENTRY_POINT
//     typedef (DxeIplPeim/Ia32/DxeLoadFunc.c), parse-error recovery
//     expands the local defines and the rule deletes them while missing
//     their in-function uses; without the BaseLib.h type information,
//     files that never declare a variable of IA32_IDT_GATE_DESCRIPTOR
//     parse the sizeof argument as an expression and escape the
//     literal-32 rule.
// https://github.com/tianocore/edk2/commit/3454d7ab417b3dcfa2b631a56b72833477bc8e6b

@@
@@
(
-#define CPU_INTERRUPT_NUM 256
|
-#define CPU_EXCEPTION_NUM 32
|
-#define SEC_IDT_ENTRY_COUNT 34
|
-#define IDT_ENTRY_COUNT 32
)

@@
@@
(
- CPU_INTERRUPT_NUM
+ X86_CPU_INTERRUPT_NUM
|
- CPU_EXCEPTION_NUM
+ X86_CPU_INTERRUPT_NUM
|
- SEC_IDT_ENTRY_COUNT
+ X86_CPU_INTERRUPT_NUM
|
- IDT_ENTRY_COUNT
+ X86_CPU_INTERRUPT_NUM
)

// The typedef declaration parses the pattern's sizeof as sizeof(type);
// the matching C-side type knowledge comes from BaseLib.h via
// --include/--include-headers-for-types (see the header comment above),
// without which files that never declare a variable of the type parse
// the sizeof argument as an expression and escape this rule.

@@
typedef IA32_IDT_GATE_DESCRIPTOR;
@@
  sizeof (IA32_IDT_GATE_DESCRIPTOR) *
- 32
+ X86_CPU_INTERRUPT_NUM
