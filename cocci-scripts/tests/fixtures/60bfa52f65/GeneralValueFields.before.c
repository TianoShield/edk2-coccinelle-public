typedef int EFI_STATUS;
typedef char CHAR8;
typedef char *EFI_STRING;
typedef struct {
  int Type;
  int OtherField;
} EDKII_REDFISH_VALUE;
typedef struct _EDKII_REDFISH_PLATFORM_CONFIG_PROTOCOL EDKII_REDFISH_PLATFORM_CONFIG_PROTOCOL;

#define EFI_INVALID_PARAMETER 2
#define RedfishValueTypeMax 5
#define RedfishValueTypeUnknown 0

EFI_STATUS
RedfishPlatformConfigProtocolSetValue (
  EDKII_REDFISH_PLATFORM_CONFIG_PROTOCOL  *This,
  CHAR8                                   *Schema,
  CHAR8                                   *Version,
  EFI_STRING                              ConfigureLang,
  EDKII_REDFISH_VALUE                     RedfishValue
  )
{
  if ((RedfishValue.Type == RedfishValueTypeUnknown) || (RedfishValue.Type >= RedfishValueTypeMax)) {
    return EFI_INVALID_PARAMETER;
  }

  return RedfishValue.OtherField;
}
