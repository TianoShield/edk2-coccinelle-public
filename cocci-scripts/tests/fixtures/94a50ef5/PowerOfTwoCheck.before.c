BOOLEAN
HasValidSizeAlignment (
  IN UINT64  BaseAddress,
  IN UINT64  Length
  )
{
  if ((BaseAddress & (SIZE_64KB - 1)) != 0) {
    return FALSE;
  }

  if (Length % SIZE_128MB == 0) {
    ASSERT ((BaseAddress & (SIZE_1GB - 1)) == 0);
    return TRUE;
  }

  ASSERT (Length % SIZE_8KB != 0);
  return FALSE;
}
