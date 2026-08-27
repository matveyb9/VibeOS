<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_PAGING_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse x86_64 paging bootstrap

**Status:** Implemented and verified in the first x86_64 QEMU UEFI profile.

After establishing its own stack and early physical-frame source, Pulse creates a four-level page-table hierarchy and loads its own PML4 address into `CR3`. This replaces firmware page-table ownership with a deliberately small Pulse-controlled address space. Intel's system programming guide documents four-level paging, the `CR3` root, paging-structure entries, and their translation semantics.[1]

## Initial mapping

| Property | Value |
|---|---|
| Virtual range | `0x00000000` through `0x3fffffff` |
| Physical range | Identical to the virtual range |
| Size | 1 GiB |
| Structure | PML4 → PDPT → page directory |
| Leaf size | 2 MiB pages |
| Permissions | Present and writable; executable while the early bootstrap remains active |
| LA57 | Explicitly rejected in this first four-level implementation |

The first GiB intentionally retains identity access to Pulse at 2 MiB, the loader-provided Dawn Context, the early stack, the active page tables, and QEMU's low I/O layout. Loading `CR3` flushes the active translation context before Pulse emits the verified diagnostic marker.

## Limits and next change

This is neither the final virtual-memory layout nor the final permission model. It has no higher-half kernel mapping, user address space, guard page, kernel mapping isolation, or write/execute separation. The next memory step will reserve all boot-owned ranges and construct an ownership-aware physical allocator before narrower mappings are introduced.

## Reference

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
