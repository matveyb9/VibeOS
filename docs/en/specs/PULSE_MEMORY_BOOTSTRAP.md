<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_MEMORY_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse early memory bootstrap

**Status:** Implemented for the first x86_64 QEMU UEFI profile.

The first Pulse memory layer is intentionally a bootstrap, not a finished general allocator. It accepts the UEFI memory map handed over in Dawn Context, records up to 32 usable `EfiConventionalMemory` descriptors, and allocates 4 KiB physical frames in ascending descriptor order. The first boot validates the handoff by allocating two adjacent frames.

The UEFI memory map describes installed RAM and firmware-reserved physical ranges; descriptor types determine how an operating system must treat each range.[1] Pulse therefore allocates only from descriptor type `7`, `EfiConventionalMemory`, and treats all other ranges as unavailable at this stage.

## State

| Field | Meaning |
|---|---|
| `selected_region_base` | Start of the current conventional-memory region. |
| `selected_region_limit` | Exclusive end of the current region. |
| `next_free_frame` | Next frame that the bootstrap allocator will hand out. |
| `usable_page_count` | Total tracked 4 KiB frames across all retained regions. |
| `region_count` | Number of retained conventional-memory regions, up to 32. |

## Explicit limits

The bootstrap does not coalesce adjacent regions, reserve pages already consumed by later Pulse subsystems, support freeing, or provide an ownership bitmap. It is a narrow prerequisite for the x86_64 page-table and interrupt stages. Future Pulse memory management will replace it with an ownership-aware physical allocator while preserving the Dawn Context ABI boundary.

The repository's `make test` target runs host-side unit checks for descriptor validation, page alignment, adjacency, and end-of-region handling before the QEMU integration probe.

## Reference

[1] [ACPI Specification: UEFI GetMemoryMap() Boot Services Function](https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/uefi-getmemorymap-boot-services-function.html)
