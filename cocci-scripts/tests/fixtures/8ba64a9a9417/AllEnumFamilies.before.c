void
FixtureAllEnumFamilies (
  void
  )
{
  EFI_PCI_IO_PROTOCOL  *ProtocolIo;
  unsigned long long   Value64;
  unsigned int         Value32;
  unsigned short       Value16;
  unsigned char        Value8;

  ProtocolIo->PollMem (
              ProtocolIo,
              EfiPciWidthUint64,
              0,
              0,
              0,
              0,
              0
              );

  ProtocolIo->PollIo (
              ProtocolIo,
              (EFI_PCI_IO_PROTOCOL_WIDTH)EfiPciWidthFifoUint8,
              0,
              0,
              0,
              0,
              0
              );

  ProtocolIo->CopyMem (
              ProtocolIo,
              EfiPciWidthFillUint16,
              0,
              0,
              0,
              0,
              1
              );

  ProtocolIo->Mem.Read (
                ProtocolIo,
                (EFI_PCI_IO_PROTOCOL_WIDTH)EfiPciWidthUint32,
                0,
                0,
                1,
                &Value32
                );

  ProtocolIo->Mem.Write (
                ProtocolIo,
                EfiPciWidthFifoUint64,
                0,
                0,
                1,
                &Value64
                );

  ProtocolIo->Io.Read (
               ProtocolIo,
               (EFI_PCI_IO_PROTOCOL_WIDTH)EfiPciWidthFillUint8,
               0,
               0,
               1,
               &Value8
               );

  ProtocolIo->Io.Write (
               ProtocolIo,
               EfiPciWidthUint16,
               0,
               0,
               1,
               &Value16
               );

  ProtocolIo->Pci.Read (
                ProtocolIo,
                (EFI_PCI_IO_PROTOCOL_WIDTH)EfiPciWidthFifoUint32,
                0,
                1,
                &Value32
                );

  ProtocolIo->Pci.Write (
                ProtocolIo,
                EfiPciWidthFillUint64,
                0,
                1,
                &Value64
                );
}
