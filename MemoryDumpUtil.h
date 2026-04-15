/** @file
  Header file for the Memory Dump Utility module.

  This utility provides raw memory debugging functions, allowing engineers
  to inspect physical memory byte-order directly. It is highly useful for
  debugging SMBIOS tables, ACPI tables, and PCIe 4.0/5.0 Configuration Spaces.

  Copyright (c) 2026, OEM Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef MEMORY_DUMP_UTIL_H_
#define MEMORY_DUMP_UTIL_H_

#include <Uefi.h>

/**
  Dumps a memory region in standard hexadecimal and ASCII format.
  This function bypasses high-level abstraction (e.g., standard GUID formatting)
  to display the raw physical byte layout.

  @param[in] Data  A pointer to the starting address of the memory buffer.
  @param[in] Size  The size of the buffer to dump, in bytes.
**/
VOID
DumpRawMemoryHex (
  IN UINT8  *Data,
  IN UINTN  Size
  );

#endif // MEMORY_DUMP_UTIL_H_