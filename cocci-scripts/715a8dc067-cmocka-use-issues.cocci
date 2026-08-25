// Fix Cmocka usage issues seen in SecureBootVariableLib unit tests:
// compare EFI_GUID parameters with expect_memory(), use scalar vs pointer
// checks for MockSetVariable() parameters, and cast MockGetVariable pointer
// return values through UINTN for will_return().
//
// The parameter check rewrites are intentionally scoped to MockSetVariable()
// because Attributes is a scalar parameter while Data is a pointer parameter
// in that mock's signature.
// https://github.com/tianocore/edk2/commit/715a8dc06756dd666eb57e86dec25752a6fc60b6

@vendor_guid_expect_memory@
identifier mock;
identifier guid;
@@
- expect_value
+ expect_memory
  (mock, VendorGuid, &guid
+, sizeof (guid)
  );

@mock_set_variable_expected_checks@
identifier fn =~ "^MockSetVariable$";
@@
fn (...)
{
  <...
(
- check_expected_ptr (Attributes);
+ check_expected (Attributes);
|
- check_expected (Data);
+ check_expected_ptr (Data);
)
  ...>
}

@mock_get_variable_will_return_pointer@
expression value;
@@
- will_return (MockGetVariable, &value);
+ will_return (MockGetVariable, (UINTN)&value);
