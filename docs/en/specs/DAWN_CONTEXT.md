<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/DAWN_CONTEXT.md">🇷🇺 РУССКИЙ</a>
</p>

# Dawn Context v5

**Status:** Version 5 is implemented by both the UEFI Prelude profile and the reproducible QEMU/SeaBIOS Legacy BIOS profile.

> **Dawn Context** is the single immutable handoff record from Prelude to Pulse. It contains no callable firmware interface and uses physical addresses exclusively.

Version 3 replaced the UEFI memory-descriptor layout with a VibeOS-owned `DAWN_MEMORY_DESCRIPTOR` array. Version 4 appended a second VibeOS-owned `DAWN_MEMORY_RANGE` array for boot-owned physical reservations. Version 5 appends the physical address of one validated ACPI Root System Description Pointer (RSDP). This keeps firmware-specific discovery at Prelude while leaving the record portable and independent of callable firmware interfaces. It is metadata for a later ACPI consumer, not an ACPI implementation.[1]

## Binary layout

| Field | Type | Meaning |
|---|---:|---|
| `magic` | `uint64_t` | Constant `DAWN_CONTEXT_MAGIC`. |
| `version` | `uint32_t` | Current contract version: `5`. |
| `size` | `uint32_t` | Producer's structure size; the layout is append-only. |
| `memory_map_physical_address` | `uint64_t` | Physical address of VibeOS-owned memory descriptors. |
| `memory_map_size` | `uint64_t` | Exact byte size of that descriptor array. |
| `memory_map_key` | `uint64_t` | Producer-private final-map token; UEFI Prelude uses it only while exiting firmware. |
| `memory_descriptor_size` | `uint64_t` | Must equal `sizeof(DAWN_MEMORY_DESCRIPTOR)`. |
| `memory_descriptor_version` | `uint32_t` | Must equal `DAWN_MEMORY_DESCRIPTOR_VERSION`. |
| `kernel_stack_top` / `kernel_stack_size` | `uint64_t` | Early Pulse stack location and byte capacity. |
| framebuffer fields | mixed | Physical framebuffer address, byte size, dimensions, pixel stride, and `RGBX8888`, `BGRX8888`, or `BGR888` format. |
| reservation fields | mixed | Physical address, byte size, descriptor stride/version, and count for sorted boot-owned ranges. |
| `acpi_rsdp_physical_address` | `uint64_t` | Physical address of exactly one Prelude-validated ACPI RSDP. It is not a root-table pointer and does not transfer any firmware API. |

| `DAWN_MEMORY_DESCRIPTOR` field | Meaning |
|---|---|
| `physical_start` | First physical byte of the range. |
| `byte_size` | Exact byte length, not a page count. |
| `kind` | `DAWN_MEMORY_USABLE` or `DAWN_MEMORY_RESERVED`. Unknown source types become reserved. |
| `attributes` | Reserved for future VibeOS-owned memory metadata; currently zero. |

| `DAWN_MEMORY_RANGE` field | Meaning |
|---|---|
| `physical_start` | First physical byte occupied by the boot-owned range. |
| `byte_size` | Exact occupied byte length; Pulse rounds outward to whole 4 KiB frames before allocation. |

The UEFI producer obtains its map through `GetMemoryMap`, converts UEFI conventional memory to `DAWN_MEMORY_USABLE`, and conservatively marks every other descriptor as reserved before `ExitBootServices`.[2] It publishes sorted, non-overlapping reservations for the fixed Pulse allocation and the early stack. The Legacy BIOS producer maps E820 usable entries to the same `DAWN_MEMORY_USABLE` kind, reserves all other entries, and publishes the first MiB plus the loaded Pulse image as boot-owned ranges. The first range protects the active loader, Dawn Context, stack, VBE data, and transition page tables even when E820 describes part of it as RAM. The QEMU Legacy BIOS profile reports its VBE `0x118` linear framebuffer as `BGR888`; the format is carried by the same contract, not by a BIOS-specific side channel. Therefore Pulse consumes one architecture-neutral shape regardless of firmware path.

Prelude discovers the RSDP before final firmware exit. UEFI Prelude searches EFI configuration tables, preferring the ACPI 2.0-or-later GUID and then falling back to the ACPI 1.0 GUID. Legacy BIOS Prelude scans 16-byte boundaries in the first KiB of EBDA followed by `0xe0000`–`0xfffff`, exactly as specified for IA-PC discovery.[1] Both paths validate the `RSD PTR ` signature and ACPI checksum; revision 2 or later requires the 36-byte extended RSDP checksum. The current x86_64 bootstrap requires the resulting address to fit the identity-mapped lower 4 GiB, then transfers that physical address through the same Dawn shape.

Pulse selects a nonzero XSDT pointer from a revision-2-or-later RSDP, otherwise its RSDT pointer. It validates that selected root table’s expected signature, 36-byte minimum header, bounded total length, entry-width divisibility, and whole-table checksum, then extracts its physical address, byte length, kind, and entry count as transient metadata.[1] A separate injected-reader operation may copy no more than 64 RSDT/XSDT entries, retain each child common header’s physical address, signature, declared length, revision, and checksum state, and report omitted entries. When a validated `APIC` child is present, a separate read-only operation checks its full-table checksum, records its fixed 44-byte MADT metadata and retains at most 64 type-and-length entry headers; zero-length, truncated, overflowing, wrong-signature, and bad-checksum inputs are rejected. A distinct x86 decoder reads only fixed-size Local APIC (type 0) and I/O APIC (type 1) fields into bounded metadata records; unrecognised entries remain generic. The current x86_64 Pulse reader deliberately accepts only lower-4-GiB physical addresses because early identity paging has the same boundary.

> MADT work observes only fixed metadata and generic entry headers; it does not interpret descriptor payloads. Child inventory is metadata only, not permission to access a described device. It does not evaluate AML, enable ACPI, program a local APIC or I/O APIC, start processors, create SMP state, configure power states, assign PCI resources, program interrupt routing, or provide a real-hardware compatibility claim.

## Contract rules

The producer must populate all required fields before transferring control and must never expose live firmware services to Pulse. Reservation ranges must be nonempty, non-overlapping, ordered by physical start, and free of integer overflow. The consumer validates magic, version, minimum size, normalized descriptor stride/version, a nonempty map, valid reservation metadata, valid stack, valid framebuffer, a nonzero RSDP address, and the RSDP signature/checksum before using the field. Unknown future memory kinds remain unusable until a later Pulse contract revision accepts them.

The current Pulse bootstrap allocator also treats the normalized memory map as a strict ordered physical partition: every raw descriptor must be nonempty, free of address overflow, and begin at or after the preceding raw descriptor limit. It rejects overlap and descending order rather than guessing precedence. After rounding usable descriptors inward to 4 KiB frames, it canonically merges directly adjacent usable intervals before allocation. This remains a bounded early allocator, not a general physical-memory manager, and does not yet provide frame release, zones, NUMA policy, or runtime memory hot-plug.

## References

[1] [ACPI Specification 6.5, §5.2.5: Root System Description Pointer](https://uefi.org/specs/ACPI/6.5/05_ACPI_Software_Programming_Model.html)

[2] [UEFI Specification: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)
