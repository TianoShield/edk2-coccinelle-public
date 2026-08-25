// Replace manual FF-A descriptor partition-info lookup boilerplate with
// ArmFfaLibGetPartitionInfo(). This simplified rule stops after the core
// rewrite and old-variable cleanup; it intentionally leaves any surrounding
// control-flow cleanup to other rules.
// https://github.com/tianocore/edk2/commit/96ad9bd397ca721a78fd0d41021f4621c4a04082

@replace_old_sequence@
typedef EFI_STATUS;
identifier Fn;
identifier Info, Count, TxBuffer, TxBufferSize, RxBuffer, RxBufferSize;
identifier PartId, Status, Size;
expression Guid;
position p;
statement S1, S2;
type SizeArgT;
@@
EFI_STATUS Fn@p (...) {
...
- EFI_FFA_PART_INFO_DESC *Info;
+ EFI_FFA_PART_INFO_DESC Info;
...
- Status = ArmFfaLibPartitionIdGet (&PartId);
- if (EFI_ERROR (Status)) S1
...
- Status = ArmFfaLibGetRxTxBuffers (
-            &TxBuffer,
-            &TxBufferSize,
-            &RxBuffer,
-            &RxBufferSize
-            );
- if (EFI_ERROR (Status)) S2
...
- Status = ArmFfaLibPartitionInfoGet (
-            Guid,
-            FFA_PART_INFO_FLAG_TYPE_DESC,
-            &Count,
-            (SizeArgT)&Size
-            );
+ Status = ArmFfaLibGetPartitionInfo (Guid, &Info);
...
(
- if ((Count != 1) || (Size < sizeof (EFI_FFA_PART_INFO_DESC))) {
-   ...
- } else {
-   Info = (EFI_FFA_PART_INFO_DESC *)RxBuffer;
    ...
- }
|
- if ((Count != 1) || (Size < sizeof (EFI_FFA_PART_INFO_DESC))) {
-   ...
- }
  ...
- Info = (EFI_FFA_PART_INFO_DESC *)RxBuffer;
)
  ...
- ArmFfaLibRxRelease (PartId);
  ...
}

@part_info_members depends on replace_old_sequence@
identifier replace_old_sequence.Info;
identifier Field =~ "^(PartitionId|PartitionProps)$";
@@
- Info->Field
+ Info.Field

@remove_old_variables depends on replace_old_sequence@
identifier replace_old_sequence.Count, replace_old_sequence.TxBuffer;
identifier replace_old_sequence.TxBufferSize, replace_old_sequence.RxBuffer, replace_old_sequence.RxBufferSize;
identifier replace_old_sequence.PartId;
identifier replace_old_sequence.Size;
type T;
@@
(
- T *TxBuffer;
|
- UINT64 TxBufferSize;
|
- T *RxBuffer;
|
- UINT64 RxBufferSize;
|
- UINT32 Count;
|
- UINT32 Size;
|
- UINT16 PartId;
|
- static UINT16 PartId;
)
