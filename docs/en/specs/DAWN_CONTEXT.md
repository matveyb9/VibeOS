<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/DAWN_CONTEXT.md">🇷🇺 РУССКИЙ</a>
</p>

# Dawn Context v3

**Status:** Version 3 is implemented by both the UEFI Prelude profile and the reproducible QEMU/SeaBIOS Legacy BIOS profile.

> **Dawn Context** is the single immutable handoff record from Prelude to Pulse. It contains no callable firmware interface and uses physical addresses exclusively.

Version 3 replaces the UEFI memory-descriptor layout with a VibeOS-owned `DAWN_MEMORY_DESCRIPTOR` array. This avoids making Pulse or later boot paths depend on UEFI's descriptor type, padding, or version. Prelude converts every firmware memory record before the final firmware exit; Pulse then trusts only the normalized result.[1]

## Binary layout

| Field | Type | Meaning |
|---|---:|---|
| `magic` | `uint64_t` | Constant `DAWN_CONTEXT_MAGIC`. |
| `version` | `uint32_t` | Current contract version: `3`. |
| `size` | `uint32_t` | Producer's structure size; the layout is append-only. |
| `memory_map_physical_address` | `uint64_t` | Physical address of VibeOS-owned memory descriptors. |
| `memory_map_size` | `uint64_t` | Exact byte size of that descriptor array. |
| `memory_map_key` | `uint64_t` | Producer-private final-map token; UEFI Prelude uses it only while exiting firmware. |
| `memory_descriptor_size` | `uint64_t` | Must equal `sizeof(DAWN_MEMORY_DESCRIPTOR)`. |
| `memory_descriptor_version` | `uint32_t` | Must equal `DAWN_MEMORY_DESCRIPTOR_VERSION`. |
| `kernel_stack_top` / `kernel_stack_size` | `uint64_t` | Early Pulse stack location and byte capacity. |
| framebuffer fields | mixed | Physical framebuffer address, byte size, dimensions, pixel stride, and `RGBX8888`, `BGRX8888`, or `BGR888` format. |

| `DAWN_MEMORY_DESCRIPTOR` field | Meaning |
|---|---|
| `physical_start` | First physical byte of the range. |
| `byte_size` | Exact byte length, not a page count. |
| `kind` | `DAWN_MEMORY_USABLE` or `DAWN_MEMORY_RESERVED`. Unknown source types become reserved. |
| `attributes` | Reserved for future VibeOS-owned memory metadata; currently zero. |

The UEFI producer obtains its map through `GetMemoryMap`, converts UEFI conventional memory to `DAWN_MEMORY_USABLE`, and conservatively marks every other descriptor as reserved before `ExitBootServices`.[1] The Legacy BIOS producer maps E820 usable entries to the same `DAWN_MEMORY_USABLE` kind, reserves all other entries, and reserves the first MiB even when E820 describes it as RAM. That bootstrap reservation protects the active loader, Dawn Context, stack, VBE data, and transition page tables. The QEMU Legacy BIOS profile reports its VBE `0x118` linear framebuffer as `BGR888`; the format is carried by the same contract, not by a BIOS-specific side channel. Therefore Pulse consumes one architecture-neutral shape regardless of firmware path.

## Contract rules

The producer must populate all required fields before transferring control and must never expose live firmware services to Pulse. The consumer validates magic, version, minimum size, normalized descriptor stride/version, nonempty map, valid stack, and valid framebuffer before using any address. Unknown future memory kinds remain unusable until a later Pulse contract revision accepts them.

## References

[1] [UEFI Specification: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)
