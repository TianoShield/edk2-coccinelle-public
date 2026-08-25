// Change Redfish Platform Config SetValue APIs to pass EDKII_REDFISH_VALUE
// by reference. Inside affected function bodies, update uses of the typed
// value parameter from by-value form to pointer form.
// https://github.com/tianocore/edk2/commit/60bfa52f6520bff0aa265c3e89ef9d6f6aeca5e7

@setvalue_protocol_typedef@
typedef EFI_STATUS;
typedef EDKII_REDFISH_VALUE;
parameter list leading;
identifier value;
@@
typedef EFI_STATUS (*EDKII_REDFISH_PLATFORM_CONFIG_SET_VALUE)(
  leading,
- EDKII_REDFISH_VALUE value
+ EDKII_REDFISH_VALUE *value
  );

@setvalue_library_proto@
typedef EFI_STATUS;
typedef EDKII_REDFISH_VALUE;
identifier value;
@@
EFI_STATUS
RedfishPlatformConfigSetValue (
  ...,
  EDKII_REDFISH_VALUE
+ *
  value
  );

@setvalue_function_value_to_pointer@
typedef EFI_STATUS;
typedef EDKII_REDFISH_VALUE;
identifier fn =~ "^(RedfishPlatformConfigSetValue|RedfishPlatformConfigProtocolSetValue)$";
identifier value, fld;
@@
EFI_STATUS fn (
  ...,
- EDKII_REDFISH_VALUE value
+ EDKII_REDFISH_VALUE *value
  )
{
  <...
(
- value.fld
+ value->fld
|
- &value
+ value
)
  ...>
}

@protocol_setvalue_null_guard@
typedef EFI_STATUS;
typedef EDKII_REDFISH_VALUE;
identifier value, fld;
statement invalid;
@@
EFI_STATUS
RedfishPlatformConfigProtocolSetValue (
  ...,
  EDKII_REDFISH_VALUE *value
  )
{
  ...
- if ((value->fld == RedfishValueTypeUnknown) || (value->fld >= RedfishValueTypeMax)) invalid
+ if ((value == NULL) || (value->fld == RedfishValueTypeUnknown) || (value->fld >= RedfishValueTypeMax)) invalid
  ...
}

@library_setvalue_null_guard@
typedef EFI_STATUS;
typedef EDKII_REDFISH_VALUE;
identifier value;
@@
EFI_STATUS
RedfishPlatformConfigSetValue (
  ...,
  EDKII_REDFISH_VALUE *value
  )
{
  if (mRedfishPlatformConfigLibPrivate.Protocol == NULL) {
    return EFI_NOT_READY;
  }
+
+ if (value == NULL) {
+   return EFI_INVALID_PARAMETER;
+ }
  ...
}
