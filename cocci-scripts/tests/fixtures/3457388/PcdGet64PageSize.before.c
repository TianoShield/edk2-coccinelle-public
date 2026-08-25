UINT64
FixturePcdGet64PageSize (
  IN UINTN   Base,
  OUT UINTN  *Offset
  )
{
  UINT64  Size;

  Size = PcdGet64 (PcdExamplePageCount) * EFI_PAGE_SIZE;

  if (Base < (PcdGet64 (PcdExamplePageCount) * EFI_PAGE_SIZE)) {
    return 0;
  }

  *Offset = Base + (PcdGet64 (PcdExamplePageCount) * EFI_PAGE_SIZE);

  return EFI_PAGE_SIZE * PcdGet64 (PcdExamplePageCount);
}
