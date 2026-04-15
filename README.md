# Dump SMBIOS (UEFI Shell Application)

![UEFI](https://img.shields.io/badge/UEFI-Spec_2.9+-blue.svg)
![EDK II](https://img.shields.io/badge/EDK_II-Tianocore-green.svg)
![Architecture](https://img.shields.io/badge/Architecture-X64-lightgrey.svg)
![License](https://img.shields.io/badge/License-BSD_2--Clause-orange.svg)

## Overview

`Dump_SMBios` is a modularized UEFI Shell application engineered to dump comprehensive SMBIOS table information directly from the system firmware. Unlike standard utilities that rely on the `EFI_SMBIOS_PROTOCOL`, this application directly locates the SMBIOS Entry Point (both 32-bit `_SM_` and 64-bit `_SM3_`) via the UEFI System Configuration Table. 

This tool is highly beneficial for firmware debugging, ACPI/SMBIOS validation, and inspecting low-level hardware topologies. Special focus has been integrated for modern architectures, including extensive decoding for PCIe 4.0 and PCIe 5.0 capabilities within Type 9 (System Slots) records.

> **Note:** This is a practice project intended for educational purposes and firmware development exercises. 

## Features

* **Direct Memory Parsing:** Bypasses UEFI protocols to directly parse raw physical memory structures.
* **Dual Architecture Support:** Automatically detects and parses both SMBIOS 2.x (32-bit) and SMBIOS 3.x (64-bit) entry point structures.
* **Modern Hardware Topologies:** Comprehensive string mapping for cutting-edge hardware, including CXL interfaces, DDR5, and PCIe Gen 4/Gen 5 enumerations.
* **Granular Filtering:** Supports command-line parameters to isolate and dump specific SMBIOS types.
* **Raw Hex Dumping:** Built-in `MemoryDumpUtil` for 16-byte aligned hexadecimal and ASCII preview dumps of unparsed or OEM-specific data regions.

## Execution Environment

* **Target Platform:** UEFI Shell
* **Architecture:** X64 (Recommended)
* **Build Environment:** EDK II (Tianocore) 

## Build Instructions

1. Clone or copy the source code into your EDK II workspace (e.g., `edk2/ShellPkg/Application/Dump_SMBios/`).
2. Add the `.inf` file path to the `[Components]` section of your target `.dsc` file (e.g., `ShellPkg.dsc` or `MdeModulePkg.dsc`):
   ```ini
   ShellPkg/Application/Dump_SMBios/Dump_SMBios.inf
   ```
3. Compile the application using the `build` command:
   ```bash
   build -a X64 -t <Toolchain_Tag> -p ShellPkg/ShellPkg.dsc
   ```

## Usage

Execute the compiled `.efi` binary within the UEFI Shell. 

| Parameter / Argument | Description |
| :--- | :--- |
| `[None]` | Parses and dumps the entire SMBIOS table hierarchy (Types 0 through 127). |
| `<Type Number>` | Filters the output to exclusively display the specified SMBIOS Type (e.g., `9` for System Slots, `17` for Memory Devices). |

**Examples:**

Dump all SMBIOS records:
```bash
FS0:\> Dump_SMBios.efi
```

Dump only Type 4 (Processor Information):
```bash
FS0:\> Dump_SMBios.efi 4
```

Dump only Type 9 (System Slots) to inspect PCIe 4.0/5.0 configurations:
```bash
FS0:\> Dump_SMBios.efi 9
```

## Project Structure

* `Dump_SMBios.c`: Main application logic, entry point location, and structure traversal.
* [cite_start]`Dump_SMBios.inf`: EDK II build metadata[cite: 1, 2, 3].
* `SmbiosStringTable.c / .h`: Static lookup tables (LUT) translating DMTF SMBIOS integer definitions into human-readable ASCII/Unicode strings.
* `MemoryDumpUtil.c / .h`: Low-level hex dump utilities for raw memory inspection.

## License

Copyright (c) 2026, OEM Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent
