<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_PAGING_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse x86_64 paging bootstrap

**Status:** Implemented and verified in the x86_64 QEMU UEFI and Legacy BIOS profiles.

After establishing its own stack and early physical-frame source, Pulse creates a four-level page-table hierarchy and loads its own PML4 address into `CR3`. Intel's system programming guide documents four-level paging, the `CR3` root, paging-structure entries, and their translation semantics.[1]

## Initial mapping

| Property | Value |
|---|---|
| Virtual range | `0x00000000` through `0xffffffff` |
| Physical range | Identical to the virtual range |
| Size | 4 GiB |
| Structure | PML4 → PDPT → four page directories |
| Leaf size | 2 MiB pages |
| Permissions | Present and writable; executable while the early bootstrap remains active |
| LA57 | Explicitly rejected in this first four-level implementation |

The four-GiB identity range preserves access to Pulse at 2 MiB, Dawn Context, the early stack, active page tables, QEMU's low I/O layout, and the firmware framebuffer range used by the first Prism software-renderer milestone. Dawn Context v4 carries explicit boot-owned physical ranges: the early allocator rounds them to frames and never returns a covered frame. The Legacy BIOS producer therefore protects its first MiB bootstrap region even when firmware exposes it as usable RAM. The map is intentionally broad and includes regions that later Pulse mappings will classify as RAM or MMIO before applying narrower permissions.

## Limits and next change

This is neither the final virtual-memory layout nor the final permission model. It has no higher-half kernel mapping, user address space, guard page, kernel mapping isolation, or write/execute separation. The next memory step will reserve boot-owned ranges and construct an ownership-aware physical allocator before narrower mappings are introduced.

## Reference

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
