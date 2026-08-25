UINT64
FixturePcdGet64PageSize (
  IN UINTN   Base,
  OUT UINTN  *Offset
  )
{
  UINT64  Size;

  Size = EFI_PAGES_TO_SIZE (PcdGet64 (PcdExamplePageCount));

  if (Base < (EFI_PAGES_TO_SIZE (PcdGet64 (PcdExamplePageCount)))) {
    return 0;
  }

  *Offset = Base + (EFI_PAGES_TO_SIZE (PcdGet64 (PcdExamplePageCount)));

  return EFI_PAGES_TO_SIZE (PcdGet64 (PcdExamplePageCount));
}
