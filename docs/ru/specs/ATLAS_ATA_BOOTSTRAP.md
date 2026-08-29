<p align="center">
  <a href="../../en/specs/ATLAS_ATA_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Atlas ATA PIO

**Статус:** Реализован как bounded, read-only ATA `IDENTIFY DEVICE` probe плюс one-sector LBA28 read для воспроизводимого x86_64 QEMU PC IDE profile.

Теперь Atlas имеет controller-specific boundary, отдельную от RAM block backend и VaultFS. C17-facing transport предоставляет 8-bit port read/write и 16-bit data read; x86_64 adapter содержит inline I/O instructions. Independent protocol выполняет только ATA command `IDENTIFY DEVICE` (`0xEC`) на legacy primary task-file port и передаёт ровно 256 response words после bounded status polling.[1] [2]

| Свойство | Текущее поведение |
|---|---|
| Scope | Primary legacy task-file command base `0x1F0`, control/alternate-status base `0x3F6`, device 0 или 1 |
| Preflight | Отклоняет floating-bus observation `0xFF` до command write |
| Command | Выбирает requested device, очищает identify task-file parameters, затем передаёт только `IDENTIFY DEVICE` (`0xEC`) |
| Polling | Пятнадцать alternate-status read после selection и fixed maximum 1 000 000 polls для BSY clear и DRQ |
| Rejection | Отклоняет no-device status, `ERR`, `DF`, bounded timeout, nonzero LBA mid/high signature, invalid input и отсутствие usable LBA sector count |
| Response | Читает ровно 256 16-bit PIO words и сообщает nonzero LBA28 capacity либо supported/nonzero LBA48 capacity |
| Sector read | Один caller-owned 512-byte-equivalent LBA28 buffer с address ниже `2^28` и ниже validated capacity |
| Handle | Immutable value с port coordinates, device select, validated capacity, format version и nonzero caller-supplied identity fingerprint |
| Adapter | Принимает ровно один sector-read request только после exact match handle, identify metadata и fingerprint; делегирует PIO primitive |
| Verification | Deterministic injected-transport host test и отдельный UEFI/OVMF QEMU PC IDE identify-and-read probe с temporary 1 MiB raw disk |

Host fixture проверяет LBA28 response, предпочитает supported nonzero LBA48 capacity, admits immutable device handle, отклоняет coordinate или fingerprint mismatch, читает один sector через LBA28 command и отклоняет floating bus, ATA error status, invalid device selection, overflowing command-base range и первый invalid LBA28 address. QEMU check запускает отдельный image `PULSE_PROBE_storage` на QEMU machine `pc`, подключает empty temporary IDE disk как primary device zero, выполняет IDENTIFY и затем один LBA28 sector read на LBA zero, наблюдает `ATLAS: ATA identify and sector read verified` и проверяет standard debug-exit status. Документация QEMU описывает PC machine как предоставляющую PCI IDE interface с hard-disk support.[3]

> Это **physical controller identity/capacity plus one-sector read probe**, а не general disk driver. Он не выдаёт multi-sector или write command, не использует DMA, не reset bus, не изменяет interrupt policy, не configures PCI, не parse partition, не bind Atlas block device, не exposes general ATA sector interface и не connect physical device к VaultFS.

Handle является admission value, а не universal security identity или global ownership registry. Adapter отклоняет любой request, если handle coordinates, device select, identify capacity или fingerprint не совпадают, и оставляет caller buffer неизменным при таком admission failure. Adapter не устанавливает durability, media ownership за пределами admitted coordinates, storage trust, hot-plug behavior, controller discovery за пределами fixed QEMU legacy range, cache behavior, error recovery или compatibility с arbitrary ATA/SATA/AHCI hardware. Writable persistence path дополнительно требует explicit physical flush и crash-consistency model.

## References

[1] [OSDev Wiki: ATA PIO Mode — detection, task-file registers, status bits и IDENTIFY sequence](https://wiki.osdev.org/ATA_PIO_Mode)

[2] [Microsoft: `IDENTIFY_DEVICE_DATA` — IDENTIFY command value и capacity fields](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ata/ns-ata-_identify_device_data)

[3] [QEMU User Documentation — PC machine peripheral model](https://www.qemu.org/docs/master/system/qemu-manpage.html)
