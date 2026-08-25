EFI_STATUS
CheckAddressAlignment (
  IN VOID   *Buffer,
  IN UINTN  IoAlign
  )
{
  if ((IoAlign > 0) && !IS_ALIGNED ((UINTN)Buffer, IoAlign)) {
    return EFI_INVALID_PARAMETER;
  }

  if (IS_ALIGNED ((UINTN)Buffer, SIZE_4KB)) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_READY;
}
