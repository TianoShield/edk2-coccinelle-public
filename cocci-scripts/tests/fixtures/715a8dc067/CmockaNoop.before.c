typedef unsigned long UINTN;
typedef unsigned long EFI_STATUS;
typedef unsigned int UINT32;
typedef unsigned short CHAR16;
typedef struct {
  UINT32 Data1;
} EFI_GUID;

EFI_STATUS
OtherMock (
  CHAR16   *VariableName,
  EFI_GUID *VendorGuid,
  UINT32   Attributes,
  UINTN    DataSize,
  void     *Data
  )
{
  check_expected_ptr (Attributes);
  check_expected (Data);

  return 0;
}

void
ExerciseNoopCases (
  void
  )
{
  EFI_GUID Guid;
  void     *Payload;

  expect_value (MockGetVariable, VariableName, &Guid);
  will_return (MockGetVariable, (UINTN)&Payload);
}
