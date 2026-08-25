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
              EfiPciIoWidthUint64,
              0,
              0,
              0,
              0,
              0
              );

  ProtocolIo->PollIo (
              ProtocolIo,
              EfiPciIoWidthFifoUint8,
              0,
              0,
              0,
              0,
              0
              );

  ProtocolIo->CopyMem (
              ProtocolIo,
              EfiPciIoWidthFillUint16,
              0,
              0,
              0,
              0,
              1
              );

  ProtocolIo->Mem.Read (
                ProtocolIo,
                EfiPciIoWidthUint32,
                0,
                0,
                1,
                &Value32
                );

  ProtocolIo->Mem.Write (
                ProtocolIo,
                EfiPciIoWidthFifoUint64,
                0,
                0,
                1,
                &Value64
                );

  ProtocolIo->Io.Read (
               ProtocolIo,
               EfiPciIoWidthFillUint8,
               0,
               0,
               1,
               &Value8
               );

  ProtocolIo->Io.Write (
               ProtocolIo,
               EfiPciIoWidthUint16,
               0,
               0,
               1,
               &Value16
               );

  ProtocolIo->Pci.Read (
                ProtocolIo,
                EfiPciIoWidthFifoUint32,
                0,
                1,
                &Value32
                );

  ProtocolIo->Pci.Write (
                ProtocolIo,
                EfiPciIoWidthFillUint64,
                0,
                1,
                &Value64
                );
}
