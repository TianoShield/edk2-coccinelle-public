// Spell the ACPI RASF "Platform RAS Capabilities" patrol-scrub capability
// flags with the BITn named macros instead of raw hex, matching the ACPI
// spec (bit 0 / bit 1).
//
//   #define ..._RASF_PLATFORM_RAS_CAPABILITY_HARDWARE_BASED_PATROL_SCRUB_SUPPORTED                          0x01
// ->#define ..._RASF_PLATFORM_RAS_CAPABILITY_HARDWARE_BASED_PATROL_SCRUB_SUPPORTED                          BIT0
//   #define ..._RASF_PLATFORM_RAS_CAPABILITY_HARDWARE_BASED_PATROL_SCRUB_SUPPORTED_AND_EXPOSED_TO_SOFTWARE  0x02
// ->#define ..._RASF_PLATFORM_RAS_CAPABILITY_HARDWARE_BASED_PATROL_SCRUB_SUPPORTED_AND_EXPOSED_TO_SOFTWARE  BIT1
//
// Scope / safety:
//   - The rule is anchored on the macro NAME, not on the value.  A bare
//     0x01 -> BIT0 / 0x02 -> BIT1 rewrite would match by value and corrupt
//     the ~90 unrelated defines (table revisions, PM-profile enums, byte/word
//     sizes, other RASF command codes) that share those literals in the same
//     header.
//   - The name regex requires the full
//     "_RASF_PLATFORM_RAS_CAPABILITY_HARDWARE_BASED_PATROL_SCRUB_SUPPORTED"
//     substring, so sibling RASF macros such as
//     ..._RASF_PCC_COMMAND_CODE_... and ..._RASF_PATROL_SCRUB_COMMAND_... are
//     left alone even though they use 0x01/0x02.
//   - The value is paired with its macro (0x01<->BIT0, 0x02<->BIT1) and the
//     rewrite is value-preserving (BIT0==1, BIT1==2): a pure spelling change.
//   - The ACPI version number in the name is not pinned, so the rule applies
//     uniformly to every version header carrying the flags (5.0/5.1/6.0/6.1)
//     and to any future version, not just the ones the commit touched.
// https://github.com/tianocore/edk2/commit/08cecc924605b4fb1571d034b707306ddea200a5

@rasf_capability@
identifier X =~ "_RASF_PLATFORM_RAS_CAPABILITY_HARDWARE_BASED_PATROL_SCRUB_SUPPORTED";
@@
(
- #define X 0x01
+ #define X BIT0
|
- #define X 0x02
+ #define X BIT1
)
