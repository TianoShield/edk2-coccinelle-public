// Normalize EFI_PCI_IO_PROTOCOL width literals accidentally taken from the
// EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL enum family.
//
// The test harness passes cocci-scripts/parsing_hacks.h via
// --macro-file-builtins, so macros like IN are dropped before matching typed
// receivers such as `IN EFI_PCI_IO_PROTOCOL *PciIo;`.
// https://github.com/tianocore/edk2/commit/8ba64a9a9417b45b033cf1307a9e96f3df2530f2

@initialize:python@
@@

def to_pci_io_width(name):
    return name.replace("EfiPciWidth", "EfiPciIoWidth", 1)

@normalize@
typedef EFI_PCI_IO_PROTOCOL;
typedef EDKII_PCI_DEVICE_PPI;
type T =~ "^EFI_PCI_IO_PROTOCOL_WIDTH$";
EFI_PCI_IO_PROTOCOL *x;
EDKII_PCI_DEVICE_PPI *y;
identifier bad =~ "EfiPciWidth(Uint|FifoUint|FillUint)(8|16|32|64)";
identifier top_op =~ "^(PollMem|PollIo|CopyMem)$";
identifier space =~ "^(Mem|Io|Pci)$";
identifier rw =~ "^(Read|Write)$";
fresh identifier good = script:python(bad) { to_pci_io_width(bad) };
expression first;
expression list rest;
@@
(
  x->top_op
|
  y->PciIo.top_op
|
  x->space.rw
|
  y->PciIo.space.rw
) (
  first,
- (T)bad
+ good
  ,
  rest
  )
