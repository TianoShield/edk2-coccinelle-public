VOID
CheckNormalizedParens (
  IN UINTN  Value,
  IN UINTN  Alignment
  )
{
  ASSERT ((IS_ALIGNED (Value, Alignment)));
  ASSERT ((!IS_ALIGNED (Value, Alignment)));
}
