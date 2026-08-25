/** @file
  Synthetic dispatcher exercising the depth-tracker move on a differently
  named counter, function, and loop variables. Same root/non-root presence
  check shape as SmiManage()/MmiManage(): the not-found early return is nested
  in the else branch.
**/

#include "Foo.h"

LIST_ENTRY  mRootFooHandlerList = INITIALIZE_LIST_HEAD_VARIABLE (mRootFooHandlerList);
UINTN       mFooManageCallingDepth = 0;

EFI_STATUS
EFIAPI
FooManage (
  IN CONST EFI_GUID  *HandlerType
  )
{
  LIST_ENTRY   *Node;
  LIST_ENTRY   *ListHead;
  FOO_ENTRY    *Entry;
  FOO_HANDLER  *Handler;
  EFI_STATUS   Status;

  mFooManageCallingDepth++;
  Status = EFI_NOT_FOUND;
  if (HandlerType == NULL) {
    ListHead = &mRootFooHandlerList;
  } else {
    Entry = FooFindEntry ((EFI_GUID *)HandlerType);
    if (Entry == NULL) {
      //
      // There is no handler registered for this interrupt source
      //
      return Status;
    }

    ListHead = &Entry->Handlers;
  }

  for (Node = ListHead->ForwardLink; Node != ListHead; Node = Node->ForwardLink) {
    Handler = CR (Node, FOO_HANDLER, Link, FOO_HANDLER_SIGNATURE);
    Status  = Handler->Handler ((EFI_HANDLE)Handler);
  }

  ASSERT (mFooManageCallingDepth > 0);
  mFooManageCallingDepth--;

  return Status;
}
