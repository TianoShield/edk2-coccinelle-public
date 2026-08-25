VOID
InitProtectedMemRange (
  VOID
  )
{
  UINTN                    Index;
  MM_CPU_MEMORY_REGION     *MemoryRegion;
  UINTN                    MemoryRegionCount;
  UINTN                    NumberOfAddedDescriptors;
  UINTN                    NumberOfProtectRange;
  UINTN                    NumberOfSpliteRange;
  UINTN                    TotalSize;
  EFI_PHYSICAL_ADDRESS     ProtectBaseAddress;
  EFI_PHYSICAL_ADDRESS     ProtectEndAddress;
  EFI_PHYSICAL_ADDRESS     Top2MBAlignedAddress;
  EFI_PHYSICAL_ADDRESS     Base2MBAlignedAddress;
  UINT64                   High4KBPageSize;
  UINT64                   Low4KBPageSize;
  MEMORY_PROTECTION_RANGE  MemProtectionRange;

  MemoryRegion             = NULL;
  MemoryRegionCount        = 0;
  NumberOfAddedDescriptors = mSmmCpuSmramRangeCount;
  NumberOfSpliteRange      = 0;

  //
  // Create extended protection MemoryRegion and add them into protected memory ranges.
  // Retrieve the accessible regions when SMM profile is enabled.
  // In SMM: only MMIO is accessible.
  // In MM: all regions described by resource HOBs are accessible.
  //
  CreateExtendedProtectionRange (&MemoryRegion, &MemoryRegionCount);
  ASSERT (MemoryRegion != NULL);

  NumberOfAddedDescriptors += MemoryRegionCount;

  ASSERT (NumberOfAddedDescriptors != 0);

  TotalSize           = NumberOfAddedDescriptors * sizeof (MEMORY_PROTECTION_RANGE) + sizeof (mProtectionMemRangeTemplate);
  mProtectionMemRange = (MEMORY_PROTECTION_RANGE *)AllocateZeroPool (TotalSize);
  ASSERT (mProtectionMemRange != NULL);
  mProtectionMemRangeCount = TotalSize / sizeof (MEMORY_PROTECTION_RANGE);

  //
  // Copy existing ranges.
  //
  CopyMem (mProtectionMemRange, mProtectionMemRangeTemplate, sizeof (mProtectionMemRangeTemplate));

  //
  // Create split ranges which come from protected ranges.
  //
  TotalSize      = (TotalSize / sizeof (MEMORY_PROTECTION_RANGE)) * sizeof (MEMORY_RANGE);
  mSplitMemRange = (MEMORY_RANGE *)AllocateZeroPool (TotalSize);
  ASSERT (mSplitMemRange != NULL);

  //
  // Create SMM ranges which are set to present and execution-enable.
  //
  NumberOfProtectRange = sizeof (mProtectionMemRangeTemplate) / sizeof (MEMORY_PROTECTION_RANGE);
  for (Index = 0; Index < mSmmCpuSmramRangeCount; Index++) {
    if ((mSmmCpuSmramRanges[Index].CpuStart >= mProtectionMemRange[0].Range.Base) &&
        (mSmmCpuSmramRanges[Index].CpuStart + mSmmCpuSmramRanges[Index].PhysicalSize < mProtectionMemRange[0].Range.Top))
    {
      //
      // If the address have been already covered by mCpuHotPlugData.SmrrBase/mCpuHotPlugData.SmrrSiz
      //
      break;
    }

    mProtectionMemRange[NumberOfProtectRange].Range.Base = mSmmCpuSmramRanges[Index].CpuStart;
    mProtectionMemRange[NumberOfProtectRange].Range.Top  = mSmmCpuSmramRanges[Index].CpuStart + mSmmCpuSmramRanges[Index].PhysicalSize;
    mProtectionMemRange[NumberOfProtectRange].Present    = TRUE;
    mProtectionMemRange[NumberOfProtectRange].Nx         = FALSE;
    NumberOfProtectRange++;
  }

  //
  // Create protection ranges which are set to present and execution-disable.
  //
  for (Index = 0; Index < MemoryRegionCount; Index++) {
    mProtectionMemRange[NumberOfProtectRange].Range.Base = MemoryRegion[Index].Base;
    mProtectionMemRange[NumberOfProtectRange].Range.Top  = MemoryRegion[Index].Base +  MemoryRegion[Index].Length;
    mProtectionMemRange[NumberOfProtectRange].Present    = TRUE;
    mProtectionMemRange[NumberOfProtectRange].Nx         = TRUE;
    NumberOfProtectRange++;
  }

  //
  // Free the MemoryRegion
  //
  if (MemoryRegion != NULL) {
    FreePool (MemoryRegion);
  }

  //
  // Check and updated actual protected memory ranges count
  //
  ASSERT (NumberOfProtectRange <= mProtectionMemRangeCount);
  mProtectionMemRangeCount = NumberOfProtectRange;

  //
  // According to protected ranges, create the ranges which will be mapped by 2KB page.
  //
  NumberOfSpliteRange  = 0;
  NumberOfProtectRange = mProtectionMemRangeCount;
  for (Index = 0; Index < NumberOfProtectRange; Index++) {
    //
    // If base address is not 2MB alignment, make 2MB alignment for create 4KB page in page table.
    //
    ProtectBaseAddress = mProtectionMemRange[Index].Range.Base;
    ProtectEndAddress  = mProtectionMemRange[Index].Range.Top;
    if (!IS_ALIGNED (ProtectBaseAddress, SIZE_2MB) || !IS_ALIGNED (ProtectEndAddress, SIZE_2MB)) {
      //
      // Check if it is possible to create 4KB-page for not 2MB-aligned range and to create 2MB-page for 2MB-aligned range.
      // A mix of 4KB and 2MB page could save SMRAM space.
      //
      Top2MBAlignedAddress  = ProtectEndAddress & ~(SIZE_2MB - 1);
      Base2MBAlignedAddress = (ProtectBaseAddress + SIZE_2MB - 1) & ~(SIZE_2MB - 1);
      if ((Top2MBAlignedAddress > Base2MBAlignedAddress) &&
          ((Top2MBAlignedAddress - Base2MBAlignedAddress) >= SIZE_2MB))
      {
        //
        // There is an range which could be mapped by 2MB-page.
        //
        High4KBPageSize = ((ProtectEndAddress + SIZE_2MB - 1) & ~(SIZE_2MB - 1)) - (ProtectEndAddress & ~(SIZE_2MB - 1));
        Low4KBPageSize  = ((ProtectBaseAddress + SIZE_2MB - 1) & ~(SIZE_2MB - 1)) - (ProtectBaseAddress & ~(SIZE_2MB - 1));
        if (High4KBPageSize != 0) {
          //
          // Add not 2MB-aligned range to be mapped by 4KB-page.
          //
          mSplitMemRange[NumberOfSpliteRange].Base = ProtectEndAddress & ~(SIZE_2MB - 1);
          mSplitMemRange[NumberOfSpliteRange].Top  = (ProtectEndAddress + SIZE_2MB - 1) & ~(SIZE_2MB - 1);
          NumberOfSpliteRange++;
        }

        if (Low4KBPageSize != 0) {
          //
          // Add not 2MB-aligned range to be mapped by 4KB-page.
          //
          mSplitMemRange[NumberOfSpliteRange].Base = ProtectBaseAddress & ~(SIZE_2MB - 1);
          mSplitMemRange[NumberOfSpliteRange].Top  = (ProtectBaseAddress + SIZE_2MB - 1) & ~(SIZE_2MB - 1);
          NumberOfSpliteRange++;
        }
      } else {
        //
        // The range could only be mapped by 4KB-page.
        //
        mSplitMemRange[NumberOfSpliteRange].Base = ProtectBaseAddress & ~(SIZE_2MB - 1);
        mSplitMemRange[NumberOfSpliteRange].Top  = (ProtectEndAddress + SIZE_2MB - 1) & ~(SIZE_2MB - 1);
        NumberOfSpliteRange++;
      }
    }
  }

  mSplitMemRangeCount = NumberOfSpliteRange;

  //
  // Sort the mProtectionMemRange
  //
  QuickSort (mProtectionMemRange, mProtectionMemRangeCount, sizeof (MEMORY_PROTECTION_RANGE), (BASE_SORT_COMPARE)ProtectionRangeCompare, &MemProtectionRange);

  DEBUG ((DEBUG_INFO, "SMM Profile Memory Ranges:\n"));
  for (Index = 0; Index < mProtectionMemRangeCount; Index++) {
    DEBUG ((DEBUG_INFO, "mProtectionMemRange[%d].Base = %lx\n", Index, mProtectionMemRange[Index].Range.Base));
    DEBUG ((DEBUG_INFO, "mProtectionMemRange[%d].Top  = %lx\n", Index, mProtectionMemRange[Index].Range.Top));
  }

  for (Index = 0; Index < mSplitMemRangeCount; Index++) {
    DEBUG ((DEBUG_INFO, "mSplitMemRange[%d].Base = %lx\n", Index, mSplitMemRange[Index].Base));
    DEBUG ((DEBUG_INFO, "mSplitMemRange[%d].Top  = %lx\n", Index, mSplitMemRange[Index].Top));
  }
}
