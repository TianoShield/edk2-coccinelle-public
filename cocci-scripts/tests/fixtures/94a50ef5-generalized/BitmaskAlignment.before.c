BOOLEAN
CheckBitmaskAlignment (
  IN UINTN  Value,
  IN UINTN  Alignment,
  IN UINTN  Offset
  )
{
  if ((((Value + Offset) & (Alignment - 1)) == 0)) {
    return TRUE;
  }

  if ((((Value + Offset) & (Alignment - 1U)) != 0U)) {
    return FALSE;
  }

  ASSERT (((Value & (SIZE_64KB - 1)) == 0));
  ASSERT (((Value & (SIZE_64KB - 1U)) != 0U));
  ASSERT (((Value & (BASE_4KB - 1)) == 0));
  ASSERT (((Value & (BIT12 - 1U)) != 0U));
  return TRUE;
}
