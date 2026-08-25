/** @file
  Negative fixture: none of these functions is a depth-tracker-below-check
  candidate, so the rule must leave all of them untouched.
**/

#include "Foo.h"

//
// A ForwardLink dispatch loop is present, but the counter moved at function
// entry is NOT a *ManageCallingDepth counter, so the name constraint must keep
// it in place.
//
UINTN  mLocalCount = 0;

EFI_STATUS
EFIAPI
WalkNoDepthName (
  IN LIST_ENTRY  *ListHead
  )
{
  LIST_ENTRY   *Node;
  FOO_HANDLER  *Handler;
  EFI_STATUS   Status;

  mLocalCount++;
  Status = EFI_NOT_FOUND;
  if (ListHead == NULL) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    if (IsListEmpty (ListHead)) {
      return Status;
    }
  }

  for (Node = ListHead->ForwardLink; Node != ListHead; Node = Node->ForwardLink) {
    Handler = CR (Node, FOO_HANDLER, Link, FOO_HANDLER_SIGNATURE);
    Status  = Handler->Handler ((EFI_HANDLE)Handler);
  }

  return Status;
}

//
// The counter name matches, but there is no ForwardLink dispatch loop (the only
// loop uses the GetFirstNode()/IsNull()/GetNextNode() cleanup form), so the
// loop anchor must keep the increment in place.
//
UINTN  mBarManageCallingDepth = 0;

VOID
EFIAPI
CleanupNoDispatchLoop (
  IN LIST_ENTRY  *ListHead
  )
{
  LIST_ENTRY   *Node;
  FOO_HANDLER  *Handler;

  mBarManageCallingDepth++;

  for ( Node = GetFirstNode (ListHead)
        ; !IsNull (ListHead, Node);
        )
  {
    Handler = CR (Node, FOO_HANDLER, Link, FOO_HANDLER_SIGNATURE);
    Node    = GetNextNode (ListHead, Node);
    if (Handler->ToRemove) {
      RemoveFooHandler (Handler);
    }
  }

  mBarManageCallingDepth--;
}

//
// The counter name matches and a ForwardLink dispatch loop follows, but there
// is no handler-presence check with an early return between the increment and
// the loop, so the increment is not being leaked and must stay in place.
//
UINTN  mBazManageCallingDepth = 0;

VOID
EFIAPI
WalkNoPresenceCheck (
  IN LIST_ENTRY  *ListHead
  )
{
  LIST_ENTRY   *Node;
  FOO_HANDLER  *Handler;

  mBazManageCallingDepth++;

  for (Node = ListHead->ForwardLink; Node != ListHead; Node = Node->ForwardLink) {
    Handler = CR (Node, FOO_HANDLER, Link, FOO_HANDLER_SIGNATURE);
    Handler->Handler ((EFI_HANDLE)Handler);
  }

  mBazManageCallingDepth--;
}
