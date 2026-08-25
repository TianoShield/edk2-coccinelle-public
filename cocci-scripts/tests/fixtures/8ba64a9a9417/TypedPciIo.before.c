void
FixtureTypedPciIo (
  void
  )
{
  EFI_PCI_IO_PROTOCOL  *ControllerIo;
  unsigned int         Data32;
  unsigned short       Data16;

  ControllerIo->Pci.Read (
                ControllerIo,
                EfiPciWidthUint32,
                0,
                1,
                &Data32
                );

  ControllerIo->Pci.Write (
                ControllerIo,
                (EFI_PCI_IO_PROTOCOL_WIDTH)EfiPciWidthUint16,
                4,
                1,
                &Data16
                );
}
