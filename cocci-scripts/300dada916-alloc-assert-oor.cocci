// In EFI_STATUS-returning functions, convert AllocateZeroPool assertion-only
// failure handling into an EFI_OUT_OF_RESOURCES return path.
// https://github.com/tianocore/edk2/commit/300dada916aa9492d5fe52693e9d6fd84adccdf3

@alloc_zero_pool_in_status_function@
typedef EFI_STATUS;
identifier Fn;
expression TempPtr;
statement S1, S2, S3, S4;
@@
EFI_STATUS Fn (...) {
  // "..." uses exists semantics: only one control flow path needs to match
  ... when exists
  TempPtr = AllocateZeroPool (...);
- ASSERT (TempPtr != NULL);
+ if (TempPtr == NULL) {
+   ASSERT (TempPtr != NULL);
+   return EFI_OUT_OF_RESOURCES;
+ }
  ... when exists
      when != if (TempPtr == NULL) S1
      when != if (TempPtr == NULL) S1 else S2
      when != if (TempPtr != NULL) S3
      when != if (TempPtr != NULL) S3 else S4
}
