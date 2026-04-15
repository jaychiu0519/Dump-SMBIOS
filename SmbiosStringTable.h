/** @file
  Header file for SMBIOS Type to String mapping module.

  Copyright (c) 2026, OEM Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef SMBIOS_STRING_TABLE_H_
#define SMBIOS_STRING_TABLE_H_

#include <Uefi.h>

/**
  @brief Retrieves the description for a specific bit in BIOS Characteristics.
**/
CONST CHAR16 *
GetBiosCharacteristicsString (
  IN UINT8 BitIndex
  );

/**
  @brief Retrieves the description for a specific bit in BIOS Extension Byte 1.
**/
CONST CHAR16 *
GetBiosExtByte1String (
  IN UINT8 BitIndex
  );

/**
  @brief Retrieves the description for a specific bit in BIOS Extension Byte 2.
**/
CONST CHAR16 *
GetBiosExtByte2String (
  IN UINT8 BitIndex
  );

/**
  Translates the SMBIOS Type integer to a human-readable ASCII string.

  @param[in] Type  The 8-bit SMBIOS structure type defined by DMTF specification.

  @return A constant pointer to the descriptive ASCII string. Returns 
          "Unknown/OEM Specific Type" if the type is not found.
**/
CONST CHAR8 *
GetSmbiosTypeName (
  IN UINT8 Type
  );

/**
  Converts the SMBIOS Type 1 Wake-up Type enumeration to a descriptive ASCII string.
  Based on DMTF SMBIOS Specification Section 7.2.2.

  @param[in] WakeUpType  The 8-bit wake-up type value from the SMBIOS Type 1 structure.

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetWakeUpTypeString (
  IN UINT8 WakeUpType
  );

/**
  Converts the SMBIOS Type 2 Board Type enumeration to a descriptive ASCII string.
  Based on DMTF SMBIOS Specification Section 7.3.1.

  @param[in] BoardType  The 8-bit board type value from the SMBIOS Type 2 structure.

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetBoardTypeString (
  IN UINT8 BoardType
  );


/**
  Retrieves the descriptive string for a specific Baseboard Feature Flag bit.

  @param[in] BitIndex  The bit index (0-7) to query.

  @return A constant pointer to the Unicode description string. 
          Returns NULL if the bit is undefined or reserved in the DMTF specification.
**/
CONST CHAR16 *
GetBaseboardFeatureString (
  IN UINT8 BitIndex
  );

/**
  Converts the SMBIOS Type 3 Chassis Type enumeration to an ASCII string.
  Bit 7 (Chassis lock present) must be masked out before passing to this function.

  @param[in] ChassisType  The 7-bit chassis type value (Bits 0-6).

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetChassisTypeString (
  IN UINT8 ChassisType
  );

/**
  Converts the SMBIOS Type 3 Chassis State enumeration to an ASCII string.
  Applies to Boot-up State, Power Supply State, and Thermal State.

  @param[in] State  The 8-bit state value.

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetChassisStateString (
  IN UINT8 State
  );

/**
  Converts the SMBIOS Type 3 Security Status enumeration to an ASCII string.

  @param[in] SecurityStatus  The 8-bit security status value.

  @return A constant pointer to the descriptive ASCII string.
**/
CONST CHAR8 *
GetChassisSecurityStatusString (
  IN UINT8 SecurityStatus
  );

CONST CHAR8 *
GetProcessorTypeString (
  IN UINT8 ProcessorType
  );

/**
  Converts the SMBIOS Type 4 Processor Upgrade enumeration to a Unicode string.

  @param[in] ProcessorUpgrade  The 8-bit processor upgrade value.

  @return A constant pointer to the descriptive Unicode string.
**/
CONST CHAR8 *
GetProcessorUpgradeString (
  IN UINT8 ProcessorUpgrade
  );

//==========Type 7=============
CONST CHAR8 *
GetCacheSramTypeString (
  IN UINT16 SramType
  );

CONST CHAR8 *
GetErrorCorrectingTypeString (
  IN UINT8 ErrorType
  );

CONST CHAR8 *
GetSystemCacheTypeString (
  IN UINT8 CacheType
  );

CONST CHAR8 *
GetCacheAssociativityString (
  IN UINT8 Associativity
  );

/**
  @brief Retrieves the description for Processor Status (Bits 2:0).
**/
CONST CHAR16 *
GetProcessorStatusString (
  IN UINT8 StatusValue
  );

/**
  @brief Retrieves the description for a specific bit in Processor Characteristics.
**/
CONST CHAR16 *
GetProcessorCharacteristicsString (
  IN UINT8 BitIndex
  );

/**
  SMBIOS Type 8 (Port Connector Information) Lookup APIs
**/

CONST CHAR8 *
GetPortConnectorTypeString (
  IN UINT8 ConnectorType
  );

CONST CHAR8 *
GetPortTypeString (
  IN UINT8 PortType
  );

/**
  SMBIOS Type 9 (System Slots) Lookup APIs
**/

CONST CHAR8 *
GetSlotTypeString (
  IN UINT8 SlotType
  );

CONST CHAR8 *
GetSlotDataBusWidthString (
  IN UINT8 SlotDataBusWidth
  );

CONST CHAR8 *
GetCurrentUsageString (
  IN UINT8 CurrentUsage
  );

CONST CHAR8 *
GetSlotLengthString (
  IN UINT8 SlotLength
  );

CONST CHAR8 *
GetSystemSlotInfoString (
  IN UINT8 SlotInfo
  );

CONST CHAR8 *
GetSystemSlotPhysicalWidthString (
  IN UINT8 PhysicalWidth
  );

CONST CHAR8 *
GetSystemSlotHeightString (
  IN UINT8 SlotHeight
  );

/**
  SMBIOS Type 16 (Physical Memory Array) Lookup APIs
**/

CONST CHAR8 *
GetMemoryArrayLocationString (
  IN UINT8 Location
  );

CONST CHAR8 *
GetMemoryArrayUseString (
  IN UINT8 Use
  );

CONST CHAR8 *
GetMemoryErrorCorrectionString (
  IN UINT8 ErrorCorrection
  );

/**
  SMBIOS Type 17 (Memory Device) Lookup APIs
**/

CONST CHAR8 *
GetMemoryFormFactorString (
  IN UINT8 FormFactor
  );

CONST CHAR8 *
GetMemoryTypeString (
  IN UINT8 MemoryType
  );

/**
  @brief Retrieves the description for a specific bit in Memory Type Detail.
**/
CONST CHAR16 *
GetMemoryTypeDetailString (
  IN UINT8 BitIndex
  );

CONST CHAR8 *
GetMemoryTechnologyString (
  IN UINT8 Technology
  );

CONST CHAR8 *
GetMemoryOperatingModeString (
  IN UINT16 Mode
  );

/**
  SMBIOS Type 38 (IPMI Device Information) Lookup APIs
**/

CONST CHAR8 *
GetIpmiInterfaceTypeString (
  IN UINT8 InterfaceType
  );

/**
  SMBIOS Type 39 (System Power Supply) Lookup APIs
**/

CONST CHAR8 *
GetPowerSupplyTypeString (
  IN UINT8 Type
  );

CONST CHAR8 *
GetPowerSupplyStatusString (
  IN UINT8 Status
  );

CONST CHAR8 *
GetPowerSupplyInputVoltageRangeString (
  IN UINT8 InputVoltage
  );

CONST CHAR8 *
GetPowerSupplyPluggableString (
  IN UINT8 Pluggable
  );

/**
  SMBIOS Type 41 (Onboard Devices Extended Information) Lookup APIs
**/

CONST CHAR8 *
GetOnboardDeviceTypeString (
  IN UINT8 DeviceType
  );

/**
  SMBIOS Type 42 (Management Controller Host Interface) Lookup APIs
**/

CONST CHAR8 *
GetMcHostInterfaceTypeString (
  IN UINT8 InterfaceType
  );

#endif // SMBIOS_STRING_TABLE_H_