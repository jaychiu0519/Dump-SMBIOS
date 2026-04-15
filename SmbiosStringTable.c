/** @file
  Implementation file for SMBIOS Type to String mapping module.
  
  This module utilizes a static lookup table (LUT) to translate DMTF SMBIOS
  structure types into human-readable strings, supporting modern configurations
  including PCIe 4.0/5.0 topology records.

  Copyright (c) 2026, OEM Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "SmbiosStringTable.h"
#include <Library/BaseLib.h>
#include <IndustryStandard/Smbios.h>

/**
  Structure definition for mapping an SMBIOS Type integer to its corresponding
  descriptive ASCII string.
**/
typedef struct {
  UINT8         Type;
  CONST CHAR8   *Name;
} SMBIOS_TYPE_STRING_MAPPING;

/**
  Static lookup table containing the mappings defined by the SMBIOS specification.
  This table is deliberately kept static to enforce encapsulation.
**/
STATIC CONST SMBIOS_TYPE_STRING_MAPPING mSmbiosTypeStrings[] = {
  { SMBIOS_TYPE_BIOS_INFORMATION,                     "BIOS Information" },
  { SMBIOS_TYPE_SYSTEM_INFORMATION,                   "System Information" },
  { SMBIOS_TYPE_BASEBOARD_INFORMATION,                "Base Board Information" },
  { SMBIOS_TYPE_SYSTEM_ENCLOSURE,                     "System Enclosure" },
  { SMBIOS_TYPE_PROCESSOR_INFORMATION,                "Processor Information" },
  { SMBIOS_TYPE_CACHE_INFORMATION,                    "Cache Information" },
  { SMBIOS_TYPE_PORT_CONNECTOR_INFORMATION,           "Port Connector Information" },
  { SMBIOS_TYPE_SYSTEM_SLOTS,                         "System Slots" }, // Critical for PCIe 4.0/5.0 topologies
  { SMBIOS_TYPE_OEM_STRINGS,                          "OEM Strings" },
  { SMBIOS_TYPE_BIOS_LANGUAGE_INFORMATION,            "BIOS Language Information" },
  { SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY,                "Physical Memory Array" },
  { SMBIOS_TYPE_MEMORY_DEVICE,                        "Memory Device" },
  { SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS,          "Memory Array Mapped Address" },
  { SMBIOS_TYPE_IPMI_DEVICE_INFORMATION,              "IPMI Device Information" },
  { SMBIOS_TYPE_SYSTEM_POWER_SUPPLY,                  "System Power Supply" },
  { SMBIOS_TYPE_ONBOARD_DEVICES_EXTENDED_INFORMATION, "Onboard Devices Extended Information" },
  { SMBIOS_TYPE_MANAGEMENT_CONTROLLER_HOST_INTERFACE, "Management Controller Host Interface" },
  { SMBIOS_TYPE_TPM_DEVICE,                           "TPM Device Information" },
  { SMBIOS_TYPE_END_OF_TABLE,                         "End-of-Table" }
};

CONST CHAR8 *
GetSmbiosTypeName (
  IN UINT8 Type
  )
{
  UINTN Index;
  UINTN TableSize;

  TableSize = ARRAY_SIZE (mSmbiosTypeStrings);

  for (Index = 0; Index < TableSize; Index++) {
    if (mSmbiosTypeStrings[Index].Type == Type) {
      return mSmbiosTypeStrings[Index].Name;
    }
  }

  return "Unknown/OEM Specific Type";
}

/**
  String table for BIOS Characteristics (Offset 0x0A, 64-bit field).
  Index maps directly to the bit position (0 to 31).
  Bits 32-63 are reserved for BIOS/System vendor specific features.
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR16 *mBiosCharacteristicsStrings[32] = {
  NULL,                                                             // Bit 0: Reserved
  NULL,                                                             // Bit 1: Reserved
  L"Unknown",                                                       // Bit 2
  L"BIOS Characteristics are not supported",                        // Bit 3
  L"ISA is supported",                                              // Bit 4
  L"MCA is supported",                                              // Bit 5
  L"EISA is supported",                                             // Bit 6
  L"PCI is supported",                                              // Bit 7
  L"PC card (PCMCIA) is supported",                                 // Bit 8
  L"Plug and Play is supported",                                    // Bit 9
  L"APM is supported",                                              // Bit 10
  L"BIOS is Upgradeable(FLASH)",                                   // Bit 11
  L"BIOS shadowing is allowed",                                     // Bit 12
  L"VL-VESA is supported",                                          // Bit 13
  L"ESCD support is available",                                     // Bit 14
  L"Boot from CD is supported",                                     // Bit 15
  L"Selectable Boot is supported",                                  // Bit 16
  L"BIOS ROM is socketed",                                          // Bit 17
  L"Boot from PC card (PCMCIA) is supported",                       // Bit 18
  L"EDD (Enhanced Disk Driver) Specification is supported",                                              // Bit 19
  L"Int 13h - Japanese floppy for NEC 9800 1.2 MB is supported",    // Bit 20
  L"Int 13h - Japanese floppy for Toshiba 1.2 MB is supported",     // Bit 21
  L"Int 13h - 5.25\" / 360 KB floppy services are supported",       // Bit 22
  L"Int 13h - 5.25\" / 1.2 MB floppy services are supported",       // Bit 23
  L"Int 13h - 3.5\" / 720 KB floppy services are supported",        // Bit 24
  L"Int 13h - 3.5\" / 2.88 MB floppy services are supported",       // Bit 25
  L"Int 5h, print screen Service is supported",                     // Bit 26
  L"Int 9h, 8042 keyboard services are supported",                  // Bit 27
  L"Int 14h, serial services are supported",                        // Bit 28
  L"Int 17h, printer services are supported",                       // Bit 29
  L"Int 10h, CGA/Mono Video Services are supported",                // Bit 30
  L"NEC PC-98"                                                      // Bit 31
};

/**
  @brief Retrieves the description for a specific bit in BIOS Characteristics.
  @param[in] BitIndex  The bit position to query (0-31).
  @return A pointer to the description string, or NULL if reserved/undefined.
**/
CONST CHAR16 *
GetBiosCharacteristicsString (
  IN UINT8 BitIndex
  )
{
  if (BitIndex < 32) {
    return mBiosCharacteristicsStrings[BitIndex];
  }
  return NULL;
}

//=============================================================================
// SMBIOS Type 0: BIOS Information Strings
//=============================================================================

/**
  String table for BIOS Characteristics Extension Byte 1 (Offset 0x12).
  Index maps directly to the bit position (0 to 7).
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR16 *mBiosExtByte1Strings[8] = {
  L"ACPI is supported",                            // Bit 0
  L"USB Legacy is supported",                      // Bit 1
  L"AGP is supported",                             // Bit 2
  L"I2O boot is supported",                        // Bit 3
  L"LS-120 SuperDisk boot is supported",           // Bit 4
  L"ATAPI ZIP drive boot is supported",            // Bit 5
  L"1394 boot is supported",                       // Bit 6
  L"Smart battery is supported"                    // Bit 7
};

/**
  String table for BIOS Characteristics Extension Byte 2 (Offset 0x13).
  Index maps directly to the bit position (0 to 7).
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR16 *mBiosExtByte2Strings[8] = {
  L"BIOS Boot Specification is supported",             // Bit 0
  L"Function key-initiated network boot is supported", // Bit 1
  L"Enable Targeted Content Distribution",         // Bit 2
  L"UEFI Specification is supported",                                // Bit 3
  L"SMBIOS table describes a virtual machine",         // Bit 4
  L"Manufacturing mode is supported",                  // Bit 5
  L"Manufacturing mode is enabled",                    // Bit 6
  L"Bits 5:7 are reserved for future assignment"       // Bit 7: Reserved
};

/**
  @brief Retrieves the description for a specific bit in BIOS Extension Byte 1.
  @param[in] BitIndex  The bit position to query (0-7).
  @return A pointer to the description string, or NULL if reserved/undefined.
**/
CONST CHAR16 *
GetBiosExtByte1String (
  IN UINT8 BitIndex
  )
{
  if (BitIndex < 8) {
    return mBiosExtByte1Strings[BitIndex];
  }
  return NULL;
}

/**
  @brief Retrieves the description for a specific bit in BIOS Extension Byte 2.
  @param[in] BitIndex  The bit position to query (0-7).
  @return A pointer to the description string, or NULL if reserved/undefined.
**/
CONST CHAR16 *
GetBiosExtByte2String (
  IN UINT8 BitIndex
  )
{
  if (BitIndex < 8) {
    return mBiosExtByte2Strings[BitIndex];
  }
  return NULL;
}

/**
  Structure definition for mapping an 8-bit Wake-up Type value to its descriptive string.
**/
typedef struct {
  UINT8         Value;
  CONST CHAR8   *Description;
} WAKEUP_TYPE_ITEM;

/**
  Static lookup table for SMBIOS Type 1 Wake-up Types.
  This uniform data-driven approach ensures consistency across the entire 
  string table module.
**/
STATIC CONST WAKEUP_TYPE_ITEM mWakeUpTypeTable[] = {
  { 0x00, "Reserved" },
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "APM Timer" },
  { 0x04, "Modem Ring" },
  { 0x05, "LAN Remote" },
  { 0x06, "Power Switch" },
  { 0x07, "PCI PME#" },          // Crucial for Wake-on-LAN via PCIe 4.0/5.0 NICs
  { 0x08, "AC Power Restored" }
};

/**
  Converts the SMBIOS Type 1 Wake-up Type enumeration to a descriptive ASCII string.
  Based on DMTF SMBIOS Specification Section 7.2.2.

  @param[in] WakeUpType  The 8-bit wake-up type value from the SMBIOS Type 1 structure.

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetWakeUpTypeString (
  IN UINT8 WakeUpType
  )
{
  UINTN Index;
  UINTN TableSize;

  TableSize = ARRAY_SIZE (mWakeUpTypeTable);

  //
  // Iterate through the lookup table to find an exact match for the wake-up event.
  //
  for (Index = 0; Index < TableSize; Index++) {
    if (mWakeUpTypeTable[Index].Value == WakeUpType) {
      return mWakeUpTypeTable[Index].Description;
    }
  }

  //
  // Fallback for undefined or OEM-specific values.
  //
  return "Reserved/OEM Specific";
}
/**
  Structure definition for mapping an 8-bit Board Type value to its descriptive string.
**/
typedef struct {
  UINT8         Value;
  CONST CHAR8   *Description;
} BOARD_TYPE_ITEM;

/**
  Static lookup table for SMBIOS Type 2 Board Types.
  This data-driven approach replaces legacy switch-case structures, 
  allowing seamless integration of OEM-specific board types (e.g., 
  custom PCIe 5.0 expansion backplanes) without altering the logic.
**/
STATIC CONST BOARD_TYPE_ITEM mBoardTypeTable[] = {
  { 0x01, "Unknown" },
  { 0x02, "Other" },
  { 0x03, "Server Blade" },
  { 0x04, "Connectivity Switch" },
  { 0x05, "System Management Module" },
  { 0x06, "Processor Module" },
  { 0x07, "I/O Module" },                 // Critical for PCIe 4.0/5.0 Expansion Boards
  { 0x08, "Memory Module" },
  { 0x09, "Daughter board" },
  { 0x0A, "Motherboard (includes processor, memory, and I/O)" },
  { 0x0B, "Processor/Memory Module" },
  { 0x0C, "Processor/IO Module" },
  { 0x0D, "Interconnect board" }
};

/**
  Converts the SMBIOS Type 2 Board Type enumeration to a descriptive ASCII string.
  Based on DMTF SMBIOS Specification Section 7.3.1.

  @param[in] BoardType  The 8-bit board type value from the SMBIOS Type 2 structure.

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetBoardTypeString (
  IN UINT8 BoardType
  )
{
  UINTN Index;
  UINTN TableSize;

  TableSize = ARRAY_SIZE (mBoardTypeTable);

  // Iterate through the lookup table to find an exact match
  for (Index = 0; Index < TableSize; Index++) {
    if (mBoardTypeTable[Index].Value == BoardType) {
      return mBoardTypeTable[Index].Description;
    }
  }

  // Fallback for undefined or OEM-specific values (e.g., 0x80 - 0xFF)
  return "Reserved/OEM Specific";
}

/**
  Structure definition for mapping a bit index to its descriptive string.
**/
typedef struct {
  UINT8         BitIndex;
  CONST CHAR16  *Description;
} FEATURE_FLAG_ITEM;

/**
  Static lookup table for SMBIOS Type 2 Baseboard Feature Flags.
  Maintained internally to prevent namespace pollution.
**/
STATIC CONST FEATURE_FLAG_ITEM mBaseBoardFeatureFlagsTable[] = {
  { 0, L"Hosting board" },
  { 1, L"Requires at least one daughter board or auxiliary card" },
  { 2, L"Removable" },
  { 3, L"Replaceable" },
  { 4, L"Hot swappable" } // Critical flag for modern PCIe 4.0/5.0 NVMe or CXL hot-plug backplanes
};

CONST CHAR16 *
GetBaseboardFeatureString (
  IN UINT8 BitIndex
  )
{
  UINTN Index;
  UINTN TableSize;

  TableSize = ARRAY_SIZE (mBaseBoardFeatureFlagsTable);

  for (Index = 0; Index < TableSize; Index++) {
    if (mBaseBoardFeatureFlagsTable[Index].BitIndex == BitIndex) {
      return mBaseBoardFeatureFlagsTable[Index].Description;
    }
  }

  // Return NULL explicitly if the bit is not defined in our lookup table
  return NULL; 
}

// =====================================================================
// SMBIOS Type 3 (System Enclosure) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE3_ITEM;

STATIC CONST TYPE3_ITEM mChassisTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Desktop" },
  { 0x04, "Low Profile Desktop" },
  { 0x05, "Pizza Box" },
  { 0x06, "Mini Tower" },
  { 0x07, "Tower" },
  { 0x08, "Portable" },
  { 0x09, "Laptop" },
  { 0x0A, "Notebook" },
  { 0x0B, "Hand Held" },
  { 0x0C, "Docking Station" },
  { 0x0D, "All in One" },
  { 0x0E, "Sub Notebook" },
  { 0x0F, "Space-saving" },
  { 0x10, "Lunch Box" },
  { 0x11, "Main Server Chassis" },
  { 0x12, "Expansion Chassis" }, // Critical for PCIe 4.0/5.0 Expansion Nodes
  { 0x13, "SubChassis" },
  { 0x14, "Bus Expansion Chassis" },
  { 0x15, "Peripheral Chassis" },
  { 0x16, "RAID Chassis" },
  { 0x17, "Rack Mount Chassis" },
  { 0x18, "Sealed-case PC" },
  { 0x19, "Multi-system chassis" },
  { 0x1A, "Compact PCI" },
  { 0x1B, "Advanced TCA" },
  { 0x1C, "Blade" },
  { 0x1D, "Blade Enclosure" }
};

STATIC CONST TYPE3_ITEM mChassisStateTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Safe" },
  { 0x04, "Warning" },
  { 0x05, "Critical" },
  { 0x06, "Non-recoverable" }
};

STATIC CONST TYPE3_ITEM mChassisSecurityStatusTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "None" },
  { 0x04, "External interface locked out" },
  { 0x05, "External interface enabled" }
};

CONST CHAR8 *
GetChassisTypeString (
  IN UINT8 ChassisType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mChassisTypeTable); Index++) {
    if (mChassisTypeTable[Index].Value == ChassisType) {
      return mChassisTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetChassisStateString (
  IN UINT8 State
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mChassisStateTable); Index++) {
    if (mChassisStateTable[Index].Value == State) {
      return mChassisStateTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetChassisSecurityStatusString (
  IN UINT8 SecurityStatus
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mChassisSecurityStatusTable); Index++) {
    if (mChassisSecurityStatusTable[Index].Value == SecurityStatus) {
      return mChassisSecurityStatusTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 4 (Processor Information) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE4_ITEM;

STATIC CONST TYPE4_ITEM mProcessorTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Central Processor" },
  { 0x04, "Math Processor" },
  { 0x05, "DSP Processor" },
  { 0x06, "Video Processor" }
};


CONST CHAR8 *
GetProcessorTypeString (
  IN UINT8 ProcessorType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mProcessorTypeTable); Index++) {
    if (mProcessorTypeTable[Index].Value == ProcessorType) {
      return mProcessorTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 4 (Processor Information) - Processor Upgrade Lookup Table
// =====================================================================

STATIC CONST TYPE4_ITEM mProcessorUpgradeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Daughter Board" },
  { 0x04, "ZIF Socket" },
  { 0x05, "Replaceable Piggy Back" },
  { 0x06, "None" },
  { 0x07, "LIF Socket" },
  { 0x08, "Slot 1" },
  { 0x09, "Slot 2" },
  { 0x0A, "370-pin socket" },
  { 0x0B, "Slot A" },
  { 0x0C, "Slot M" },
  { 0x0D, "Socket 423" },
  { 0x0E, "Socket A" },
  { 0x0F, "Socket 478" },
  { 0x10, "Socket 754" },
  { 0x11, "Socket 940" },
  { 0x12, "Socket 939" },
  { 0x13, "Socket mPGA604" },
  { 0x14, "Socket LGA771" },
  { 0x15, "Socket LGA775" },
  { 0x16, "Socket S1" },
  { 0x17, "Socket AM2" },
  { 0x18, "Socket F" },
  { 0x19, "Socket LGA1366" },
  { 0x1A, "Socket G34" },
  { 0x1B, "Socket AM3" },
  { 0x1C, "Socket C32" },
  { 0x1D, "Socket LGA1156" },
  { 0x1E, "Socket LGA1567" },
  { 0x1F, "Socket PGA988A" },
  { 0x20, "Socket BGA1288" },
  { 0x21, "Socket rPGA988B" },
  { 0x22, "Socket BGA1023" },
  { 0x23, "Socket BGA1224" },
  { 0x24, "Socket LGA1155" },
  { 0x25, "Socket LGA1356" },
  { 0x26, "Socket LGA2011" },
  { 0x27, "Socket FS1" },
  { 0x28, "Socket FS2" },
  { 0x29, "Socket FM1" },
  { 0x2A, "Socket FM2" },
  { 0x2B, "Socket LGA2011-3" },
  { 0x2C, "Socket LGA1356-3" },
  { 0x2D, "Socket LGA1150" },
  { 0x2E, "Socket BGA1168" },
  { 0x2F, "Socket BGA1234" },
  { 0x30, "Socket BGA1364" },
  { 0x31, "Socket AM4" },
  { 0x32, "Socket LGA1151" },
  { 0x33, "Socket BGA1356" },
  { 0x34, "Socket BGA1440" },
  { 0x35, "Socket BGA1515" },
  { 0x36, "Socket LGA3647-1" },
  { 0x37, "Socket SP3" },
  { 0x38, "Socket SP3r2" },
  { 0x39, "Socket LGA2066" },
  { 0x3A, "Socket BGA1392" },
  { 0x3B, "Socket BGA1510" },
  { 0x3C, "Socket BGA1528" },
  { 0x3D, "Socket LGA4189" },
  { 0x3E, "Socket LGA1200" },
  { 0x3F, "Socket LGA4677" }
};

/**
  Converts the SMBIOS Type 4 Processor Upgrade enumeration to a Unicode string.

  @param[in] ProcessorUpgrade  The 8-bit processor upgrade value.

  @return A constant pointer to the descriptive Unicode string.
**/
CONST CHAR8 *
GetProcessorUpgradeString (
  IN UINT8 ProcessorUpgrade
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mProcessorUpgradeTable); Index++) {
    if (mProcessorUpgradeTable[Index].Value == ProcessorUpgrade) {
      return mProcessorUpgradeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

//=============================================================================
// SMBIOS Type 4: Processor Information Strings
//=============================================================================

/**
  String table for Processor Status (Bits 2:0 of Offset 0x18).
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR16 *mProcessorStatusStrings[8] = {
  L"Unknown",                              // 000b
  L"CPU Enabled",                          // 001b
  L"CPU Disabled by User via BIOS Setup",  // 010b
  L"CPU Disabled by BIOS (POST Error)",    // 011b
  L"CPU is Idle, waiting to be enabled",   // 100b
  L"Reserved",                             // 101b
  L"Reserved",                             // 110b
  L"Other"                                 // 111b
};

/**
  String table for Processor Characteristics (Offset 0x26, 16-bit field).
  Index maps directly to the bit position (0 to 15).
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR16 *mProcessorCharacteristicsStrings[16] = {
  L"Reserved",                             // Bit 0
  L"Unknown",                              // Bit 1
  L"64-bit Capable",                       // Bit 2
  L"Multi-Core",                           // Bit 3
  L"Hardware Thread",                      // Bit 4
  L"Execute Protection",                   // Bit 5
  L"Enhanced Virtualization",              // Bit 6
  L"Power/Performance Control",            // Bit 7
  L"128-bit Capable",                      // Bit 8
  L"Arm64 SoC ID",                         // Bit 9
  NULL, NULL, NULL, NULL, NULL, NULL       // Bits 10-15: Reserved
};

/**
  @brief Retrieves the description for Processor Status (Bits 2:0).
  @param[in] StatusValue  The 3-bit status value (0-7).
**/
CONST CHAR16 *
GetProcessorStatusString (
  IN UINT8 StatusValue
  )
{
  if (StatusValue < 8) {
    return mProcessorStatusStrings[StatusValue];
  }
  return NULL;
}

/**
  @brief Retrieves the description for a specific bit in Processor Characteristics.
  @param[in] BitIndex  The bit position to query (0-15).
**/
CONST CHAR16 *
GetProcessorCharacteristicsString (
  IN UINT8 BitIndex
  )
{
  if (BitIndex < 16) {
    return mProcessorCharacteristicsStrings[BitIndex];
  }
  return NULL;
}

// =====================================================================
// SMBIOS Type 7 (Cache Information) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE7_ITEM;

typedef struct {
  UINT16      BitMask;
  CONST CHAR8 *Description;
} SRAM_TYPE_ITEM;

STATIC CONST SRAM_TYPE_ITEM mSramTypeTable[] = {
  { BIT0, "Other" },
  { BIT1, "Unknown" },
  { BIT2, "Non-Burst" },
  { BIT3, "Burst" },
  { BIT4, "Pipeline Burst" }, // Matches 0x10
  { BIT5, "Synchronous" },
  { BIT6, "Asynchronous" }
};

STATIC CONST TYPE7_ITEM mErrorCorrectingTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "None" },
  { 0x04, "Parity" },
  { 0x05, "Single-bit ECC" },
  { 0x06, "Multi-bit ECC" }
};

STATIC CONST TYPE7_ITEM mSystemCacheTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Instruction" },
  { 0x04, "Data" },
  { 0x05, "Unified" }
};

STATIC CONST TYPE7_ITEM mCacheAssociativityTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Direct Mapped" },
  { 0x04, "2-way Set-Associative" },
  { 0x05, "4-way Set-Associative" },
  { 0x06, "Fully Associative" },
  { 0x07, "8-way Set-Associative" },
  { 0x08, "16-way Set-Associative" },
  { 0x09, "12-way Set-Associative" },
  { 0x0A, "24-way Set-Associative" },
  { 0x0B, "32-way Set-Associative" },
  { 0x0C, "48-way Set-Associative" },
  { 0x0D, "64-way Set-Associative" },
  { 0x0E, "20-way Set-Associative" }
};

CONST CHAR8 *
GetCacheSramTypeString (
  IN UINT16 SramType
  )
{
  UINTN Index;
  // Iterate to find the active SRAM bit
  for (Index = 0; Index < ARRAY_SIZE (mSramTypeTable); Index++) {
    if ((SramType & mSramTypeTable[Index].BitMask) != 0) {
      return mSramTypeTable[Index].Description;
    }
  }
  return "Unknown";
}

CONST CHAR8 *
GetErrorCorrectingTypeString (
  IN UINT8 ErrorType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mErrorCorrectingTypeTable); Index++) {
    if (mErrorCorrectingTypeTable[Index].Value == ErrorType) {
      return mErrorCorrectingTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetSystemCacheTypeString (
  IN UINT8 CacheType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mSystemCacheTypeTable); Index++) {
    if (mSystemCacheTypeTable[Index].Value == CacheType) {
      return mSystemCacheTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetCacheAssociativityString (
  IN UINT8 Associativity
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mCacheAssociativityTable); Index++) {
    if (mCacheAssociativityTable[Index].Value == Associativity) {
      return mCacheAssociativityTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 8 (Port Connector Information) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE8_ITEM;

STATIC CONST TYPE8_ITEM mPortConnectorTypeTable[] = {
  { 0x00, "None" },
  { 0x01, "Centronics" },
  { 0x02, "Mini Centronics" },
  { 0x03, "Proprietary" },
  { 0x04, "DB-25 pin male" },
  { 0x05, "DB-25 pin female" },
  { 0x06, "DB-15 pin male" },
  { 0x07, "DB-15 pin female" },
  { 0x08, "DB-9 pin male" },
  { 0x09, "DB-9 pin female" },
  { 0x0A, "RJ-11" },
  { 0x0B, "RJ-45" },
  { 0x0C, "50-pin MiniSCSI" },
  { 0x0D, "Mini-DIN" },
  { 0x0E, "Micro-DIN" },
  { 0x0F, "PS/2" },
  { 0x10, "Infrared" },
  { 0x11, "HP-HIL" },
  { 0x12, "Access Bus (USB)" },
  { 0x16, "On Board IDE" },
  { 0x17, "On Board Floppy" },
  { 0x1F, "Mini-Jack (headphones)" },
  { 0x20, "BNC" },
  { 0x21, "1394" },
  { 0x22, "SAS/SATA Plug Receptacle" },
  { 0x23, "USB Type-C Receptacle" },
  { 0xFF, "Other" }
};

STATIC CONST TYPE8_ITEM mPortTypeTable[] = {
  { 0x00, "None" },
  { 0x01, "Parallel Port XT/AT Compatible" },
  { 0x02, "Parallel Port PS/2" },
  { 0x03, "Parallel Port ECP" },
  { 0x04, "Parallel Port EPP" },
  { 0x05, "Parallel Port ECP/EPP" },
  { 0x06, "Serial Port XT/AT Compatible" },
  { 0x07, "Serial Port 16450 Compatible" },
  { 0x08, "Serial Port 16550 Compatible" },
  { 0x09, "Serial Port 16550A Compatible" },
  { 0x0A, "SCSI Port" },
  { 0x0B, "MIDI Port" },
  { 0x0C, "Joy Stick Port" },
  { 0x0D, "Keyboard Port" },
  { 0x0E, "Mouse Port" },
  { 0x10, "USB" },
  { 0x11, "FireWire (IEEE P1394)" },
  { 0x12, "PCMCIA Type I2" },
  { 0x13, "PCMCIA Type II" },
  { 0x14, "PCMCIA Type III" },
  { 0x15, "Cardbus" },
  { 0x1C, "Video Port" },
  { 0x1D, "Audio Port" },
  { 0x1E, "Modem Port" },
  { 0x1F, "Network Port" },
  { 0x20, "SATA" },
  { 0x21, "SAS" },
  { 0x22, "MFDP (Multi-Function Display Port)" },
  { 0x23, "Thunderbolt" },
  { 0xFF, "Other" }
};

CONST CHAR8 *
GetPortConnectorTypeString (
  IN UINT8 ConnectorType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mPortConnectorTypeTable); Index++) {
    if (mPortConnectorTypeTable[Index].Value == ConnectorType) {
      return mPortConnectorTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetPortTypeString (
  IN UINT8 PortType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mPortTypeTable); Index++) {
    if (mPortTypeTable[Index].Value == PortType) {
      return mPortTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 9 (System Slots) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE9_ITEM;

// =====================================================================
// SMBIOS Type 9 (System Slots) - Slot Type Lookup Table
// =====================================================================

STATIC CONST TYPE9_ITEM mSlotTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "ISA" },
  { 0x04, "MCA" },
  { 0x05, "EISA" },
  { 0x06, "PCI" },
  { 0x07, "PC Card (PCMCIA)" },
  { 0x08, "VL-VESA" },
  { 0x09, "Proprietary" },
  { 0x0A, "Processor Card Slot" },
  { 0x0B, "Proprietary Memory Card Slot" },
  { 0x0C, "I/O Riser Card Slot" },
  { 0x0D, "NuBus" },
  { 0x0E, "PCI - 66MHz Capable" },
  { 0x0F, "AGP" },
  { 0x10, "AGP 2X" },
  { 0x11, "AGP 4X" },
  { 0x12, "PCI-X" },
  { 0x13, "AGP 8X" },
  { 0x14, "M.2 Socket 1-DP (Mechanical Key A)" },
  { 0x15, "M.2 Socket 1-SD (Mechanical Key E)" },
  { 0x16, "M.2 Socket 2 (Mechanical Key B)" },
  { 0x17, "M.2 Socket 3 (Mechanical Key M)" },
  { 0x18, "MXM Type I" },
  { 0x19, "MXM Type II" },
  { 0x1A, "MXM Type III (standard connector)" },
  { 0x1B, "MXM Type III (HE connector)" },
  { 0x1C, "MXM Type IV" },
  { 0x1D, "MXM 3.0 Type A" },
  { 0x1E, "MXM 3.0 Type B" },
  { 0x1F, "PCI Express Gen 2 SFF-8639" },
  { 0x20, "PCI Express Gen 3 SFF-8639" },
  { 0x21, "PCI Express Mini 52-pin (CEM spec. 2.0) with bottom-side keep-outs" },
  { 0x22, "PCI Express Mini 52-pin (CEM spec. 2.0) without bottom-side keep-outs" },
  { 0x23, "PCI Express Mini 76-pin (CEM spec. 2.0) Corresponds to Display-Mini card" },
  { 0x24, "PCI Express Gen 4 SFF-8639 (U.2)" },
  { 0x25, "PCI Express Gen 5 SFF-8639 (U.2)" },
  { 0x26, "OCP NIC 3.0 Small Form Factor (SFF)" },
  { 0x27, "OCP NIC 3.0 Large Form Factor (LFF)" },
  { 0x28, "OCP NIC Prior to 3.0" },
  { SlotTypeCXLFlexbus10, "CXL Flexbus 1.0" },
  { 0xA0, "PC-98/C20 " },
  { 0xA1, "PC-98/C24 " },
  { 0xA2, "PC-98/E " },
  { 0xA3, "PC-98/Local Bus " },
  { 0xA4, "PC-98/Card " },
  { 0xA5, "PCI Express " },
  { 0xA6, "PCI Express X1" },
  { 0xA7, "PCI Express X2" },
  { 0xA8, "PCI Express X4" },
  { 0xA9, "PCI Express X8" },
  { 0xAA, "PCI Express X16" },
  { 0xAB, "PCI Express Gen 2" },
  { 0xAC, "PCI Express Gen 2 X1" },
  { 0xAD, "PCI Express Gen 2 X2" },
  { 0xAE, "PCI Express Gen 2 X4" },
  { 0xAF, "PCI Express Gen 2 X8" },
  { 0xB0, "PCI Express Gen 2 X16" },
  { 0xB1, "PCI Express Gen 3" },
  { 0xB2, "PCI Express Gen 3 X1" },
  { 0xB3, "PCI Express Gen 3 X2" },
  { 0xB4, "PCI Express Gen 3 X4" },
  { 0xB5, "PCI Express Gen 3 X8" },
  { 0xB6, "PCI Express Gen 3 X16" },
  { SlotTypePciExpressGen4, "PCI Express Gen 4" },
  { SlotTypePciExpressGen4X1, "PCI Express Gen 4 X1" },
  { SlotTypePciExpressGen4X2, "PCI Express Gen 4 X2" },
  { SlotTypePciExpressGen4X4, "PCI Express Gen 4 X4" },
  { SlotTypePciExpressGen4X8, "PCI Express Gen 4 X8" },
  { SlotTypePciExpressGen4X16, "PCI Express Gen 4 X16" },
  { SlotTypePCIExpressGen5, "PCI Express Gen 5" },
  { SlotTypePCIExpressGen5X1, "PCI Express Gen 5 x1" },
  { SlotTypePCIExpressGen5X2, "PCI Express Gen 5 x2" },
  { SlotTypePCIExpressGen5X4, "PCI Express Gen 5 x4" },
  { SlotTypePCIExpressGen5X8, "PCI Express Gen 5 x8" },
  { SlotTypePCIExpressGen5X16, "PCI Express Gen 5 x16" },
  { SlotTypePCIExpressGen6andBeyond, "PCI Express Gen 6 and Beyond" },
  { SlotTypeEnterpriseandDatacenter1UE1FormFactorSlot, "Enterprise and Datacenter 1U E1 Form Factor Slot" },
  { SlotTypeEnterpriseandDatacenter3E3FormFactorSlot, "Enterprise and Datacenter 3'E3 Form Factor Slot" }
};

STATIC CONST TYPE9_ITEM mSlotDataBusWidthTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "8 bit" },
  { 0x04, "16 bit" },
  { 0x05, "32 bit" },
  { 0x06, "64 bit" },
  { 0x07, "128 bit" },
  { 0x08, "1x or x1" },
  { 0x09, "2x or x2" },
  { 0x0A, "4x or x4" },
  { 0x0B, "8x or x8" },
  { 0x0C, "12x or x12" },
  { 0x0D, "16x or x16" },
  { 0x0E, "32x or x32" }
};

STATIC CONST TYPE9_ITEM mCurrentUsageTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Available" },
  { 0x04, "In use" },
  { 0x05, "Unavailable" } // E.g., disabled via bifurcation or BIOS setup
};

STATIC CONST TYPE9_ITEM mSlotLengthTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Short Length" },
  { 0x04, "Long Length" },
  { 0x05, "2.5\" drive form factor" },
  { 0x06, "3.5\" drive form factor" }
};

CONST CHAR8 *
GetSlotTypeString (
  IN UINT8 SlotType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mSlotTypeTable); Index++) {
    if (mSlotTypeTable[Index].Value == SlotType) {
      return mSlotTypeTable[Index].Description;
    }
  }
  return "OEM Specific/Other PCIe";
}

CONST CHAR8 *
GetSlotDataBusWidthString (
  IN UINT8 SlotDataBusWidth
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mSlotDataBusWidthTable); Index++) {
    if (mSlotDataBusWidthTable[Index].Value == SlotDataBusWidth) {
      return mSlotDataBusWidthTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetCurrentUsageString (
  IN UINT8 CurrentUsage
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mCurrentUsageTable); Index++) {
    if (mCurrentUsageTable[Index].Value == CurrentUsage) {
      return mCurrentUsageTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetSlotLengthString (
  IN UINT8 SlotLength
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mSlotLengthTable); Index++) {
    if (mSlotLengthTable[Index].Value == SlotLength) {
      return mSlotLengthTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetSystemSlotInfoString (
  IN UINT8 SlotInfo
  )
{
  switch (SlotInfo) {
    case 0x00: return "Others";
    case 0x01: return "Gen 1";
    case 0x02: return "Gen 2";
    case 0x03: return "Gen 3";
    case 0x04: return "Gen 4";
    case 0x05: return "Gen 5";
    case 0x06: return "Gen 6";
    default:   return "OEM Specific";
  }
}

CONST CHAR8 *
GetSystemSlotPhysicalWidthString (
  IN UINT8 PhysicalWidth
  )
{
  // Re-use the existing data bus width table, but handle specific edge cases
  if (PhysicalWidth == 0 || PhysicalWidth > 0x0E) {
    return "Undefined Value";
  }
  return GetSlotDataBusWidthString (PhysicalWidth);
}

CONST CHAR8 *
GetSystemSlotHeightString (
  IN UINT8 SlotHeight
  )
{
  switch (SlotHeight) {
    case 0x00: return "Not applicable";
    case 0x01: return "Other";
    case 0x02: return "Unknown";
    case 0x03: return "Full height";
    case 0x04: return "Low-Profile";
    default:   return "OEM Specific";
  }
}

// =====================================================================
// SMBIOS Type 16 (Physical Memory Array) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE16_ITEM;

STATIC CONST TYPE16_ITEM mMemoryArrayLocationTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "System board or motherboard" },
  { 0x04, "ISA add-on card" },
  { 0x05, "EISA add-on card" },
  { 0x06, "PCI add-on card" },
  { 0x07, "MCA add-on card" },
  { 0x08, "PCMCIA add-on card" },
  { 0x09, "Proprietary add-on card" },
  { 0x0A, "NuBus" },
  { 0xA0, "PC-98/C20 add-on card" },
  { 0xA1, "PC-98/C24 add-on card" },
  { 0xA2, "PC-98/E add-on card" },
  { 0xA3, "PC-98/Local bus add-on card" },
  { 0xA4, "CXL add-on card" } // SMBIOS 3.5.0+ extension
};

STATIC CONST TYPE16_ITEM mMemoryArrayUseTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "System memory" },
  { 0x04, "Video memory" },
  { 0x05, "Flash memory" },
  { 0x06, "Non-volatile RAM" },
  { 0x07, "Cache memory" }
};

STATIC CONST TYPE16_ITEM mMemoryErrorCorrectionTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "None" },
  { 0x04, "Parity" },
  { 0x05, "Single-bit ECC" },
  { 0x06, "Multi-bit ECC" },
  { 0x07, "CRC" }
};

CONST CHAR8 *
GetMemoryArrayLocationString (
  IN UINT8 Location
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMemoryArrayLocationTable); Index++) {
    if (mMemoryArrayLocationTable[Index].Value == Location) {
      return mMemoryArrayLocationTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetMemoryArrayUseString (
  IN UINT8 Use
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMemoryArrayUseTable); Index++) {
    if (mMemoryArrayUseTable[Index].Value == Use) {
      return mMemoryArrayUseTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetMemoryErrorCorrectionString (
  IN UINT8 ErrorCorrection
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMemoryErrorCorrectionTable); Index++) {
    if (mMemoryErrorCorrectionTable[Index].Value == ErrorCorrection) {
      return mMemoryErrorCorrectionTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 17 (Memory Device) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE17_ITEM;

STATIC CONST TYPE17_ITEM mMemoryFormFactorTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "SIMM" },
  { 0x04, "SIP" },
  { 0x05, "Chip" },
  { 0x06, "DIP" },
  { 0x07, "ZIP" },
  { 0x08, "Proprietary Card" },
  { 0x09, "DIMM" },
  { 0x0A, "TSOP" },
  { 0x0B, "Row of chips" },
  { 0x0C, "RIMM" },
  { 0x0D, "SODIMM" },
  { 0x0E, "SRIMM" },
  { 0x0F, "FB-DIMM" },
  { MemoryFormFactorDie, "Die" }
};

STATIC CONST TYPE17_ITEM mMemoryTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "DRAM" },
  { 0x04, "EDRAM" },
  { 0x05, "VRAM" },
  { 0x06, "SRAM" },
  { 0x07, "RAM" },
  { 0x08, "ROM" },
  { 0x09, "FLASH" },
  { 0x0A, "EEPROM" },
  { 0x0B, "FEPROM" },
  { 0x0C, "EPROM" },
  { 0x0D, "CDRAM" },
  { 0x0E, "3DRAM" },
  { 0x0F, "SDRAM" },
  { 0x10, "SGRAM" },
  { 0x11, "RDRAM" },
  { 0x12, "DDR" },
  { 0x13, "DDR2" },
  { 0x14, "DDR2 FB-DIMM" },
  { 0x18, "DDR3" },
  { 0x19, "FBD2" },
  { 0x1A, "DDR4" },
  { 0x1B, "LPDDR" },
  { 0x1C, "LPDDR2" },
  { 0x1D, "LPDDR3" },
  { 0x1E, "LPDDR4" },
  { 0x1F, "Logical non-volatile device" },
  { MemoryTypeHBM, "HBM (High Bandwidth Memory)" },
  { MemoryTypeHBM2, "HBM2 (High Bandwidth Memory Generation 2)" },
  { MemoryTypeDdr5, "DDR5" },
  { MemoryTypeLpddr5, "LPDDR5" }
};

CONST CHAR8 *
GetMemoryFormFactorString (IN UINT8 FormFactor) {
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMemoryFormFactorTable); Index++) {
    if (mMemoryFormFactorTable[Index].Value == FormFactor) return mMemoryFormFactorTable[Index].Description;
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetMemoryTypeString (IN UINT8 MemoryType) {
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMemoryTypeTable); Index++) {
    if (mMemoryTypeTable[Index].Value == MemoryType) return mMemoryTypeTable[Index].Description;
  }
  return "OEM Specific";
}

/**
  String table for Memory Type Detail (Offset 0x13, 16-bit field).
  Index maps directly to the bit position (0 to 15).
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR16 *mMemoryTypeDetailStrings[16] = {
  L"Reserved",                     // Bit 0
  L"Other",                        // Bit 1
  L"Unknown",                      // Bit 2
  L"Fast-paged",                   // Bit 3
  L"Static column",                // Bit 4
  L"Pseudo-static",                // Bit 5
  L"RAM",                          // Bit 6
  L"Synchronous",                  // Bit 7
  L"CMOS",                         // Bit 8
  L"EDO",                          // Bit 9
  L"Window DRAM",                  // Bit 10
  L"Cache DRAM",                   // Bit 11
  L"Non-volatile",                 // Bit 12
  L"Registered (Buffered)",        // Bit 13
  L"Unbuffered (Unregistered)",    // Bit 14
  L"LRDIMM"                        // Bit 15
};

/**
  @brief Retrieves the description for a specific bit in Memory Type Detail.
  @param[in] BitIndex  The bit position to query (0-15).
**/
CONST CHAR16 *
GetMemoryTypeDetailString (
  IN UINT8 BitIndex
  )
{
  if (BitIndex < 16) {
    return mMemoryTypeDetailStrings[BitIndex];
  }
  return NULL;
}

STATIC CONST TYPE17_ITEM mMemoryTechnologyTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "DRAM" },
  { 0x04, "NVDIMM-N" },
  { 0x05, "NVDIMM-F" },
  { 0x06, "NVDIMM-P" },
  { MemoryTechnologyIntelOptanePersistentMemory, "Intel Optane DC persistent memory" }
};

CONST CHAR8 *
GetMemoryTechnologyString (
  IN UINT8 Technology
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMemoryTechnologyTable); Index++) {
    if (mMemoryTechnologyTable[Index].Value == Technology) {
      return mMemoryTechnologyTable[Index].Description;
    }
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetMemoryOperatingModeString (
  IN UINT16 Mode
  )
{
  // Operating Mode is a bit-field defined in SMBIOS 3.2
  if ((Mode & BIT3) != 0) return "Volatile memory";
  if ((Mode & BIT4) != 0) return "Byte-accessible persistent memory";
  if ((Mode & BIT5) != 0) return "Block-accessible persistent memory";
  if ((Mode & BIT1) != 0) return "Other";
  if ((Mode & BIT2) != 0) return "Unknown";
  return "None";
}

// =====================================================================
// SMBIOS Type 38 (IPMI Device Information) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE38_ITEM;

STATIC CONST TYPE38_ITEM mIpmiInterfaceTypeTable[] = {
  { 0x00, "Unknown" },
  { 0x01, "KCS (Keyboard Controller Style)" },
  { 0x02, "SMIC (Server Management Interface Chip)" },
  { 0x03, "BT (Block Transfer)" },
  { 0x04, "SSIF (SMBus System Interface)" }
};

CONST CHAR8 *
GetIpmiInterfaceTypeString (
  IN UINT8 InterfaceType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mIpmiInterfaceTypeTable); Index++) {
    if (mIpmiInterfaceTypeTable[Index].Value == InterfaceType) {
      return mIpmiInterfaceTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 39 (System Power Supply) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE39_ITEM;

STATIC CONST TYPE39_ITEM mPowerSupplyTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Linear" },
  { 0x04, "Switching" },
  { 0x05, "Battery" },
  { 0x06, "UPS" },
  { 0x07, "Converter" },
  { 0x08, "Regulator" }
};

STATIC CONST TYPE39_ITEM mPowerSupplyStatusTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "OK" },
  { 0x04, "Non-critical" },
  { 0x05, "Critical, power supply failed" }
};

STATIC CONST TYPE39_ITEM mPowerSupplyInputVoltageRangeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Manual" },
  { 0x04, "Auto-switch" },
  { 0x05, "Wide range" },
  { 0x06, "Not applicable" }
};

STATIC CONST TYPE39_ITEM mPowerSupplyPluggableTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Pluggable" },
  { 0x04, "Not pluggable" }
};

CONST CHAR8 *
GetPowerSupplyTypeString (
  IN UINT8 Type
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mPowerSupplyTypeTable); Index++) {
    if (mPowerSupplyTypeTable[Index].Value == Type) return mPowerSupplyTypeTable[Index].Description;
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetPowerSupplyStatusString (
  IN UINT8 Status
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mPowerSupplyStatusTable); Index++) {
    if (mPowerSupplyStatusTable[Index].Value == Status) return mPowerSupplyStatusTable[Index].Description;
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetPowerSupplyInputVoltageRangeString (
  IN UINT8 InputVoltage
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mPowerSupplyInputVoltageRangeTable); Index++) {
    if (mPowerSupplyInputVoltageRangeTable[Index].Value == InputVoltage) return mPowerSupplyInputVoltageRangeTable[Index].Description;
  }
  return "OEM Specific";
}

CONST CHAR8 *
GetPowerSupplyPluggableString (
  IN UINT8 Pluggable
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mPowerSupplyPluggableTable); Index++) {
    if (mPowerSupplyPluggableTable[Index].Value == Pluggable) return mPowerSupplyPluggableTable[Index].Description;
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 41 (Onboard Devices Extended Information) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE41_ITEM;

STATIC CONST TYPE41_ITEM mOnboardDeviceTypeTable[] = {
  { 0x01, "Other" },
  { 0x02, "Unknown" },
  { 0x03, "Video" },
  { 0x04, "SCSI Controller" },
  { 0x05, "Ethernet" },
  { 0x06, "Token Ring" },
  { 0x07, "Sound" },
  { 0x08, "PATA Controller" },
  { 0x09, "SATA Controller" },
  { 0x0A, "SAS Controller" },
  { 0x0B, "Wireless LAN" },
  { 0x0C, "Bluetooth" },
  { 0x0D, "WWAN" },
  { 0x0E, "eMMC" },
  { 0x0F, "NVMe Controler" },
  { 0x10, "UFS Controler" }
};

CONST CHAR8 *
GetOnboardDeviceTypeString (
  IN UINT8 DeviceType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mOnboardDeviceTypeTable); Index++) {
    if (mOnboardDeviceTypeTable[Index].Value == DeviceType) {
      return mOnboardDeviceTypeTable[Index].Description;
    }
  }
  return "OEM Specific";
}

// =====================================================================
// SMBIOS Type 42 (Management Controller Host Interface) Lookup Tables
// =====================================================================

typedef struct {
  UINT8       Value;
  CONST CHAR8 *Description;
} TYPE42_ITEM;

STATIC CONST TYPE42_ITEM mMcHostInterfaceTypeTable[] = {
  { 0x00, "Unknown" },
  { 0xF0, "OEM Specific" },
  { 0x40, "Network Host Interface" } 
};

CONST CHAR8 *
GetMcHostInterfaceTypeString (
  IN UINT8 InterfaceType
  )
{
  UINTN Index;
  for (Index = 0; Index < ARRAY_SIZE (mMcHostInterfaceTypeTable); Index++) {
    if (mMcHostInterfaceTypeTable[Index].Value == InterfaceType) {
      return mMcHostInterfaceTypeTable[Index].Description;
    }
  }
  return "Reserved / Other";
}