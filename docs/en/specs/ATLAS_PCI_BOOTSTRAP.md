<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/ATLAS_PCI_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas PCI bootstrap

**Status:** Implemented as a bounded, read-only PCI configuration-space inventory for the reproducible x86_64 QEMU profiles.

Atlas separates a transport-specific configuration read from a neutral inventory scan. The x86_64 transport uses PCI configuration mechanism #1: it writes an enabled, DWORD-aligned address to `0xCF8` and reads its result from `0xCFC`. The scanner inventories functions on bus zero, treats vendor ID `0xffff` as absent, and reads identity, class, revision, and header type without modifying device state.[1] [2]

| Property | Current behavior |
|---|---|
| Inventory | Up to 32 discovered functions in a fixed caller-owned buffer |
| Functions | Function zero of every root-bus device; functions 1–7 only when the multi-function header flag is present |
| Fields | Bus/device/function, vendor/device ID, class, subclass, programming interface, revision, and header type |
| I/O | x86_64-only `CF8/CFC` read transport, isolated behind a C17 callback |
| Verification | Deterministic host fake-config test plus UEFI/OVMF and Legacy BIOS/SeaBIOS runtime markers |

The bootstrap performs **inventory only**. It does not enumerate behind PCI bridges, parse ACPI/MCFG, configure BARs, enable decoding or bus mastering, configure DMA, MSI/MSI-X or INTx, or bind drivers. Those functions require a later Atlas resource model and platform policy; absence from this step is intentional.

## References

[1] [UEFI Platform Initialization Specification 1.8: PCI configuration space](https://uefi.org/specs/PI/1.8/V5_Introduction.html)

[2] [OSDev Wiki: PCI configuration space access mechanism #1](https://wiki.osdev.org/PCI)
