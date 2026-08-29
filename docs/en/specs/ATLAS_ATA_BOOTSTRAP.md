<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/ATLAS_ATA_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas ATA PIO bootstrap

**Status:** Implemented as a bounded, read-only ATA `IDENTIFY DEVICE` probe for the reproducible x86_64 QEMU PC IDE profile.

Atlas now has a controller-specific boundary that is separate from its RAM block backend and VaultFS. A C17-facing transport provides 8-bit port reads/writes and 16-bit data reads; the x86_64 adapter contains the inline I/O instructions. The independent protocol performs only the ATA `IDENTIFY DEVICE` command (`0xEC`) on the legacy primary task-file ports and transfers exactly 256 response words after bounded status polling.[1] [2]

| Property | Current behavior |
|---|---|
| Scope | Primary legacy task-file command base `0x1F0`, control/alternate-status base `0x3F6`, device 0 or 1 |
| Preflight | Rejects the `0xFF` floating-bus observation before emitting command writes |
| Command | Selects the requested device, clears the identify task-file parameters, then emits only `IDENTIFY DEVICE` (`0xEC`) |
| Polling | Fifteen alternate-status reads after selection and a fixed maximum of 1,000,000 polls for BSY clear and DRQ |
| Rejection | Rejects no-device status, `ERR`, `DF`, bounded timeout, nonzero LBA mid/high signature, invalid input, or no usable LBA sector count |
| Response | Reads exactly 256 16-bit PIO words and reports nonzero LBA28 capacity or supported/nonzero LBA48 capacity |
| Verification | Deterministic injected-transport host test plus a dedicated UEFI/OVMF QEMU PC IDE probe with a temporary 1 MiB raw disk |

The host fixture validates a LBA28 response, prefers a supported nonzero LBA48 capacity, and rejects a floating bus, an ATA error status, invalid device selection, and an overflowing command-base range. The QEMU check starts a dedicated `PULSE_PROBE_storage` image on the QEMU `pc` machine, attaches an empty temporary IDE disk at primary device zero, observes `ATLAS: ATA identify verified`, and checks the standard debug-exit status. QEMU documents the PC machine as providing PCI IDE interfaces with hard-disk support.[3]

> This is a **physical controller identity/capacity probe**, not a general disk driver. It does not issue sector read or write commands, use DMA, reset the bus, change interrupt policy, configure PCI, parse partitions, bind an Atlas block device, expose an ATA sector interface, or connect any physical device to VaultFS.

The probe does not establish durability, media ownership, storage trust, hot-plug behavior, controller discovery outside the fixed QEMU legacy range, cache behavior, error recovery, or compatibility with arbitrary ATA/SATA/AHCI hardware. Any later read path must obtain its own bounded addressing, buffer, failure and device-selection contract; any writable persistence path additionally needs an explicit physical flush and crash-consistency model.

## References

[1] [OSDev Wiki: ATA PIO Mode — detection, task-file registers, status bits, and IDENTIFY sequence](https://wiki.osdev.org/ATA_PIO_Mode)

[2] [Microsoft: `IDENTIFY_DEVICE_DATA` — IDENTIFY command value and capacity fields](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ata/ns-ata-_identify_device_data)

[3] [QEMU User Documentation — PC machine peripheral model](https://www.qemu.org/docs/master/system/qemu-manpage.html)
