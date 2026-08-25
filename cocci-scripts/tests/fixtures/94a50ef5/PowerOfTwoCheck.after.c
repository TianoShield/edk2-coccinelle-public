BOOLEAN
HasValidSizeAlignment (
  IN UINT64  BaseAddress,
  IN UINT64  Length
  )
{
  if (!IS_ALIGNED (BaseAddress, SIZE_64KB)) {
    return FALSE;
  }

  if (IS_ALIGNED (Length, SIZE_128MB)) {
    ASSERT (IS_ALIGNED (BaseAddress, SIZE_1GB));
    return TRUE;
  }

  ASSERT (!IS_ALIGNED (Length, SIZE_8KB));
  return FALSE;
}
