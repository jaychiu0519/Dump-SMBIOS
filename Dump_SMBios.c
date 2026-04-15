/** @file
  A modularized UEFI Shell application to dump comprehensive SMBIOS table information,
  starting with the Entry Point Structure.
  
  This application bypasses the EFI_SMBIOS_PROTOCOL and directly locates the
  SMBIOS Entry Point from the UEFI Configuration Table. It supports command-line
  parameter parsing to filter specific SMBIOS Types. Special focus is given to 
  identifying modern PCIe 4.0 and PCIe 5.0 topologies in Type 9 records, which
  heavily rely on SMBIOS 3.x (64-bit) capabilities.

  Copyright (c) 2026, OEM Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Protocol/ShellParameters.h>
#include <Guid/Smbios.h>
#include <IndustryStandard/Smbios.h>
#include "SmbiosStringTable.h"
#include "MemoryDumpUtil.h"

#define AE_SMBIOS_VERSION(MajorVersion, MinorVersion) \
  (SmbiosMajorVersion > (MajorVersion) || (SmbiosMajorVersion == (MajorVersion) && SmbiosMinorVersion >= (MinorVersion)))

//=============================================================================
// Internal Helper Functions
//=============================================================================

/**
  Retrieves the ASCII string from the SMBIOS Unformatted Section (String Pool) 
  based on the provided String Index.

  @param[in] SmbiosStruct  A union pointer to the current SMBIOS structure.
  @param[in] StringIndex   The 1-based index of the target string.

  @return A pointer to the null-terminated ASCII string. If the index is 0, 
          it returns "None". If the index is out of bounds or the string pool 
          terminates unexpectedly, it returns "BadStringIndex".
**/
CHAR8 *
GetStringByIndex (
  IN SMBIOS_STRUCTURE_POINTER SmbiosStruct,
  IN UINT8                    StringIndex
  )
{
  CHAR8  *StringPtr;
  UINT8  Index;

  if (StringIndex == 0) {
    return "None";
  }

  StringPtr = (CHAR8 *)(SmbiosStruct.Raw + SmbiosStruct.Hdr->Length);

  for (Index = 1; Index < StringIndex; Index++) {
    while (*StringPtr != 0) {
      StringPtr++;
    }
    StringPtr++; 
    if (*StringPtr == 0) {
      return "BadStringIndex";
    }
  }

  return StringPtr;
}

/**
  @brief Calculates the total size of an SMBIOS structure.
  
  This function determines the full size by adding the length of the formatted 
  area (specified in the structure header) to the length of the unformatted 
  string set, terminating at the double-null (0x00 0x00) marker.

  @param[in] Header  Pointer to the SMBIOS structure header.

  @retval UINTN      The total size of the SMBIOS structure in bytes.
**/
UINTN
GetSmbiosStructureTotalSize (
  IN SMBIOS_STRUCTURE  *Header
  )
{
  UINT8 *Ptr;

  if (Header == NULL) {
    return 0;
  }

  Ptr = (UINT8 *)Header + Header->Length; 
  
  // Parse through the unformatted string set to find the double-null terminator
  while ((*Ptr != 0) || (*(Ptr + 1) != 0)) {
    Ptr++;
  }
  
  return (UINTN)(Ptr - (UINT8 *)Header + 2); 
}

/**
  @brief Dumps the raw memory of an SMBIOS structure in Hex and ASCII formats.

  This function outputs the memory layout of the given SMBIOS structure, 
  matching standard OEM debug specifications (16 bytes per line, with a 
  hyphen separator and an ASCII preview).

  @param[in] Header  Pointer to the SMBIOS structure header.
  @param[in] Index   The numerical index of the current SMBIOS structure.
**/
VOID
DumpRawStructure (
  IN SMBIOS_STRUCTURE  *Header,
  IN UINTN             Index
  )
{
  UINT8  *RawData;
  UINTN  TotalSize;
  UINTN  i;
  UINTN  j;
  UINT8  Byte;

  if (Header == NULL) {
    return;
  }

  RawData   = (UINT8 *)Header;
  TotalSize = GetSmbiosStructureTotalSize (Header);

  // Print the structure header info
  Print (L"Type=%d, Handle=0x%X\n", Header->Type, Header->Handle);
  Print (L"Dump Structure as:\n");
  Print (L"Index=%d,Length=0x%X,Addr=0x%08X\n", Index, TotalSize, (UINT32)(UINTN)Header);

  // Execute 16-byte aligned Hex Dump
  for (i = 0; i < TotalSize; i += 16) {
    Print (L"%08X: ", (UINT32)i);

    // Print Hex section
    for (j = 0; j < 16; j++) {
      if (i + j < TotalSize) {
        Print (L"%02X ", RawData[i + j]);
      } else {
        Print (L"   ");
      }
      
      // Insert a hyphen between the 8th and 9th byte for readability
      if (j == 7) {
        Print (L"- ");
      }
    }

    // Print ASCII preview section
    Print (L" *");
    for (j = 0; j < 16; j++) {
      if (i + j < TotalSize) {
        Byte = RawData[i + j];
        // Only print printable ASCII characters (0x20 to 0x7E)
        if (Byte >= 0x20 && Byte <= 0x7E) {
          Print (L"%c", (CHAR16)Byte);
        } else {
          Print (L".");
        }
      } else {
        Print (L" ");
      }
    }
    Print (L"*\n");
  }
} 

/**
  Dumps the Interface Specific Data with a custom prefix for Type 42.

  @param[in] DataPtr      Pointer to the start of the specific data.
  @param[in] DataLength   Length of the specific data in bytes.
**/
STATIC
VOID
DumpInterfaceSpecificData (
  IN UINT8  *DataPtr,
  IN UINTN  DataLength
  )
{
  UINTN i;
  UINTN j;

  Print (L"Dump InterfaceTypeSpecificData\n");
  Print (L"size=%d:\n", DataLength);
  Print (L" |--> size=%d:\n", DataLength);

  for (i = 0; i < DataLength; i += 16) {
    Print (L" |--> %08X: ", (UINT32)i);

    // Print Hex section
    for (j = 0; j < 16; j++) {
      if (i + j < DataLength) {
        Print (L"%02X ", DataPtr[i + j]);
      } else {
        Print (L"   ");
      }
      
      if (j == 7) {
        Print (L"- ");
      }
    }

    // Print ASCII preview section
    Print (L" *");
    for (j = 0; j < 16; j++) {
      if (i + j < DataLength) {
        UINT8 Byte = DataPtr[i + j];
        // Only print printable ASCII characters (0x20 to 0x7E)
        if (Byte >= 0x20 && Byte <= 0x7E) {
          Print (L"%c", (CHAR16)Byte);
        } else {
          Print (L".");
        }
      } else {
        Print (L" ");
      }
    }
    Print (L"*\n");
  }
}

/**
  @brief Locates both SMBIOS 2.x and 3.x Entry Points in the EFI System Table.

  @param[out] Smbios2Table  Pointer to store the 32-bit (_SM_) entry point.
  @param[out] Smbios3Table  Pointer to store the 64-bit (_SM3_) entry point.
**/
VOID
GetSmbiosEntryPoint (
  OUT VOID **Smbios2Table,
  OUT VOID **Smbios3Table
  )
{
  UINTN  Index;

  *Smbios2Table = NULL;
  *Smbios3Table = NULL;

  for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
    if (CompareGuid (&gST->ConfigurationTable[Index].VendorGuid, &gEfiSmbios3TableGuid)) {
      *Smbios3Table = gST->ConfigurationTable[Index].VendorTable;
    }
    if (CompareGuid (&gST->ConfigurationTable[Index].VendorGuid, &gEfiSmbiosTableGuid)) {
      *Smbios2Table = gST->ConfigurationTable[Index].VendorTable;
    }
  }
}

//=============================================================================
// Business Logic Modules
//=============================================================================

/**
  Parses command line arguments to determine if a specific Type filter is requested.

  @param[in]  ImageHandle    The handle of the loaded image.
  @param[out] TargetType     The SMBIOS type to filter for.
  @param[out] FilterEnabled  TRUE if filtering is requested.
**/
VOID
ParseCommandLine (
  IN  EFI_HANDLE  ImageHandle,
  OUT UINT32      *TargetType,
  OUT BOOLEAN     *FilterEnabled
  )
{
  EFI_STATUS                     Status;
  EFI_SHELL_PARAMETERS_PROTOCOL  *ShellParam;

  *TargetType    = 0xFFFFFFFF;
  *FilterEnabled = FALSE;

  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiShellParametersProtocolGuid,
                  (VOID **)&ShellParam
                  );

  if (!EFI_ERROR (Status) && ShellParam != NULL) {
    if (ShellParam->Argc > 1) {
      *TargetType = (UINT32)StrDecimalToUintn (ShellParam->Argv[1]);
      *FilterEnabled = TRUE;
      Print (L"[Info] Filter Enabled: Targeting SMBIOS Type %d\n", *TargetType);
    }
  }
}

/**
  Dump both SMBIOS 2.x (32-bit) and SMBIOS 3.x (64-bit) Entry Point Structures.
  Strict flush-left formatting for OEM automated log parsing.
  
  @param[in] EntryPoint   Pointer to the SMBIOS Table Entry Point.
  @param[in] IsSmbios3    TRUE if the pointer is SMBIOS 3.x (_SM3_), FALSE for 2.x (_SM_).
**/
VOID
DumpSmbiosEntryPoint (
  IN VOID     *EntryPoint,
  IN BOOLEAN  IsSmbios3
  )
{
  if (EntryPoint == NULL) {
    return;
  }

  Print (L"SMBIOS Entry Point Structure:\n");
  Print (L"\n");

  if (IsSmbios3) {
    SMBIOS_TABLE_3_0_ENTRY_POINT *Ep3 = (SMBIOS_TABLE_3_0_ENTRY_POINT *)EntryPoint;
    
    Print (L"Architecture:         64-bit (SMBIOS 3.x)\n");
    Print (L"Anchor String:        %c%c%c%c%c\n", 
           (CHAR16)Ep3->AnchorString[0], (CHAR16)Ep3->AnchorString[1], 
           (CHAR16)Ep3->AnchorString[2], (CHAR16)Ep3->AnchorString[3], 
           (CHAR16)Ep3->AnchorString[4]);
           
    Print (L"EPS Checksum:         0x%02X\n", Ep3->EntryPointStructureChecksum);
    Print (L"Entry Point Len:      %d\n", Ep3->EntryPointLength);
    Print (L"Version:              %d.%d.%d\n", Ep3->MajorVersion, Ep3->MinorVersion, Ep3->DocRev);
    Print (L"EPS Revision:         0x%02X\n", Ep3->EntryPointRevision);
    
    Print (L"Table Max Size:       %d bytes\n", Ep3->TableMaximumSize);
    Print (L"Table Address:        0x%08lX\n", Ep3->TableAddress);
    
  } else {
    SMBIOS_TABLE_ENTRY_POINT *Ep2 = (SMBIOS_TABLE_ENTRY_POINT *)EntryPoint;
    
    Print (L"Architecture:         32-bit (SMBIOS 2.x)\n");
    Print (L"Anchor String:        %c%c%c%c\n", 
           (CHAR16)Ep2->AnchorString[0], (CHAR16)Ep2->AnchorString[1], 
           (CHAR16)Ep2->AnchorString[2], (CHAR16)Ep2->AnchorString[3]);
           
    Print (L"EPS Checksum:         0x%02X\n", Ep2->EntryPointStructureChecksum);
    Print (L"Entry Point Len:      %d\n", Ep2->EntryPointLength);
    Print (L"Version:              %d.%d\n", Ep2->MajorVersion, Ep2->MinorVersion);
    Print (L"Number of Structures: %d\n", Ep2->NumberOfSmbiosStructures);
    Print (L"Max Struct size:      %d\n", Ep2->MaxStructureSize);
    
    Print (L"Table Address:        0x%08X\n", Ep2->TableAddress);
    Print (L"Table Length:         %d\n", Ep2->TableLength);
    
    Print (L"Entry Point revision: 0x%X\n", Ep2->EntryPointRevision);
    Print (L"SMBIOS BCD Revision:  0x%02X\n", Ep2->SmbiosBcdRevision);

    Print (L"Inter Anchor:         %c%c%c%c%c\n", 
           (CHAR16)Ep2->IntermediateAnchorString[0], (CHAR16)Ep2->IntermediateAnchorString[1], 
           (CHAR16)Ep2->IntermediateAnchorString[2], (CHAR16)Ep2->IntermediateAnchorString[3], 
           (CHAR16)Ep2->IntermediateAnchorString[4]);
           
    Print (L"Inter Checksum:       0x%02X\n", Ep2->IntermediateChecksum);

    Print (L"Formatted Area:\n");
    DumpRawMemoryHex ((UINT8 *)Ep2->FormattedArea, sizeof (Ep2->FormattedArea));
  }
}

/**
  Dumps the active SMBIOS Type 2 Feature Flags to the UEFI console.
**/
STATIC
VOID
DumpBaseboardFeatureFlags (
  IN UINT8 FeatureFlag
  )
{
  UINT8        BitIndex;
  CONST CHAR16 *FeatureStr;
  Print (L"Base Board Feature Flags: 0x%02x\n", FeatureFlag);

  for (BitIndex = 0; BitIndex < 8; BitIndex++) {
    FeatureStr = GetBaseboardFeatureString (BitIndex);
    if ((FeatureStr != NULL) && ((FeatureFlag & (1 << BitIndex)) != 0)) {
      Print (L" |--> [Bit %d] %s\n", BitIndex, FeatureStr);
    }
  }
}

/**
  Parses and dumps the formatted section of a single SMBIOS structure based on its Type.
  Note: This function parses exactly ONE structure. Looping is handled by UefiMain.

  @param[in] SmbiosStruct  A union pointer to the current SMBIOS structure.
**/
VOID
DumpSmbiosStructure (
  IN SMBIOS_STRUCTURE_POINTER  SmbiosStruct,
  IN UINTN                     StructureIndex,
  IN UINT8                     SmbiosMajorVersion,
  IN UINT8                     SmbiosMinorVersion
  )
{
  Print (L"\n============================================================\n");
  DumpRawStructure (SmbiosStruct.Hdr, StructureIndex);
  Print (L"Type : %a \nLength : %d bytes \nHandle : %d \n", 
            GetSmbiosTypeName (SmbiosStruct.Hdr->Type),
            SmbiosStruct.Hdr->Length,
            SmbiosStruct.Hdr->Handle
          );

  switch (SmbiosStruct.Hdr->Type) {
    case SMBIOS_TYPE_BIOS_INFORMATION: // Type 0
      Print (L"Vendor: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type0->Vendor));
      Print (L"BIOS Version: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type0->BiosVersion));
      Print (L"BIOS Starting Address Segment: 0x%04X\n", SmbiosStruct.Type0->BiosSegment);
      Print (L"BIOS Release Date: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type0->BiosReleaseDate));


      /**
        Parse 64-bit BIOS Characteristics (Offset 0x0A).
        Utilizes external string table for clean bit-mask parsing.
      **/
      {
        UINT64        Char64 = *(UINT64 *)&SmbiosStruct.Type0->BiosCharacteristics;
        UINT8         BitIndex;
        CONST CHAR16  *FeatureStr;

        Print (L"BIOS Characteristics: 0x%X\n", Char64);

        // According to SMBIOS spec, if Bit 2 is set, the characteristics are unknown.
        if ((Char64 & BIT2) != 0) {
          Print (L"  |--> Unknown\n");
        } else {
          // Iterate from Bit 3 to Bit 31
          for (BitIndex = 3; BitIndex < 32; BitIndex++) {
            // Use 1ULL to safely perform 64-bit bitwise shift in C
            if ((Char64 & (1ULL << BitIndex)) != 0) {
              FeatureStr = GetBiosCharacteristicsString (BitIndex);
              if (FeatureStr != NULL) {
                Print (L"  |--> %s\n", FeatureStr);
              }
            }
          }
          
          // UINT16 PlatformFirmwareVendorBits = (UINT16)((Char64 >> 32) & 0xFFFF);
          // UINT16 SystemVendorBits           = (UINT16)((Char64 >> 48) & 0xFFFF);

          // if (PlatformFirmwareVendorBits != 0) {
          //   Print (L"  |--> Bits 32:47 are reserved for BIOS Vendor\n");
          // }
          
          // if (SystemVendorBits != 0) {
          //   Print (L"  |--> Bits 48:63 are reserved for System Vendor\n");
          // }
        }
      }

      {
        UINT8         ExtByte1 = SmbiosStruct.Type0->BIOSCharacteristicsExtensionBytes[0];
        UINT8         ExtByte2 = SmbiosStruct.Type0->BIOSCharacteristicsExtensionBytes[1];
        UINT8         BitIndex;
        CONST CHAR16  *FeatureStr;

        if (SmbiosStruct.Hdr->Length > 0x12){
          Print (L"BIOS Characteristics Extension Byte 1: 0x%02X\n", ExtByte1);
          for (BitIndex = 0; BitIndex < 8; BitIndex++) {
            FeatureStr = GetBiosExtByte1String (BitIndex);
            if ((FeatureStr != NULL) && ((ExtByte1 & (1 << BitIndex)) != 0)) {
              Print (L"  |--> %s\n", FeatureStr);
            }
          }
        }

        if (SmbiosStruct.Hdr->Length > 0x13){
          Print (L"BIOS Characteristics Extension Byte 2: 0x%02X\n", ExtByte2);
          for (BitIndex = 0; BitIndex < 8; BitIndex++) {
            FeatureStr = GetBiosExtByte2String (BitIndex);
            if ((FeatureStr != NULL) && ((ExtByte2 & (1 << BitIndex)) != 0)) {
              Print (L"  |--> %s\n", FeatureStr);
            }
          }
        }
      }


      if (AE_SMBIOS_VERSION (0x2, 0x4) && (SmbiosStruct.Hdr->Length > 0x14)) {
        Print (L"System BIOS Major Release: %d\n", SmbiosStruct.Type0->SystemBiosMajorRelease);
        Print (L"System BIOS Minor Release: %d\n", SmbiosStruct.Type0->SystemBiosMinorRelease);
         Print (L"Embedded Controller Firmware Major Release: %d\n", SmbiosStruct.Type0->EmbeddedControllerFirmwareMajorRelease);
        Print (L"Embedded Controller Firmware Minor Release: %d\n", SmbiosStruct.Type0->EmbeddedControllerFirmwareMinorRelease);
      }
      
      if (AE_SMBIOS_VERSION (0x3, 0x1) && (SmbiosStruct.Hdr->Length > 0x18)) {
        // Access offset 0x18 via raw pointer to bypass header definition issues
        UINT16 ExtSize = *((UINT16 *)((UINT8 *)SmbiosStruct.Type0 + 0x18));
        UINT16 Size    = ExtSize & 0x3FFF;       // Bits 13:0
        UINT16 Unit    = (ExtSize >> 14) & 0x03; // Bits 15:14

        Print (L"Extended BIOS ROM Size: %d %a\n", Size, (Unit == 0) ? "MB" : "GB");
      } else {
        Print (L"BIOS ROM Size: %d KB\n", (SmbiosStruct.Type0->BiosSize + 1) * 64);
      }
      break;

    case SMBIOS_TYPE_SYSTEM_INFORMATION: // Type 1
      Print (L"Manufacturer: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type1->Manufacturer));
      Print (L"Product Name: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type1->ProductName));
      Print (L"Version: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type1->Version));
      Print (L"SerialNumber: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type1->SerialNumber));
      Print (L"Dump Uuid: \n");
      DumpRawMemoryHex ((UINT8 *)&SmbiosStruct.Type1->Uuid, sizeof (GUID));
      Print (L"System Wakeup Type: %a\n", GetWakeUpTypeString (SmbiosStruct.Type1->WakeUpType));
      if (AE_SMBIOS_VERSION (0x2, 0x4) && (SmbiosStruct.Hdr->Length > 0x19)){
        Print (L"SKUNumber: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type1->SKUNumber));
        Print (L"Family: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type1->Family));
      }
      break;

    case SMBIOS_TYPE_BASEBOARD_INFORMATION: // Type 2
      Print (L"Manufacturer: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type2->Manufacturer));
      Print (L"Product Name: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type2->ProductName));
      Print (L"Version: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type2->Version));
      Print (L"SerialNumber: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type2->SerialNumber));
      if (SmbiosStruct.Hdr->Length > 0x8){
        Print (L"Asset Tag: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type2->AssetTag));
        DumpBaseboardFeatureFlags (*(UINT8 *)&SmbiosStruct.Type2->FeatureFlag);
        Print (L"Location In Chassis: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type2->LocationInChassis));
        Print (L"Chassis Handle: 0x%x\n", SmbiosStruct.Type2->ChassisHandle);
        Print (L"Base Board Board Type: %a (0x%02x)\n", 
              GetBoardTypeString (SmbiosStruct.Type2->BoardType), 
              SmbiosStruct.Type2->BoardType);
      }
      {
        UINT8  NumObjects;
        UINT8  Index;
        UINT16 *ObjectHandles;
        NumObjects = SmbiosStruct.Type2->NumberOfContainedObjectHandles;
        Print (L"Number Of Contained Object Handles: %d\n", NumObjects);
        if (NumObjects > 0) {
          ObjectHandles = SmbiosStruct.Type2->ContainedObjectHandles;
          for (Index = 0; Index < NumObjects; Index++) {
            Print (L" |--> [Object %d] Handle : 0x%04x\n", Index, ObjectHandles[Index]);
          }
        }
      }
      break;

    case SMBIOS_TYPE_SYSTEM_ENCLOSURE: // Type 3
      Print (L"Manufacturer: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type3->Manufacturer));
      {
        UINT8 RawType  = SmbiosStruct.Type3->Type;
        UINT8 LockBit  = RawType & 0x80;
        UINT8 BaseType = RawType & 0x7F;

        Print (L"Type: %d\n", RawType);
        Print (L"System Enclosure or Chassis Types:  %a\n", GetChassisTypeString (BaseType));
        
        if (LockBit != 0) {
          Print (L"Chassis Lock present\n");
        } else {
          Print (L"Chassis Lock not present\n");
        }
      }

      Print (L"Version: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type3->Version));
      Print (L"SerialNumber: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type3->SerialNumber));
      Print (L"AssetTag: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type3->AssetTag));

      Print (L"Bootup state System Enclosure or Chassis Status:  %a\n", GetChassisStateString (SmbiosStruct.Type3->BootupState));
      Print (L"Power Supply State System Enclosure or Chassis Status:  %a\n", GetChassisStateString (SmbiosStruct.Type3->PowerSupplyState));
      Print (L"Thermal state System Enclosure or Chassis Status:  %a\n", GetChassisStateString (SmbiosStruct.Type3->ThermalState));
      Print (L"Security Status System Enclosure or Chassis Security Status:  %a\n", GetChassisSecurityStatusString (SmbiosStruct.Type3->SecurityStatus));
      if (AE_SMBIOS_VERSION (0x2, 0x3)){
        if (SmbiosStruct.Hdr->Length > 0xD) {
          Print (L"Dump OemDefined\n");
          DumpRawMemoryHex ((UINT8 *)&SmbiosStruct.Type3->OemDefined, sizeof (UINT32));
        }
        
        if (SmbiosStruct.Hdr->Length > 0x11) {
          Print (L"Height: %d\n", SmbiosStruct.Type3->Height);
        }

        if (SmbiosStruct.Hdr->Length > 0x12) {
          Print (L"NumberofPowerCords: %d\n", SmbiosStruct.Type3->NumberofPowerCords);
        }

        if (SmbiosStruct.Hdr->Length > 0x13) {
          Print (L"ContainedElementCount: %d\n", SmbiosStruct.Type3->ContainedElementCount);
        }

        if (SmbiosStruct.Hdr->Length > 0x14) {
          Print (L"ContainedElementRecordLength: %d\n", SmbiosStruct.Type3->ContainedElementRecordLength);
        }

        if (SmbiosStruct.Hdr->Length > 0x15) {

        }
        
      }

      if (AE_SMBIOS_VERSION (0x02, 0x07) && (SmbiosStruct.Hdr->Length > 0x14)) {
        UINTN               SkuOffset;
        SMBIOS_TABLE_STRING SkuStringIndex;

        SkuOffset = 0x15 + (SmbiosStruct.Type3->ContainedElementCount * SmbiosStruct.Type3->ContainedElementRecordLength);

        if (SmbiosStruct.Hdr->Length > SkuOffset) {
          SkuStringIndex = *( (UINT8 *)SmbiosStruct.Type3 + SkuOffset );
          Print (L"SKUNumber: %a\n", GetStringByIndex (SmbiosStruct, SkuStringIndex));
        } else {
          Print (L"SKUNumber: Default string\n");
        }
      }
        
      
      // {
      //   UINTN               SkuOffset;
      //   SMBIOS_TABLE_STRING SkuStringIndex;

      //   SkuOffset = OFFSET_OF (SMBIOS_TABLE_TYPE3, ContainedElementRecordLength) + 1 + 
      //               (SmbiosStruct.Type3->ContainedElementCount * SmbiosStruct.Type3->ContainedElementRecordLength);

      //   if (SmbiosStruct.Hdr->Length > SkuOffset) {
      //     SkuStringIndex = *( (UINT8 *)SmbiosStruct.Type3 + SkuOffset );
      //     Print (L"SKUNumber: %a\n", GetStringByIndex (SmbiosStruct, SkuStringIndex));
      //   } else {
      //     Print (L"SKUNumber: Default string\n");
      //   }
      // }
      break;

    case SMBIOS_TYPE_PROCESSOR_INFORMATION: // Type 4
      Print (L"Socket Designation: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type4->Socket));
      Print (L"Processor Type: %a\n", GetProcessorTypeString (SmbiosStruct.Type4->ProcessorType));
      Print (L"Processor Manufacturer: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type4->ProcessorManufacturer));
      Print (L"Processor ID:\n");
      DumpRawMemoryHex ((UINT8 *)&SmbiosStruct.Type4->ProcessorId, sizeof (PROCESSOR_ID_DATA));
      Print (L"Processor Version: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type4->ProcessorVersion));
      
      {
        UINT8 RawVoltage = *(UINT8 *)&SmbiosStruct.Type4->Voltage;
        Print (L"Voltage: 0x%02x\n", RawVoltage);
        if ((RawVoltage & 0x80) != 0) {
          UINT8 ActualVoltage = RawVoltage & 0x7F;
          Print (L"Processor current voltage = %d.%d V\n", (ActualVoltage / 10), (ActualVoltage % 10));
        } else {
          Print (L"Processor current voltage = ");
          if (RawVoltage == 0) {
            Print (L"Unknown\n");
          } else {
            if ((RawVoltage & BIT0) != 0) Print (L"5.0V ");
            if ((RawVoltage & BIT1) != 0) Print (L"3.3V ");
            if ((RawVoltage & BIT2) != 0) Print (L"2.9V ");
            Print (L"(Legacy Mode)\n");
          }
        }
      }
      Print (L"External Clock: %d\n", SmbiosStruct.Type4->ExternalClock);
      Print (L"Max Speed: %d MHz\n", SmbiosStruct.Type4->MaxSpeed);
      Print (L"Current Speed: %d MHz\n", SmbiosStruct.Type4->CurrentSpeed);

      /**
        Parse Processor Status (Offset 0x18).
        Bit 6 indicates if the Socket is Populated.
        Bits 2:0 indicate the CPU Status.
      **/
      {
        UINT8 StatusByte = SmbiosStruct.Type4->Status;
        Print (L"Status: 0x%02X\n", StatusByte);
        Print (L"  |--> Socket Populated: %a\n", (StatusByte & BIT6) ? "Yes" : "No");
        Print (L"  |--> CPU Status: %s\n", GetProcessorStatusString (StatusByte & 0x07));
      }

      Print (L"Processor Upgrade: %a\n", GetProcessorUpgradeString (SmbiosStruct.Type4->ProcessorUpgrade));
      Print (L"L1 Cache Handle: 0x%04x\n", SmbiosStruct.Type4->L1CacheHandle);
      Print (L"L2 Cache Handle: 0x%04x\n", SmbiosStruct.Type4->L2CacheHandle);
      Print (L"L3 Cache Handle: 0x%04x\n", SmbiosStruct.Type4->L3CacheHandle);

      if (AE_SMBIOS_VERSION (0x2, 0x3) && (SmbiosStruct.Hdr->Length > 0x20)){
        Print (L"Serial Number: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type4->SerialNumber));
        Print (L"Asset Tag: %a\n", GetStringByIndex(SmbiosStruct, SmbiosStruct.Type4->AssetTag));
        Print (L"Part Number: %a\n", GetStringByIndex(SmbiosStruct, SmbiosStruct.Type4->PartNumber));
      }
      
      if (AE_SMBIOS_VERSION (0x2, 0x5) && (SmbiosStruct.Hdr->Length > 0x23)) {
        Print (L"Core Count: %d\n", SmbiosStruct.Type4->CoreCount);
        Print (L"Core Enabled: %d\n", SmbiosStruct.Type4->EnabledCoreCount); 
        Print (L"Thread Count: %d\n", SmbiosStruct.Type4->ThreadCount);

        UINT16 Chars = SmbiosStruct.Type4->ProcessorCharacteristics;
        UINT8  BitIdx;
        CONST CHAR16 *CharStr;

        Print (L"Processor Characteristics: 0x%04X\n", Chars);
        
        // According to SMBIOS spec, if Bit 1 is set, characteristics are Unknown.
        if ((Chars & BIT1) != 0) {
          Print (L"  |--> Unknown\n");
        } else {
          // Iterate from Bit 2 to Bit 15 (Valid characteristic flags)
          for (BitIdx = 2; BitIdx < 16; BitIdx++) {
            if ((Chars & (1 << BitIdx)) != 0) {
              CharStr = GetProcessorCharacteristicsString (BitIdx);
              if (CharStr != NULL) Print (L"  |--> %s\n", CharStr);
            }
          }
        }
      }
  
      if ((SmbiosMajorVersion >= 0x3) && (SmbiosStruct.Hdr->Length > 0x2A)) {
        Print (L"Core Count 2: %d\n", SmbiosStruct.Type4->CoreCount2);
        Print (L"Core Enabled 2: %d\n", SmbiosStruct.Type4->EnabledCoreCount2); 
        Print (L"Thread Count 2: %d\n", SmbiosStruct.Type4->ThreadCount2);
      }
      break;

    case SMBIOS_TYPE_CACHE_INFORMATION: // Type 7
      Print (L"Socket Designation: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type7->SocketDesignation));
      {
        Print (L"Cache Configuration:\n");
        UINT16 CacheConfig = SmbiosStruct.Type7->CacheConfiguration;
        
        UINT8 Mode = (CacheConfig & 0x0300) >> 8;
        if (Mode == 0)      Print (L"Write Through\n");
        else if (Mode == 1) Print (L"Write Back\n");
        else if (Mode == 2) Print (L"Varies with Memory Address\n");
        else                Print (L"Unknown\n");

        UINT8 Enabled  = (CacheConfig & 0x0080) >> 7;
        UINT8 Location = (CacheConfig & 0x0060) >> 5;
        Print (L"%a ", (Enabled != 0) ? "Enabled" : "Disabled");
        
        if (Location == 0)      Print (L"Internal\n");
        else if (Location == 1) Print (L"External\n");
        else if (Location == 2) Print (L"Reserved\n");
        else                    Print (L"Unknown\n");

        UINT8 Socketed = (CacheConfig & 0x0008) >> 3;
        Print (L"%a\n", (Socketed != 0) ? "Socketed" : "Not Socketed");

        UINT8 Level = (CacheConfig & 0x0007) + 1;
        Print (L"Level %d\n", Level);
      }
      Print (L"MaximumCacheSize: 0x%04X\n", SmbiosStruct.Type7->MaximumCacheSize);
      Print (L"InstalledSize: 0x%04X\n", SmbiosStruct.Type7->InstalledSize);
      {
        UINT16 SupportedSram = *(UINT16 *)&SmbiosStruct.Type7->SupportedSRAMType;
        UINT16 CurrentSram   = *(UINT16 *)&SmbiosStruct.Type7->CurrentSRAMType;

        Print (L"SupportedSRAMType: 0x%02x\n", SupportedSram);
        Print (L"CurrentSRAMType: 0x%02x\n", CurrentSram);
        Print (L"Cache SRAM Type:  %a\n", GetCacheSramTypeString (CurrentSram));
      }
      Print (L"CacheSpeed: 0x%x\n", SmbiosStruct.Type7->CacheSpeed);
      Print (L"Cache Error Correcting Type: %a\n", GetErrorCorrectingTypeString (SmbiosStruct.Type7->ErrorCorrectionType));
      Print (L"Cache System Cache Type:%a\n", GetSystemCacheTypeString (SmbiosStruct.Type7->SystemCacheType));
      Print (L"Cache Associativity:%a\n", GetCacheAssociativityString (SmbiosStruct.Type7->Associativity));
      if (AE_SMBIOS_VERSION (0x3, 0x1) && (SmbiosStruct.Hdr->Length > 0x13)) {
        Print (L"MaximumCacheSize2: 0x%04X\n", SmbiosStruct.Type7->MaximumCacheSize2);
        Print (L"InstalledSize2: 0x%04X\n", SmbiosStruct.Type7->InstalledSize2);
      }
      break;

    case SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION: // Type 8
      Print (L"Internal Reference Designator: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type8->InternalReferenceDesignator));
      Print (L"Internal Port Connector Type: %a (0x%02x)\n", 
            GetPortConnectorTypeString (SmbiosStruct.Type8->InternalConnectorType),
            SmbiosStruct.Type8->InternalConnectorType);
      
      Print (L"External Reference Designator: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type8->ExternalReferenceDesignator));
      Print (L"External Port Connector Type: %a (0x%02x)\n", 
            GetPortConnectorTypeString (SmbiosStruct.Type8->ExternalConnectorType),
            SmbiosStruct.Type8->ExternalConnectorType);
      
      Print (L"Port Type: %a (0x%02x)\n", 
            GetPortTypeString (SmbiosStruct.Type8->PortType),
            SmbiosStruct.Type8->PortType);
      break;

    case SMBIOS_TYPE_SYSTEM_SLOTS: // Type 9
      Print (L"Slot Designation: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type9->SlotDesignation));
      Print (L"System Slot Type: %a (0x%02x)\n", GetSlotTypeString (SmbiosStruct.Type9->SlotType), SmbiosStruct.Type9->SlotType);
      Print (L"System Slot Data Bus Width: %a (0x%02x)\n", GetSlotDataBusWidthString (SmbiosStruct.Type9->SlotDataBusWidth), SmbiosStruct.Type9->SlotDataBusWidth);
      Print (L"System Slot Current Usage: %a (0x%02x)\n", GetCurrentUsageString (SmbiosStruct.Type9->CurrentUsage), SmbiosStruct.Type9->CurrentUsage);
      Print (L"System Slot Length: %a (0x%02x)\n", GetSlotLengthString (SmbiosStruct.Type9->SlotLength), SmbiosStruct.Type9->SlotLength);
      Print (L"Slot ID: %d\n", SmbiosStruct.Type9->SlotID);

      {
        UINT8 Char1 = *(UINT8 *)&SmbiosStruct.Type9->SlotCharacteristics1;
        UINT8 Char2 = *(UINT8 *)&SmbiosStruct.Type9->SlotCharacteristics2;

        Print (L"Slot Characteristics 1: 0x%x\n", Char1);
        if (Char1 != 0 && Char1 != 0xFF) {
          if ((Char1 & BIT0) != 0) Print (L"  Characteristics unknown\n");
          if ((Char1 & BIT1) != 0) Print (L"  Provides 5.0 volts\n");
          if ((Char1 & BIT2) != 0) Print (L"  Provides 3.3 volts\n");
          if ((Char1 & BIT3) != 0) Print (L"  Slot's opening is shared with another slot\n");
          if ((Char1 & BIT4) != 0) Print (L"  PC Card slot supports PCMCIA 16-bit\n");
          if ((Char1 & BIT5) != 0) Print (L"  PC Card slot supports CardBus\n");
          if ((Char1 & BIT6) != 0) Print (L"  PC Card slot supports Zoom Video\n");
          if ((Char1 & BIT7) != 0) Print (L"  PC Card slot supports Modem Ring Resume\n");
        }

        Print (L"Slot Characteristics 2: 0x%x\n", Char2);
        if (Char2 != 0 && Char2 != 0xFF) {
          if ((Char2 & BIT0) != 0) Print (L"  PCI slot supports Power Management Event (PME#)\n");
          if ((Char2 & BIT1) != 0) Print (L"  Slot supports hot-plug devices\n");
          if ((Char2 & BIT2) != 0) Print (L"  PCI slot supports SMBus signal\n");
          if ((Char2 & BIT3) != 0) Print (L"  PCIe slot supports bifurcation\n"); 
        }
      }

      if (AE_SMBIOS_VERSION (0x2, 0x6) && (SmbiosStruct.Hdr->Length > 0xD)) {
        Print (L"Segment Group Number: 0x%x\n", SmbiosStruct.Type9->SegmentGroupNum);
        Print (L"Bus Number: 0x%x\n", SmbiosStruct.Type9->BusNum);
        Print (L"Device/Function Number: 0x%x\n", SmbiosStruct.Type9->DevFuncNum);
      }

      if (AE_SMBIOS_VERSION (0x03, 0x02)) {

        if (SmbiosStruct.Hdr->Length > 0x11) {
          UINT8 DataBusWidth = SmbiosStruct.Type9->DataBusWidth;
          Print (L"Data Bus Width: %a (0x%X)\n", GetSlotDataBusWidthString (DataBusWidth), DataBusWidth);
        }

        if (SmbiosStruct.Hdr->Length > 0x12) {
          UINT8 PeerGroupCount = SmbiosStruct.Type9->PeerGroupingCount;
          Print (L"Peer Grouping Count: %d\n", PeerGroupCount);

          // Iterate through the variable-length Peer Groups array
          if (PeerGroupCount > 0) {
            MISC_SLOT_PEER_GROUP *PeerGroupPtr = SmbiosStruct.Type9->PeerGroups;
            UINT8 Index;
            
            for (Index = 0; Index < PeerGroupCount; Index++) {
              Print (L"  |--> [Peer Group %d]\n", Index + 1);
              Print (L"  |--> Segment Group Number: 0x%04X\n", PeerGroupPtr[Index].SegmentGroupNum);
              Print (L"  |--> Bus Number: 0x%02X\n", PeerGroupPtr[Index].BusNum);
              Print (L"  |--> Device/Function Number: 0x%02X\n", PeerGroupPtr[Index].DevFuncNum);
              Print (L"  |--> Data Bus Width: %d\n", PeerGroupPtr[Index].DataBusWidth);
            }
          }

          UINTN ExpectedLength = 0x13 + (PeerGroupCount * sizeof (MISC_SLOT_PEER_GROUP)) + sizeof (SMBIOS_TABLE_TYPE9_EXTENDED);
          
          // Final boundary check to ensure the extended fields are physically present
          if (SmbiosStruct.Hdr->Length >= ExpectedLength) {
            SMBIOS_TABLE_TYPE9_EXTENDED *ExtStruct;
            
            // Push the pointer past the PeerGroups array
            ExtStruct = (SMBIOS_TABLE_TYPE9_EXTENDED *)((UINT8 *)SmbiosStruct.Type9->PeerGroups + (PeerGroupCount * sizeof (MISC_SLOT_PEER_GROUP)));

            Print (L"System Slot Information:  %a\n", GetSystemSlotInfoString (ExtStruct->SlotInformation));
            Print (L"System Slot Physical Width: %a\n", GetSystemSlotPhysicalWidthString (ExtStruct->SlotPhysicalWidth));
            Print (L"System Slot Pitch: %d\n", ExtStruct->SlotPitch);
            Print (L"System Slot Height:  %a\n", GetSystemSlotHeightString (ExtStruct->SlotHeight));
          }
        }
      }
      break;

    case SMBIOS_TYPE_OEM_STRINGS: // Type 11
      {
        UINT8 StringCount = SmbiosStruct.Type11->StringCount;
        
        Print (L"StringCount: %d\n", StringCount);

        if (StringCount > 0) {
          UINT8 Index;
          for (Index = 1; Index <= StringCount; Index++) {
            Print (L"String %d: %a\n", Index, GetStringByIndex (SmbiosStruct, Index));
          }
        }
      }
      break;
    
    case SMBIOS_TYPE_BIOS_LANGUAGE_INFORMATION: // Type 13
      Print (L"Installable Languages: %d\n", SmbiosStruct.Type13->InstallableLanguages);
      Print (L"Flags: %d\n", SmbiosStruct.Type13->Flags);
      Print (L"Dump Reserved\nsize=%d:\n", sizeof (SmbiosStruct.Type13->Reserved));
      DumpRawMemoryHex ((UINT8 *)SmbiosStruct.Type13->Reserved, sizeof (SmbiosStruct.Type13->Reserved));
      if ((SmbiosStruct.Type13->Flags & BIT0) != 0) {
        Print (L"Language Format: Abbreviated\n");
      } else {
        Print (L"Language Format: Long\n");
      }

      Print (L"Current Languages: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type13->CurrentLanguages));

      {
        UINT8 NumLangs = SmbiosStruct.Type13->InstallableLanguages;
        if (NumLangs > 0) {
          UINT8 Index;
          for (Index = 1; Index <= NumLangs; Index++) {
            Print (L"Language %d: %a\n", Index, GetStringByIndex (SmbiosStruct, Index));
          }
        }
      }
      break;

    case SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY: // Type 16
      Print (L"Physical Memory Array Location: %a (0x%02x)\n", 
            GetMemoryArrayLocationString (SmbiosStruct.Type16->Location), 
            SmbiosStruct.Type16->Location);
            
      Print (L"Physical Memory Array Use: %a (0x%02x)\n", 
            GetMemoryArrayUseString (SmbiosStruct.Type16->Use), 
            SmbiosStruct.Type16->Use);
            
      Print (L"Physical Memory Array Error Correction Type: %a (0x%02x)\n", 
            GetMemoryErrorCorrectionString (SmbiosStruct.Type16->MemoryErrorCorrection), 
            SmbiosStruct.Type16->MemoryErrorCorrection);

      {
        UINT32 MaxCapacity = SmbiosStruct.Type16->MaximumCapacity;
        
        Print (L"Maximum Capacity: ");
        if (MaxCapacity == 0x80000000) {
          Print (L"See Extended Maximum Capacity (%d)\n", MaxCapacity);
        } else {
          Print (L"%d KB\n", MaxCapacity);
        }
      }

      Print (L"Memory Error Information Handle: 0x%x\n", SmbiosStruct.Type16->MemoryErrorInformationHandle);
      Print (L"Number of Memory Devices: 0x%x\n", SmbiosStruct.Type16->NumberOfMemoryDevices);

      if (AE_SMBIOS_VERSION (0x2, 0x7) && (SmbiosStruct.Hdr->Length > 0xF)) {
        Print (L"Extended Maximum Capacity: 0x%lx\n", SmbiosStruct.Type16->ExtendedMaximumCapacity);
      }
      break;

    case SMBIOS_TYPE_MEMORY_DEVICE: // Type 17
      Print (L"Physical Memory Array Handle: 0x%X\n", SmbiosStruct.Type17->MemoryArrayHandle);
      Print (L"Memory Error Information Handle: 0x%04X\n", SmbiosStruct.Type17->MemoryErrorInformationHandle);
      Print (L"Total Width: 0x%X\n", SmbiosStruct.Type17->TotalWidth);
      Print (L"Data Width: 0x%X\n", SmbiosStruct.Type17->DataWidth);

      {
        UINT16 SizeField = SmbiosStruct.Type17->Size;
        if (SizeField == 0) {
          Print (L"Size: 0\n");
        } else if (SizeField == 0x7FFF) {
          if (SmbiosStruct.Hdr->Length > OFFSET_OF (SMBIOS_TABLE_TYPE17, ExtendedSize)) {
            Print (L"Size: 32767\n"); 
          }
        } else {
          UINT16 ActualSize = SizeField & 0x7FFF;
          Print (L"Size: %d %a\n", ActualSize, (SizeField & 0x8000) ? "KB" : "MB");
        }
      }

      Print (L"Form Factor:  %a\n", GetMemoryFormFactorString (SmbiosStruct.Type17->FormFactor));
      Print (L"Device Set: 0x%X\n", SmbiosStruct.Type17->DeviceSet);
      Print (L"Device Locator: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type17->DeviceLocator));
      Print (L"Bank Locator: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type17->BankLocator));
      Print (L"Memory Type:  %a\n", GetMemoryTypeString (SmbiosStruct.Type17->MemoryType));
      
/**
        Parse Memory Type Detail (Offset 0x13).
        This is a 16-bit field where multiple characteristics can be set.
      **/
      {
        UINT16 Detail = *(UINT16 *)&SmbiosStruct.Type17->TypeDetail;
        UINT8  BitIdx;
        CONST CHAR16 *DetailStr;

        Print (L"Type Detail: 0x%04X\n", Detail);
        
        if (Detail == 0) {
          Print (L"  |--> None\n");
        } else if ((Detail & BIT2) != 0) {
          // Bit 2 indicates the detail is Unknown
          Print (L"  |--> Unknown\n");
        } else {
          // Iterate from Bit 1 to Bit 15 (Bit 0 is reserved)
          for (BitIdx = 1; BitIdx < 16; BitIdx++) {
            // Skip Bit 2 since we already handled the Unknown state
            if (BitIdx == 2) continue; 

            if ((Detail & (1 << BitIdx)) != 0) {
              DetailStr = GetMemoryTypeDetailString (BitIdx);
              if (DetailStr != NULL) {
                Print (L"  |--> %s\n", DetailStr);
              }
            }
          }
        }
      }

      Print (L"Speed: 0x%X\n", SmbiosStruct.Type17->Speed);
      Print (L"Manufacturer: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type17->Manufacturer));
      Print (L"Serial Number: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type17->SerialNumber));
      
      if (SmbiosStruct.Type17->AssetTag == 0) {
        Print (L"Asset Tag: <null string>\n");
      } else {
        Print (L"Asset Tag: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type17->AssetTag));
      }
      
      Print (L"Part Number: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type17->PartNumber));

      if (AE_SMBIOS_VERSION (0x2, 0x7) && (SmbiosStruct.Hdr->Length > 0xF)) {
        Print (L"Attributes: 0x%X\n", SmbiosStruct.Type17->Attributes & 0x0F);
      }

      if (AE_SMBIOS_VERSION (0x2, 0x7) && (SmbiosStruct.Hdr->Length > 0x1C)) {
        Print (L"Extended Size: %d\n", SmbiosStruct.Type17->ExtendedSize);
        Print (L"Configured Memory Speed: 0x%X\n", SmbiosStruct.Type17->ConfiguredMemoryClockSpeed);
      }

      if (AE_SMBIOS_VERSION (0x2, 0x8) && (SmbiosStruct.Hdr->Length > 0x22)) {
        Print (L"Minimum voltage: %d\n", SmbiosStruct.Type17->MinimumVoltage);
        Print (L"Maximum voltage: %d\n", SmbiosStruct.Type17->MaximumVoltage);
        Print (L"Configured voltage: %d\n", SmbiosStruct.Type17->ConfiguredVoltage);
      }

      if (AE_SMBIOS_VERSION (0x03, 0x02)) {
        UINT8  *BasePtr = (UINT8 *)SmbiosStruct.Type17;

        if (SmbiosStruct.Hdr->Length > 0x28) {
          UINT8  MemTechnology = *(BasePtr + 0x28);
          UINT16 OperatingMode = *(UINT16 *)(BasePtr + 0x29);
          UINT8  FwVersionIdx  = *(BasePtr + 0x2B);
          UINT16 ModMfgId      = *(UINT16 *)(BasePtr + 0x2C);
          UINT16 ModProdId     = *(UINT16 *)(BasePtr + 0x2E);
          UINT16 CtrlMfgId     = *(UINT16 *)(BasePtr + 0x30);
          UINT16 CtrlProdId    = *(UINT16 *)(BasePtr + 0x32);

          Print (L"Memory Technology:  %a\n", GetMemoryTechnologyString (MemTechnology));
          Print (L"Memory Operating Mode Capability:  %a\n", GetMemoryOperatingModeString (OperatingMode));
          
          if (FwVersionIdx == 0) {
            Print (L"Firmware Version: Unknown\n");
          } else {
            Print (L"Firmware Version: %a\n", GetStringByIndex (SmbiosStruct, FwVersionIdx));
          }
          
          Print (L"Module Manufacturer ID: 0x%X\n", ModMfgId);
          Print (L"Module Product ID: 0x%X\n", ModProdId);
          Print (L"Memory Subsystem Controller Manufacturer ID: 0x%X\n", CtrlMfgId);
          Print (L"Memory Subsystem Controller Product ID: 0x%X\n", CtrlProdId);
        }

        if (SmbiosStruct.Hdr->Length > 0x34) {
          UINT64 NonVolatileSize = *(UINT64 *)(BasePtr + 0x34);
          Print (L"Non-volatile Size: 0x%lX\n", NonVolatileSize);
        }

        if (SmbiosStruct.Hdr->Length > 0x3C) {
          UINT64 VolatileSize = *(UINT64 *)(BasePtr + 0x3C);
          Print (L"Volatile Size: 0x%lX\n", VolatileSize);
        }

        if (SmbiosStruct.Hdr->Length > 0x44) {
          UINT64 CacheSize = *(UINT64 *)(BasePtr + 0x44);
          Print (L"Cache Size: 0x%lX\n", CacheSize);
        }

        if (SmbiosStruct.Hdr->Length > 0x4C) {
          UINT64 LogicalSize = *(UINT64 *)(BasePtr + 0x4C);
          Print (L"Logical Size: 0x%lX\n", LogicalSize);
        }
      }

      if (AE_SMBIOS_VERSION (0x03, 0x03) && (SmbiosStruct.Hdr->Length > 0x54)) {
        UINT8  *BasePtr = (UINT8 *)SmbiosStruct.Type17;
        
        UINT32 ExtSpeed           = *(UINT32 *)(BasePtr + 0x54);
        UINT32 ExtConfiguredSpeed = *(UINT32 *)(BasePtr + 0x58);

        Print (L"Extended Speed: 0x%X\n", ExtSpeed);
        Print (L"Extended Configured Memory Speed: 0x%X\n", ExtConfiguredSpeed);
      }
      break;

    case SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS: // Type 19
      {
        UINT32 StartAddr = SmbiosStruct.Type19->StartingAddress;
        UINT32 EndAddr   = SmbiosStruct.Type19->EndingAddress;

        Print (L"Starting Address: 0x%x\n", StartAddr);
        Print (L"Ending Address: 0x%x\n", EndAddr);
        Print (L"Memory Array Handle: 0x%x\n", SmbiosStruct.Type19->MemoryArrayHandle);
        Print (L"Partition Width: 0x%d\n", SmbiosStruct.Type19->PartitionWidth);

        if(AE_SMBIOS_VERSION (0x2, 0x7) && (SmbiosStruct.Hdr->Length > 0xF)){
          Print (L"Extended Starting Address: 0x%lx\n", SmbiosStruct.Type19->ExtendedStartingAddress);
          Print (L"Extended Ending Address: 0x%lx\n", SmbiosStruct.Type19->ExtendedEndingAddress);
        }
      }
      break;
    
    case SMBIOS_TYPE_IPMI_DEVICE_INFORMATION: // Type 38
      Print (L"BMC Interface Type: %a (0x%02X)\n", 
            GetIpmiInterfaceTypeString (SmbiosStruct.Type38->InterfaceType),
            SmbiosStruct.Type38->InterfaceType);
            
      Print (L"IPMI Specification Revision: %d.%d\n", 
            (SmbiosStruct.Type38->IPMISpecificationRevision >> 4) & 0x0F,
            SmbiosStruct.Type38->IPMISpecificationRevision & 0x0F);
            
      Print (L"I2C Slave Address: 0x%02X\n", SmbiosStruct.Type38->I2CSlaveAddress);
      Print (L"NV Storage Device Address: 0x%02X\n", SmbiosStruct.Type38->NVStorageDeviceAddress);
      Print (L"Base Address: 0x%X\n", SmbiosStruct.Type38->BaseAddress);

      // {
      //   UINT8 Mod = SmbiosStruct.Type38->BaseAddressModifier_InterruptInfo;
      //   Print (L"Base Address Modifier / Interrupt Info: 0x%02X\n", Mod);
        
      //   UINT8 RegSpacing = (Mod >> 6) & 0x03;
      //   Print (L"Register Spacing: ");
      //   if (RegSpacing == 0)      Print (L"Successive Byte Boundaries\n");
      //   else if (RegSpacing == 1) Print (L"32-bit Boundaries\n");
      //   else if (RegSpacing == 2) Print (L"16-bit Boundaries\n");
      //   else                      Print (L"Reserved\n");
        
      //   Print (L"Address Space: %a\n", (Mod & BIT5) ? "I/O Mapped" : "Memory Mapped");
      //   Print (L"Base Address LS-bit: %d\n", (Mod & BIT4) ? 1 : 0);
        
      //   if ((Mod & BIT3) != 0) {
      //     Print (L"Interrupt Specified: Yes\n");
      //     Print (L"Interrupt Polarity: %a\n", (Mod & BIT1) ? "Active High" : "Active Low");
      //     Print (L"Interrupt Trigger Mode: %a\n", (Mod & BIT0) ? "Level" : "Edge");
      //     Print (L"Interrupt Number: 0x%02X\n", SmbiosStruct.Type38->InterruptNumber);
      //   } else {
      //     Print (L"Interrupt Specified: No\n");
      //   }
      // }
      break;

    case SMBIOS_TYPE_SYSTEM_POWER_SUPPLY: // Type 39
      Print (L"Power Unit Group: %d\n", SmbiosStruct.Type39->PowerUnitGroup);
      Print (L"Location: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->Location));
      Print (L"Device Name: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->DeviceName));
      Print (L"Manufacturer: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->Manufacturer));
      Print (L"Serial Number: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->SerialNumber));
      Print (L"Asset Tag Number: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->AssetTagNumber));
      Print (L"Model Part Number: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->ModelPartNumber));
      Print (L"Revision Level: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type39->RevisionLevel));

      if (SmbiosStruct.Type39->MaxPowerCapacity == 0x8000) {
        Print (L"Max Power Capacity: Unknown\n");
      } else {
        Print (L"Max Power Capacity: %d W\n", SmbiosStruct.Type39->MaxPowerCapacity);
      }
      {
        UINT16 Char16 = *(UINT16 *)&SmbiosStruct.Type39->PowerSupplyCharacteristics;
        Print (L"Power Supply Characteristics: 0x%04X\n", Char16);

        if ((Char16 & BIT0) != 0) {
          Print (L"  |--> Hot Replaceable: Yes\n");
        } else {
          Print (L"  |--> Hot Replaceable: No\n");
        }

        if ((Char16 & BIT1) != 0) {
          Print (L"  |--> Power Supply Present: Yes\n");
        } else {
          Print (L"  |--> Power Supply Present: No\n");
        }

        if ((Char16 & BIT2) != 0) {
          Print (L"  |--> Power Supply Unplugged From The Wall: Yes\n");
        } else {
          Print (L"  |--> Power Supply Unplugged From The Wall: No\n");
        }

        Print (L"  |--> Input Voltage Range Switching: %a\n", 
              GetPowerSupplyInputVoltageRangeString ((Char16 >> 3) & 0x0F));

        Print (L"  |--> Status: %a\n", 
              GetPowerSupplyStatusString ((Char16 >> 7) & 0x07));

        Print (L"  |--> Power Supply Type: %a\n", 
              GetPowerSupplyTypeString ((Char16 >> 10) & 0x0F));

        if ((Char16 & 0xC000) != 0) {
          Print (L"  |--> [Reserved Bits 15:14 are set]\n");
        }
      }

      Print (L"Input Voltage Probe Handle: 0x%04X\n", SmbiosStruct.Type39->InputVoltageProbeHandle);
      Print (L"Cooling Device Handle: 0x%04X\n", SmbiosStruct.Type39->CoolingDeviceHandle);
      Print (L"Input Current Probe Handle: 0x%04X\n", SmbiosStruct.Type39->InputCurrentProbeHandle);
      break;

    case SMBIOS_TYPE_ONBOARD_DEVICES_EXTENDED_INFORMATION: // Type 41
      Print (L"Reference Designation: %a\n", GetStringByIndex (SmbiosStruct, SmbiosStruct.Type41->ReferenceDesignation));
      {
        UINT8 DevTypeByte = SmbiosStruct.Type41->DeviceType;
        UINT8 Status      = (DevTypeByte & BIT7) >> 7;
        UINT8 TypeOfDev   = DevTypeByte & 0x7F;

        Print (L"Device Status: %a\n", (Status != 0) ? "Enabled" : "Disabled");
        Print (L"Device Type: %a (0x%02X)\n", GetOnboardDeviceTypeString (TypeOfDev), TypeOfDev);
      }

      Print (L"Device Type Instance: 0x%d\n", SmbiosStruct.Type41->DeviceTypeInstance);
      Print (L"Segment Group Number: 0x%04X\n", SmbiosStruct.Type41->SegmentGroupNum);
      Print (L"Bus Number: 0x%02X\n", SmbiosStruct.Type41->BusNum);

      {
        UINT8 DevFunc = SmbiosStruct.Type41->DevFuncNum;
        UINT8 DeviceNum   = (DevFunc >> 3) & 0x1F;
        UINT8 FunctionNum = DevFunc & 0x07;
        Print (L"Device/Function Number: 0x%X (Dev: 0x%X, Func: 0x%X)\n", DevFunc, DeviceNum, FunctionNum);
      }
      break;

    case SMBIOS_TYPE_MANAGEMENT_CONTROLLER_HOST_INTERFACE: // Type 42
      {
        UINT8 *BasePtr = (UINT8 *)SmbiosStruct.Hdr;

        if (SmbiosStruct.Hdr->Length > 0x04) {
          UINT8 InterfaceType = *(BasePtr + 0x04);
          Print (L"Management Controller Interface Type: %a (0x%02X)\n", 
                GetMcHostInterfaceTypeString (InterfaceType), 
                InterfaceType);
        }

        if (AE_SMBIOS_VERSION (0x3, 0x2) && SmbiosStruct.Hdr->Length > 0x05) {
          UINT8 SpecDataLen = *(BasePtr + 0x05);
          Print (L"Interface Specific Data Length: 0x%X\n", SpecDataLen);

          if (SpecDataLen > 0 && SmbiosStruct.Hdr->Length >= (0x06 + SpecDataLen)) {
            DumpInterfaceSpecificData (BasePtr + 0x06, SpecDataLen);
          }
        }
      }
      break;

    case SMBIOS_TYPE_TPM_DEVICE: // Type 43
      {
        Print (L"Vendor ID: %c%c%c%c\n",
                (CHAR16)SmbiosStruct.Type43->VendorID[0],
                (CHAR16)SmbiosStruct.Type43->VendorID[1],
                (CHAR16)SmbiosStruct.Type43->VendorID[2],
                (CHAR16)SmbiosStruct.Type43->VendorID[3]
              );
        Print (L"Major Spec Version: %a\n", SmbiosStruct.Type43->MajorSpecVersion);
        Print (L"Minor Spec Version: %a\n", SmbiosStruct.Type43->MinorSpecVersion);
        Print (L"Firmware Version 1: 0x%08x\n", SmbiosStruct.Type43->FirmwareVersion1);
        Print (L"Firmware Version 2: 0x%08x\n", SmbiosStruct.Type43->FirmwareVersion2);
        Print (L"Description: %a\n", GetStringByIndex(SmbiosStruct, SmbiosStruct.Type43->Description));
        
        UINT64 Char64 = SmbiosStruct.Type43->Characteristics;
        Print (L"Characteristics: 0x%016lX\n", Char64);
        if ((Char64 & BIT2) != 0) {
          Print (L"TPM Device Characteristics are not supported\n");
        } else {
          if ((Char64 & BIT3) != 0) Print (L"Family configurable via firmware update\n");
          if ((Char64 & BIT4) != 0) Print (L"Family configurable via platform software\n");
          if ((Char64 & BIT5) != 0) Print (L"Family configurable via OEM proprietary mechanism\n");
        }

        Print (L"Oem-defined: 0x%08X\n", SmbiosStruct.Type43->OemDefined);
      }
      break;

    case SMBIOS_TYPE_END_OF_TABLE: // Type 127
      {
        Print (L"This structure indicates the End-of-table!\n"); 
      }
      break;

    default:
      Print (L"[Unparsed/Ignored Type]\n");
      break;
  }
}

/**
  @brief Processes and dumps a complete SMBIOS table starting from its Entry Point.

  @param[in] EntryPoint     Pointer to the SMBIOS Entry Point structure.
  @param[in] IsSmbios3      TRUE if processing a 64-bit (_SM3_) table.
  @param[in] TargetType     The specific SMBIOS Type to filter for.
  @param[in] FilterEnabled  TRUE if type filtering is enabled.
**/
VOID
ProcessSmbiosTable (
  IN VOID     *EntryPoint,
  IN BOOLEAN  IsSmbios3,
  IN UINT32   TargetType,
  IN BOOLEAN  FilterEnabled
  )
{
  SMBIOS_STRUCTURE_POINTER  SmbiosStruct;
  UINT8                     *RawPtr = NULL;
  UINTN                     StructureIndex = 0;
  UINT8                     SmbiosMajorVersion;
  UINT8                     SmbiosMinorVersion;

  DumpSmbiosEntryPoint (EntryPoint, IsSmbios3);

  // Extract Table Address AND Versions based on Architecture
  if (IsSmbios3) {
    SMBIOS_TABLE_3_0_ENTRY_POINT *Ep3 = (SMBIOS_TABLE_3_0_ENTRY_POINT *)EntryPoint;
    SmbiosStruct.Raw   = (UINT8 *)(UINTN)Ep3->TableAddress;
    SmbiosMajorVersion = Ep3->MajorVersion;
    SmbiosMinorVersion = Ep3->MinorVersion;
  } else {
    SMBIOS_TABLE_ENTRY_POINT *Ep2 = (SMBIOS_TABLE_ENTRY_POINT *)EntryPoint;
    SmbiosStruct.Raw   = (UINT8 *)(UINTN)Ep2->TableAddress;
    SmbiosMajorVersion = Ep2->MajorVersion;
    SmbiosMinorVersion = Ep2->MinorVersion;
  }

  // Main traversal loop
  while (TRUE) {
    if ((!FilterEnabled) || (SmbiosStruct.Hdr->Type == TargetType)) {

      DumpSmbiosStructure (SmbiosStruct, StructureIndex, SmbiosMajorVersion, SmbiosMinorVersion);
      
      Print (L"\n"); 
    }

    if (SmbiosStruct.Hdr->Type == SMBIOS_TYPE_END_OF_TABLE) {
      break;
    }

    RawPtr = SmbiosStruct.Raw + SmbiosStruct.Hdr->Length;
    while (!(*RawPtr == 0 && *(RawPtr + 1) == 0)) {
      RawPtr++;
    }
    
    SmbiosStruct.Raw = RawPtr + 2;
    StructureIndex++; 
  }
}

/**
  @brief The user Entry Point for the Application. 
  
  Orchestrates the flow of parameter parsing, SMBIOS Entry Point location, 
  structure traversal, and output generation.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval EFI_NOT_FOUND     The SMBIOS Table could not be located.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  VOID    *Smbios2Table = NULL; 
  VOID    *Smbios3Table = NULL; 
  UINT32  TargetType = 0;
  BOOLEAN FilterEnabled = FALSE;

  Print (L"=== EDKII SMBIOS Dump Tool ===\n");

  // Parse command line arguments
  ParseCommandLine (ImageHandle, &TargetType, &FilterEnabled);

  // Locate BOTH Entry Points from the System Configuration Table
  GetSmbiosEntryPoint (&Smbios2Table, &Smbios3Table);

  if (Smbios2Table == NULL && Smbios3Table == NULL) {
    Print (L"[Error] No SMBIOS Entry Points found in Configuration Table.\n");
    return EFI_NOT_FOUND;
  }

  // Sequentially process SMBIOS 2.x (_SM_) if it exists
  if (Smbios2Table != NULL) {
    Print (L"\n============================================================\n");
    Print (L"Found SMBIOS 2.x (32-bit) Entry Point. Starting dump...\n");
    Print (L"============================================================\n\n");
    ProcessSmbiosTable (Smbios2Table, FALSE, TargetType, FilterEnabled);
  }

  // Sequentially process SMBIOS 3.x (_SM3_) if it exists
  if (Smbios3Table != NULL) {
    Print (L"\n============================================================\n");
    Print (L"Found SMBIOS 3.x (64-bit) Entry Point. Starting dump...\n");
    Print (L"============================================================\n\n");
    ProcessSmbiosTable (Smbios3Table, TRUE, TargetType, FilterEnabled);
  }

  Print (L"\n=== SMBIOS Dump Complete ===\n");
  return EFI_SUCCESS;
}