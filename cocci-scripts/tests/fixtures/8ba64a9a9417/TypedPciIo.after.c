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
                EfiPciIoWidthUint32,
                0,
                1,
                &Data32
                );

  ControllerIo->Pci.Write (
                ControllerIo,
                EfiPciIoWidthUint16,
                4,
                1,
                &Data16
                );
}
