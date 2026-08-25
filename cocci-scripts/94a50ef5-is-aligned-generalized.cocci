// Generalize hand-written alignment checks to edk2's IS_ALIGNED() macro.
// Both bitmask and modulo alignment checks are limited to known power-of-two
// expressions: edk2 named size/alignment constants, left shifts of 1, or
// sizeof() of UEFI integer types or pointer types.  BITn and integer literal
// powers of two are not accepted because they are too broad across the real
// codebase.  If integer literals are enabled later, the 1 << E1 branch must
// appear before the integer-literal branch so literal 1 does not bind before the
// whole shift can match.  The E & (E - 1) idiom is intentionally left untouched
// because it checks power-of-two-or-zero rather than alignment.
// Based on https://github.com/tianocore/edk2/commit/94a50ef550b9523a871b9742501995cba0920e67

// Integer literal powers of two are currently disabled.  This helper is kept
// here for reference if that matching scope is later narrowed enough to enable.
//
// @initialize:python@
// @@
//
// def is_int_literal_power_of_2(value):
//     text = str(value).rstrip("uUlL")
//     if not text:
//         return False
//
//     try:
//         if len(text) > 1 and text[0] == "0" and text[1] not in "xXbB":
//             number = int(text, 8)
//         else:
//             number = int(text, 0)
//     except ValueError:
//         return False
//
//     return number >= 1 and (number & (number - 1)) == 0

@power_of_2_expr@
expression PowOf2Expr;
expression E1;
typedef BOOLEAN, CHAR8, CHAR16, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, INTN, UINTN;
type ScalarType = { BOOLEAN, CHAR8, CHAR16, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, INTN, UINTN };
type AnyType;
type PointerType = AnyType *;
idexpression ScalarType ScalarValue;
idexpression PointerType PointerValue;
constant SizeBase =~ "^(SIZE|BASE)_(1|2|4|8|16|32|64|128|256|512)[KMGTPE]B$";
constant NamedPowerOf2 =~ "^(EFI_PAGE_SIZE|CPU_STACK_ALIGNMENT|RUNTIME_PAGE_ALLOCATION_GRANULARITY)$";
// constant IntPowerOf2 : script:python() { is_int_literal_power_of_2(IntPowerOf2) };
constant ONE = {1, 1U, 1u};
@@
(
(
  SizeBase
|
  NamedPowerOf2
|
  ONE << E1
// |
//   IntPowerOf2
|
  sizeof (ScalarType)
|
  sizeof (PointerType)
|
  sizeof (ScalarValue)
|
  sizeof (PointerValue)
)
&
PowOf2Expr
)

@aligned depends on power_of_2_expr disable is_zero,isnt_zero@
expression E;
expression power_of_2_expr.PowOf2Expr;
constant ONE = {1, 1U, 1u};
constant ZERO = {0, 0U, 0u};
@@
(
  ((E & (E - ONE)) == ZERO)
|
- ((E & ((PowOf2Expr) - ONE)) == ZERO)
+ IS_ALIGNED (E, PowOf2Expr)
|
  ((E & (E - ONE)) != ZERO)
|
- ((E & ((PowOf2Expr) - ONE)) != ZERO)
+ !IS_ALIGNED (E, PowOf2Expr)
|
- ((E % (PowOf2Expr)) == ZERO)
+ IS_ALIGNED (E, PowOf2Expr)
|
- ((E % (PowOf2Expr)) != ZERO)
+ !IS_ALIGNED (E, PowOf2Expr)
)

@address_is_aligned@
typedef UINTN;
expression *Address;
expression Alignment;
@@
- IS_ALIGNED ((UINTN) Address, Alignment)
+ ADDRESS_IS_ALIGNED (Address, Alignment)

@normalize_aligned disable paren expression@
expression E, SZ;
@@
(
- (IS_ALIGNED (E, SZ))
+ IS_ALIGNED (E, SZ)
|
- (!IS_ALIGNED (E, SZ))
+ !IS_ALIGNED (E, SZ)
)

@normalize_macro_args disable paren expression@
expression E, SZ;
@@
(
- IS_ALIGNED ((E), SZ)
+ IS_ALIGNED (E, SZ)
|
- IS_ALIGNED (E, (SZ))
+ IS_ALIGNED (E, SZ)
)
