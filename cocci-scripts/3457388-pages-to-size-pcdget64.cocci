// Replace manual EFI page-size multiplication with EFI_PAGES_TO_SIZE
// for PcdFfaTxRxPageCount, as done in commit 3457388b7cec770c.
// https://github.com/tianocore/edk2/commit/3457388b7cec770cc6599c85e50ec81030f3cc9f

@pages_to_size@
expression PAGE_COUNT;
@@
- PcdGet64 (PAGE_COUNT) * EFI_PAGE_SIZE
+ EFI_PAGES_TO_SIZE (PcdGet64 (PAGE_COUNT))
