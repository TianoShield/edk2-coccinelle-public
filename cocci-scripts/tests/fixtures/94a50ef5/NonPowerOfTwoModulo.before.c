BOOLEAN
HasExpectedStride (
  IN UINTN  Value,
  IN UINTN  Divisor
  )
{
  if (Value % 24 == 0) {
    return TRUE;
  }

  if (Value % Divisor != 0) {
    return FALSE;
  }

  ASSERT (Value % 6 == 0);
  ASSERT (Value % Divisor != 0);
  return TRUE;
}
