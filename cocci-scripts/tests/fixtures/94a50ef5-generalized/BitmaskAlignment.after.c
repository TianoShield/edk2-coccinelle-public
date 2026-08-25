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

  ASSERT (IS_ALIGNED (Value, SIZE_64KB));
  ASSERT (!IS_ALIGNED (Value, SIZE_64KB));
  ASSERT (IS_ALIGNED (Value, BASE_4KB));
  ASSERT (((Value & (BIT12 - 1U)) != 0U));
  return TRUE;
}
