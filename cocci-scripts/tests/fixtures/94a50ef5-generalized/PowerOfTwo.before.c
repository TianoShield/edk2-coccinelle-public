BOOLEAN
CheckPowerOfTwo (
  IN UINTN  Value,
  IN UINTN  Other
  )
{
  if (((Value & (Value - 1)) == 0)) {
    return TRUE;
  }

  if ((((Other + 1) & ((Other + 1) - 1U)) != 0U)) {
    return FALSE;
  }

  ASSERT (((Value & (Value - 1U)) == 0U));
  return TRUE;
}
