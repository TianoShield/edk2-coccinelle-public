/** @file
  Adversarial neighbors for the 3454d7ab41 X86_CPU_INTERRUPT_NUM rule.

  Every IDT-flavored constant below is deliberately different from the
  exact patterns the rule rewrites; nothing in this file may be
  transformed.
**/

#include <PiPei.h>

//
// A same-shaped define under a different name stays untouched, and so do
// its uses.
//
#define DEBUG_AGENT_IDT_ENTRY_COUNT  33

VOID
AdversarialNeighbors (
  VOID
  )
{
  IA32_DESCRIPTOR           Idtr;
  IA32_IDT_GATE_DESCRIPTOR  IdtEntry[DEBUG_AGENT_IDT_ENTRY_COUNT];
  IA32_IDT_GATE_DESCRIPTOR  *IdtEntryPtr;
  UINT64                    TopOfStack;
  UINTN                     Index;
  UINTN                     Size;

  //
  // The DebugAgent 33-entry IDT: the value differs, it must not become
  // X86_CPU_INTERRUPT_NUM.
  //
  CopyMem (&IdtEntry, (VOID *)Idtr.Base, 33 * sizeof (IA32_IDT_GATE_DESCRIPTOR));
  Idtr.Limit = (UINT16)(sizeof (IA32_IDT_GATE_DESCRIPTOR) * 33 - 1);

  //
  // The S3 save-state 0x100 spelling is a different pattern.
  //
  Idtr.Limit = (UINT16)(sizeof (IA32_IDT_GATE_DESCRIPTOR) * 0x100 - 1);

  //
  // Descriptor *offsets* scale on the left with small vector numbers.
  //
  IdtEntryPtr = (IA32_IDT_GATE_DESCRIPTOR *)(Idtr.Base + (3 * sizeof (IA32_IDT_GATE_DESCRIPTOR)));

  //
  // Bare 32s that are not scaled by the IDT descriptor size.
  //
  TopOfStack = TopOfStack - 32;
  Size       = sizeof (IA32_DESCRIPTOR) * 32;

  for (Index = 0; Index < 32; Index++) {
    IdtEntry[Index].Bits.Reserved_0 = 0;
  }
}
