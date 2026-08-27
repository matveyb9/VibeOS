<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/ATLAS_PCI_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas PCI bootstrap

**Status:** Implemented as a bounded, read-only PCI configuration-space and already-programmed resource inventory for the reproducible x86_64 QEMU profiles.

Atlas separates a transport-specific configuration read from a neutral inventory scan. The x86_64 transport uses PCI configuration mechanism #1: it writes an enabled, DWORD-aligned address to `0xCF8` and reads its result from `0xCFC`. The scanner inventories functions on bus zero, treats vendor ID `0xffff` as absent, and reads identity, class, revision, and header type without modifying device state.[1] [2]

| Property | Current behavior |
|---|---|
| Inventory | Up to 32 discovered functions in a fixed caller-owned buffer |
| Functions | Function zero of every root-bus device; functions 1–7 only when the multi-function header flag is present |
| Fields | Bus/device/function, vendor/device ID, class, subclass, programming interface, revision, and header type |
| Topology | Bounded breadth-first scan of bus zero and already numbered PCI-to-PCI bridge secondary buses; a 256-byte visited map prevents re-entry cycles |
| Resource records | Caller-owned bounded records preserving function location, BAR/aperture index, resource kind, flags, observed base, and observed limit |
| BAR classification | Reads only the current Type-0 six or Type-1 two standard BAR dwords; classifies I/O, 32-bit memory, and 64-bit memory BARs, including prefetchability |
| Bridge apertures | Reconstructs valid Type-1 I/O, non-prefetchable-memory, and prefetchable-memory base/limit windows without changing the bridge |
| I/O | x86_64-only `CF8/CFC` read transport, isolated behind a C17 callback |
| Verification | Deterministic host fake-config test plus UEFI/OVMF and Legacy BIOS/SeaBIOS runtime markers |

The bootstrap performs **read-only inventory only**. It follows a Type-1 PCI-to-PCI bridge only when firmware has already provided a nonzero secondary bus number; it never assigns, changes, or repairs bus numbering. For every standard BAR it reports only the observed current base and encoding; a zero base remains an unassigned observation. A 64-bit memory BAR consumes its adjacent BAR dword in the inventory. Bridge apertures are emitted only when their reconstructed base does not exceed their limit; malformed values are retained by neither repair nor inference.[3]

> The resource records are intentionally **not allocations, mappings, or authorisations to access hardware**. This milestone does not perform BAR size probing (which would write configuration space), parse ACPI/MCFG, enable decoding or bus mastering, configure DMA, MSI/MSI-X or INTx, assign bridge resources, or bind drivers.

## References

[1] [UEFI Platform Initialization Specification 1.8: PCI configuration space](https://uefi.org/specs/PI/1.8/V5_Introduction.html)

[2] [OSDev Wiki: PCI configuration space access mechanism #1](https://wiki.osdev.org/PCI)

[3] [PCI Local Bus Specification, Revision 2.2, Chapter 6: Configuration Space](https://ics.uci.edu/~iharris/ics216/pci/PCI_22.pdf)
