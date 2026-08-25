EFI_STATUS
FfaTpm2GetServicePartitionId (
  OUT UINT16  *PartitionId
  )
{
  EFI_STATUS              Status;
  EFI_FFA_PART_INFO_DESC  TpmPartInfo;

  if (PartitionId == NULL) {
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  Status = ArmFfaLibGetPartitionInfo (&gTpm2ServiceFfaGuid, &TpmPartInfo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to get Tpm2 partition info. Status: %r\n", Status));
    goto RxRelease;
  }

  *PartitionId = TpmPartInfo.PartitionId;
  if (TpmPartInfo.PartitionId == TPM2_FFA_PARTITION_ID_INVALID) {
    /*
       * Tpm partition id never be TPM2_FFA_PARTITION_ID_INVALID.
       */
    Status = EFI_DEVICE_ERROR;
  }

RxRelease:

Exit:
  return Status;
}
