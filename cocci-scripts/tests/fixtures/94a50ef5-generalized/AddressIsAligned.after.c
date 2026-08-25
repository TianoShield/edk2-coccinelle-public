EFI_STATUS
CheckAddressAlignment (
  IN VOID   *Buffer,
  IN UINTN  IoAlign
  )
{
  if ((IoAlign > 0) && !ADDRESS_IS_ALIGNED (Buffer, IoAlign)) {
    return EFI_INVALID_PARAMETER;
  }

  if (ADDRESS_IS_ALIGNED (Buffer, SIZE_4KB)) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_READY;
}
