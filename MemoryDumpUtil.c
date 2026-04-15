/** @file
  Implementation file for the Memory Dump Utility module.

  Copyright (c) 2026, OEM Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "MemoryDumpUtil.h"
#include <Library/UefiLib.h>

VOID
DumpRawMemoryHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN Index;

  Print (L" |--> size=%d:\n", Size);
  Print (L" |--> 00000000: ");

  //
  // Step 1: Iterate through the buffer and print hexadecimal values.
  // A hyphen is inserted between the 8th and 9th bytes for standard 
  // hex dump readability.
  //
  for (Index = 0; Index < Size; Index++) {
    if (Index == 7) {
      Print (L"%02X-", Data[Index]);
    } else {
      Print (L"%02X ", Data[Index]);
    }
  }

  Print (L" *");

  //
  // Step 2: Print the printable ASCII representation.
  // Non-printable characters are replaced with a dot ('.').
  //
  for (Index = 0; Index < Size; Index++) {
    // Printable ASCII range is 0x20 (Space) to 0x7E (Tilde)
    if (Data[Index] >= 0x20 && Data[Index] <= 0x7E) {
      Print (L"%c", (CHAR16)Data[Index]); 
    } else {
      Print (L".");
    }
  }

  Print (L"*\n");
}