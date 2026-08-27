<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/DAWN_CONTEXT.md">🇷🇺 РУССКИЙ</a>
</p>

# Dawn Context v1

**Status:** Implemented in Prelude and validated by the first x86_64 Pulse entry.

`Dawn Context` is the compact, versioned boot contract through which Prelude transfers platform state to Pulse. Version 1 contains only the UEFI memory map. It deliberately does not expose live UEFI Boot Services to Pulse.

UEFI Boot Services remain available only before a successful `ExitBootServices()` call; after that transition, the OS loader has assumed platform control.[1] Prelude obtains the memory map through `GetMemoryMap()`, which supplies the memory-resource description that an OS loader must convey onward.[2]

## Binary layout

| Field | Type | Meaning |
|---|---|---|
| `magic` | `UINT64` | `DAWN_CONTEXT_MAGIC`, identifying the binary contract. |
| `version` | `UINT32` | Contract version, initially `1`. |
| `size` | `UINT32` | Full size of the producer's structure. |
| `memory_map_physical_address` | `uint64_t` | Physical address of UEFI memory descriptors retained for Pulse. |
| `memory_map_size` | `uint64_t` | Used byte count in the retained map. |
| `memory_map_key` | `UINTN` | Key that successfully sealed Boot Services. |
| `memory_descriptor_size` | `UINTN` | Stride between descriptors. |
| `memory_descriptor_version` | `UINT32` | UEFI descriptor version. |
| `reserved` | `UINT32` | Zero in v1. |
| `kernel_stack_top` | `uint64_t` | Initial stack-top physical address for Pulse. |
| `kernel_stack_size` | `uint64_t` | Initial stack allocation size in bytes. |

## Producer rules

Prelude first queries the map size, allocates a `LoaderData` buffer with extra descriptor capacity, retrieves a current map and key, then calls `ExitBootServices()`. If firmware rejects the key because the map changed, Prelude captures a fresh map and retries up to three times. After success, it invokes no Boot Services function.

The structure is **append-only**. A future Pulse build must reject an unknown `magic`, a newer unsupported `version`, or a `size` too small for the fields it reads. A future Prelude may append fields without invalidating an older, size-aware Pulse.

## Current handoff boundary

This milestone seals the context, loads the first native Pulse image at physical address `0x00200000`, and proves the direct transition in QEMU. Prelude allocates a separate 128 KiB stack for Pulse. Pulse validates the contract, selects an EFI conventional-memory region and allocates two 4 KiB bootstrap frames; paging, interrupts, and task scheduling remain later Pulse milestones.

## References

[1] [UEFI Specification: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)

[2] [ACPI Specification: UEFI GetMemoryMap() Boot Services Function](https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/uefi-getmemorymap-boot-services-function.html)
