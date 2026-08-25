BOOLEAN
CheckModuloAlignment (
  IN UINTN  Value,
  IN UINTN  Alignment,
  IN UINTN  Offset,
  IN UINTN  Length,
  IN UINT8  *Pointer
  )
{
  if (IS_ALIGNED (Value + Offset, SIZE_64KB)) {
    return TRUE;
  }

  if (!IS_ALIGNED (Length, EFI_PAGE_SIZE)) {
    return FALSE;
  }

  if (IS_ALIGNED (Length, BASE_2MB)) {
    return TRUE;
  }

  if ((Length % BIT21) != 0) {
    return FALSE;
  }

  if (IS_ALIGNED (Length, CPU_STACK_ALIGNMENT)) {
    return TRUE;
  }

  if (!IS_ALIGNED (Value, RUNTIME_PAGE_ALLOCATION_GRANULARITY)) {
    return FALSE;
  }

  if (IS_ALIGNED (Length, 1 << (Offset & 0x03))) {
    return TRUE;
  }

  if ((Length % 0x20000ULL) != 0) {
    return FALSE;
  }

  if ((Length % 1U) == 0) {
    return TRUE;
  }

  if (IS_ALIGNED (Length, sizeof (Pointer))) {
    return TRUE;
  }

  if (!IS_ALIGNED (Length, sizeof (UINT8 *))) {
    return FALSE;
  }

  if ((Value % Alignment) != 0) {
    return FALSE;
  }

  ASSERT ((Value % 24) == 0);
  return TRUE;
}
