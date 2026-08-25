BOOLEAN
CheckModuloAlignment (
  IN UINTN  Value,
  IN UINTN  Alignment,
  IN UINTN  Offset,
  IN UINTN  Length,
  IN UINT8  *Pointer
  )
{
  if (((Value + Offset) % SIZE_64KB) == 0) {
    return TRUE;
  }

  if ((Length % EFI_PAGE_SIZE) != 0U) {
    return FALSE;
  }

  if ((Length % BASE_2MB) == 0) {
    return TRUE;
  }

  if ((Length % BIT21) != 0) {
    return FALSE;
  }

  if ((Length % CPU_STACK_ALIGNMENT) == 0) {
    return TRUE;
  }

  if ((Value % RUNTIME_PAGE_ALLOCATION_GRANULARITY) != 0) {
    return FALSE;
  }

  if ((Length % (1 << (Offset & 0x03))) == 0) {
    return TRUE;
  }

  if ((Length % 0x20000ULL) != 0) {
    return FALSE;
  }

  if ((Length % 1U) == 0) {
    return TRUE;
  }

  if ((Length % sizeof (Pointer)) == 0) {
    return TRUE;
  }

  if ((Length % sizeof (UINT8 *)) != 0) {
    return FALSE;
  }

  if ((Value % Alignment) != 0) {
    return FALSE;
  }

  ASSERT ((Value % 24) == 0);
  return TRUE;
}
