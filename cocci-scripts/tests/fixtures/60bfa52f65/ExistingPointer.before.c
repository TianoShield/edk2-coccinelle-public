typedef int EFI_STATUS;
typedef struct {
  int Type;
} EDKII_REDFISH_VALUE;

EFI_STATUS
RedfishPlatformConfigGetValue (
  EDKII_REDFISH_VALUE  *Value
  )
{
  return Value->Type;
}
