// Replace hand-written alignment checks with edk2’s IS_ALIGNED() macro,
// but only when the alignment value is known to be a valid power-of-two
// constant.
// https://github.com/tianocore/edk2/commit/94a50ef550b9523a871b9742501995cba0920e67

@align_const@
constant SZ =~ "^(EFI_PAGE_SIZE|SIZE_[0-9]+[KMGTPE]B)$";
@@
SZ

// Disable iso is_zero,isnt_zero so that expressions like
// (E & (SZ - 1)) are not accidently replaced.
@depends on align_const disable is_zero,isnt_zero@
constant align_const.SZ;
expression E;
@@
(
- ((E & (SZ - 1)) == 0)
+ IS_ALIGNED (E, SZ)
|
- ((E & (SZ - 1)) != 0)
+ !IS_ALIGNED (E, SZ)
|
- ((E % SZ) == 0)
+ IS_ALIGNED (E, SZ)
|
- ((E % SZ) != 0)
+ !IS_ALIGNED (E, SZ)
)

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
