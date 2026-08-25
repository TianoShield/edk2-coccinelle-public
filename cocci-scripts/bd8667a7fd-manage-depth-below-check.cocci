// Move the recursion-depth counter increment in the SMI/MMI dispatchers below
// the handler-presence check, so it is incremented only when a handler list is
// actually about to be walked.
//
//   mXxxManageCallingDepth++;        // at function entry, before the check
//   ...
//   if (HandlerType == NULL) {
//     Head = &mRoot...;              // root handler
//   } else {
//     Entry = ...FindEntry (...);
//     if (Entry == NULL) {
//       return Status;               // early return leaks the increment
//     }
//     Head = &Entry->Handlers;
//   }
//   for (Link = Head->ForwardLink; Link != Head; Link = Link->ForwardLink) ...
// ->
//   ...
//   if (HandlerType == NULL) {
//     Head = &mRoot...;
//   } else {
//     Entry = ...FindEntry (...);
//     if (Entry == NULL) {
//       return Status;
//     }
//     Head = &Entry->Handlers;
//   }
//   mXxxManageCallingDepth++;        // moved to just before the dispatch loop
//   for (Link = Head->ForwardLink; Link != Head; Link = Link->ForwardLink) ...
//
// The counter is only decremented on the path that finds a handler, so
// incrementing it before the "no handler registered" early return made the
// depth grow without bound. SmiManage() and MmiManage() are mirrored copies of
// the same dispatcher, so the fix is the same statement move in both.
//
// Scope / safety:
//   - The counter is a metavariable constrained by name (regex
//     "ManageCallingDepth"); the rule never reorders an unrelated `x++;`.
//   - The move only fires when the handler-presence check sits between the
//     increment and the loop: an `if (...) {...} else {...}` whose else branch
//     contains the `if (Entry == NULL) { ... return ...; }` early exit. This is
//     the leak the commits fix, so an increment already correctly placed (no
//     such guard between it and the loop) is left alone -- the rule is a no-op
//     on already-fixed code.
//   - Requiring the return *between* the increment and the loop is impossible:
//     that early-return branch exits the function and never reaches the loop.
//     It is matched instead inside the else branch, which the fall-through path
//     does traverse on the way to the loop.
//   - The move target is anchored on the doubly-linked-list dispatch loop
//     `for (link = head->ForwardLink; link != head; link = link->ForwardLink)`.
//     The deferred-unregister cleanup loops later in the same function use the
//     GetFirstNode()/IsNull()/GetNextNode() form and are not matched, so the
//     increment lands immediately before the handler walk and nowhere else.
//   - The increment is inserted right before that loop rather than after the
//     `Head = ...` assignment, because in MmiManage() the assignment sits
//     inside the if/else branch while the loop is at function-body scope.
// https://github.com/tianocore/edk2/commit/bd8667a7fd9ff48b90ba7cca82eaf489500708f9
// https://github.com/tianocore/edk2/commit/05cec0b14beff2113ceedba7983e1ce82ff60a9c

@move_depth_increment@
identifier depth =~ "ManageCallingDepth";
expression link, head;
statement body;
@@
- depth++;
  ...
  if (...) {...} else { ... if (...) { ... return ...; } ... }
  ...
+ depth++;
  for (link = head->ForwardLink; link != head; link = link->ForwardLink) body
