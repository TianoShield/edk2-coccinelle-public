//
// Partition ID if FF-A support is enabled
//
STATIC UINT16  mStMmPartId;

/**
  Initialize communication via FF-A.

**/
STATIC
EFI_STATUS
EFIAPI
InitializeFfaCommunication (
  VOID
  )
{
  EFI_STATUS              Status;
  EFI_FFA_PART_INFO_DESC  StmmPartInfo;

  Status = ArmFfaLibGetPartitionInfo (
             &gEfiMmCommunication2ProtocolGuid,
             &StmmPartInfo
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to get Stmm(%g) partition Info. Status: %r\n",
      &gEfiMmCommunication2ProtocolGuid,
      Status
      ));
    return Status;
  }

  if ((StmmPartInfo.PartitionProps & FFA_PART_PROP_RECV_DIRECT_REQ) == 0x00) {
    Status = EFI_UNSUPPORTED;
    DEBUG ((DEBUG_ERROR, "StandaloneMm doesn't receive DIRECT_MSG_REQ...\n"));
    goto ErrorHandler;
  }

  mStMmPartId = StmmPartInfo.PartitionId;

ErrorHandler:
  return Status;
}
