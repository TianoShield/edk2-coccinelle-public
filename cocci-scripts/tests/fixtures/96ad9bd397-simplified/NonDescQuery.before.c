EFI_STATUS
ExampleNonDescriptorQuery (
  IN EFI_GUID  *Guid
  )
{
  EFI_STATUS              Status;
  VOID                    *TxBuffer;
  UINT64                  TxBufferSize;
  VOID                    *RxBuffer;
  UINT64                  RxBufferSize;
  EFI_FFA_PART_INFO_DESC  *PartInfo;
  UINT32                  Count;
  UINT32                  Size;

  Status = ArmFfaLibGetRxTxBuffers (
             &TxBuffer,
             &TxBufferSize,
             &RxBuffer,
             &RxBufferSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ArmFfaLibPartitionInfoGet (
             Guid,
             FFA_PART_INFO_FLAG_TYPE_COUNT,
             &Count,
             &Size
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PartInfo = (EFI_FFA_PART_INFO_DESC *)RxBuffer;
  return PartInfo->PartitionId == 0 ? EFI_NOT_FOUND : EFI_SUCCESS;
}
