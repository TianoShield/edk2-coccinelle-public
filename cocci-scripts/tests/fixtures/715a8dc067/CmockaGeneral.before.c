typedef unsigned long UINTN;
typedef unsigned long EFI_STATUS;
typedef unsigned long UINT64;
typedef unsigned int UINT32;
typedef unsigned short CHAR16;
typedef struct {
  UINT32 Data1;
} EFI_GUID;

EFI_STATUS
MockSetVariable (
  CHAR16   *VariableName,
  EFI_GUID *VendorGuid,
  UINT32   Attributes,
  UINTN    DataSize,
  void     *Data
  )
{
  check_expected_ptr (VariableName);
  check_expected_ptr (VendorGuid);
  check_expected_ptr (Attributes);
  check_expected (DataSize);
  check_expected (Data);

  return 0;
}

void
ExerciseCmockaUse (
  void
  )
{
  EFI_GUID Guid;
  UINT64   Payload;

  expect_value (OtherMockSetVariable, VendorGuid, &Guid);
  will_return (MockGetVariable, &Payload);
}
