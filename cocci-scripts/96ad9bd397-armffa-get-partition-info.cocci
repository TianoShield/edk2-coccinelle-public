// Replace manual FF-A descriptor partition-info lookup boilerplate with
// ArmFfaLibGetPartitionInfo().  The rule is anchored on
// EFI_FFA_PART_INFO_DESC and FFA_PART_INFO_FLAG_TYPE_DESC so non-descriptor
// partition-info queries are left alone.
// https://github.com/tianocore/edk2/commit/96ad9bd397ca721a78fd0d41021f4621c4a04082

virtual after_replace_old_sequence

@initialize:python@
@@

seen = set()

def queue_after_replace(file, fn):
    key = ("after_replace_old_sequence", file, fn)
    if key not in seen:
        seen.add(key)
        it = Iteration()
        it.set_files([file])
        it.add_virtual_rule(after_replace_old_sequence)
        it.add_virtual_identifier(replaced_fn, fn)
        it.register()

@replace_old_sequence depends on !after_replace_old_sequence@
typedef EFI_STATUS;
identifier Fn;
identifier Info, Count, TxBuffer, TxBufferSize, RxBuffer, RxBufferSize;
identifier PartId, Status, Size;
identifier Field =~ "^(PartitionId|PartitionProps)$";
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
    <...
-   Info->Field
+   Info.Field
    ...>
- }
|
- if ((Count != 1) || (Size < sizeof (EFI_FFA_PART_INFO_DESC))) {
-   ...
- }
  ...
- Info = (EFI_FFA_PART_INFO_DESC *)RxBuffer;
  <...
- Info->Field
+ Info.Field
  ...>
)
  ...
- ArmFfaLibRxRelease (PartId);
  ...
}

@script:python depends on replace_old_sequence@
fn << replace_old_sequence.Fn;
p << replace_old_sequence.p;
@@

queue_after_replace(p[0].file, fn)

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

@find_duplicate_return_labels depends on after_replace_old_sequence exists@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
identifier Label1, Label2;
@@
EFI_STATUS replaced_fn (...) {
  ...
  Label1:
  Label2:
  return Status;
}

@status_goto_to_return depends on after_replace_old_sequence exists@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
identifier Label;
expression Ret;
@@
EFI_STATUS replaced_fn (...) {
  ...
- Status = Ret;
- goto Label;
+ return Ret;
  ...
}

@status_block_goto_to_return depends on after_replace_old_sequence exists@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
identifier Label, OtherLabel;
expression Ret;
statement S;
@@
EFI_STATUS replaced_fn (...) {
  ...
  if (...) {
    ...
    Status = Ret;
    S
    ...
-   goto Label;
+   return Status;
  }
  ...
(
  Label:
|
  Label:
  OtherLabel:
)
  return Status;
}

@efi_error_goto_to_return depends on after_replace_old_sequence exists@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
identifier Label, OtherLabel;
@@
EFI_STATUS replaced_fn (...) {
  ...
  if (EFI_ERROR (Status)) {
    ...
-   goto Label;
+   return Status;
  }
  ...
(
  Label:
|
  Label:
  OtherLabel:
)
  return Status;
}

@device_error_to_return depends on after_replace_old_sequence exists@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
@@
EFI_STATUS replaced_fn (...) {
  ...
-   Status = EFI_DEVICE_ERROR;
+   return EFI_DEVICE_ERROR;
  ... when any
}

@collapse_duplicate_return_labels depends on find_duplicate_return_labels@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
identifier find_duplicate_return_labels.Label1, find_duplicate_return_labels.Label2;
@@
EFI_STATUS replaced_fn (...) {
  ...
- Label1:
- Label2:
- return Status;
+ return EFI_SUCCESS;
}

@cleanup_labels_to_success depends on after_replace_old_sequence && !find_duplicate_return_labels@
typedef EFI_STATUS;
identifier virtual.replaced_fn, Status;
identifier Label;
@@
EFI_STATUS replaced_fn (...) {
  ...
- Label:
- return Status;
+ return EFI_SUCCESS;
}
