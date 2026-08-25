VOID
RomDecode (
  IN PCI_IO_DEVICE  *PciDevice,
  IN UINT8          RomBarIndex,
  IN UINT32         RomBar,
  IN BOOLEAN        Enable
  )
{
  UINT32               Value32;
  EFI_PCI_IO_PROTOCOL  *PciIo;

  PciIo = &PciDevice->PciIo;
  if (Enable) {
    //
    // set the Rom base address: now is hardcode
    // enable its decoder
    //
    Value32 = RomBar | 0x1;
    PciIo->Pci.Write (
                 PciIo,
                 EfiPciIoWidthUint32,
                 RomBarIndex,
                 1,
                 &Value32
                 );

    //
    // Programe all upstream bridge
    //
    ProgramUpstreamBridgeForRom (PciDevice, RomBar, TRUE);

    //
    // Setting the memory space bit in the function's command register
    //
    PCI_ENABLE_COMMAND_REGISTER (PciDevice, EFI_PCI_COMMAND_MEMORY_SPACE);
  } else {
    //
    // disable command register decode to memory
    //
    PCI_DISABLE_COMMAND_REGISTER (PciDevice, EFI_PCI_COMMAND_MEMORY_SPACE);

    //
    // Destroy the programmed bar in all the upstream bridge.
    //
    ProgramUpstreamBridgeForRom (PciDevice, RomBar, FALSE);

    //
    // disable rom decode
    //
    Value32 = 0xFFFFFFFE;
    PciIo->Pci.Write (
                 PciIo,
                 EfiPciIoWidthUint32,
                 RomBarIndex,
                 1,
                 &Value32
                 );
  }
}
