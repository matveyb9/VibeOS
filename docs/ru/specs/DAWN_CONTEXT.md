<p align="center">
  <a href="../../en/specs/DAWN_CONTEXT.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Dawn Context v5

**Статус:** Version 5 реализован и для UEFI Prelude profile, и для воспроизводимого QEMU/SeaBIOS Legacy BIOS profile.

> **Dawn Context** — единая immutable handoff record от Prelude к Pulse. Она не содержит callable firmware interface и использует только physical address.

Version 3 заменил UEFI memory-descriptor layout на принадлежащий VibeOS массив `DAWN_MEMORY_DESCRIPTOR`. Version 4 добавил вторым append-only массив `DAWN_MEMORY_RANGE` для boot-owned physical reservation. Version 5 добавляет physical address одного проверенного ACPI Root System Description Pointer (RSDP). Так firmware-specific discovery остаётся в Prelude, а record остаётся portable и не зависит от callable firmware interface. Это metadata для будущего ACPI consumer, а не реализация ACPI.[1]

## Бинарная структура

| Поле | Тип | Значение |
|---|---:|---|
| `magic` | `uint64_t` | Константа `DAWN_CONTEXT_MAGIC`. |
| `version` | `uint32_t` | Текущая contract version: `5`. |
| `size` | `uint32_t` | Размер структуры producer; layout append-only. |
| `memory_map_physical_address` | `uint64_t` | Physical address VibeOS-owned memory descriptor. |
| `memory_map_size` | `uint64_t` | Точный byte size этого descriptor array. |
| `memory_map_key` | `uint64_t` | Producer-private final-map token; UEFI Prelude использует его только при firmware exit. |
| `memory_descriptor_size` | `uint64_t` | Должен быть равен `sizeof(DAWN_MEMORY_DESCRIPTOR)`. |
| `memory_descriptor_version` | `uint32_t` | Должен быть равен `DAWN_MEMORY_DESCRIPTOR_VERSION`. |
| `kernel_stack_top` / `kernel_stack_size` | `uint64_t` | Location early Pulse stack и byte capacity. |
| framebuffer fields | mixed | Physical framebuffer address, byte size, dimension, pixel stride и format `RGBX8888`, `BGRX8888` или `BGR888`. |
| reservation fields | mixed | Physical address, byte size, descriptor stride/version и count упорядоченных boot-owned range. |
| `acpi_rsdp_physical_address` | `uint64_t` | Physical address ровно одного проверенного Prelude ACPI RSDP. Это не root-table pointer и не передаёт firmware API. |

| Поле `DAWN_MEMORY_DESCRIPTOR` | Значение |
|---|---|
| `physical_start` | Первый physical byte range. |
| `byte_size` | Точная длина в byte, не page count. |
| `kind` | `DAWN_MEMORY_USABLE` или `DAWN_MEMORY_RESERVED`. Unknown source type становятся reserved. |
| `attributes` | Зарезервировано для будущей VibeOS-owned memory metadata; сейчас zero. |

| Поле `DAWN_MEMORY_RANGE` | Значение |
|---|---|
| `physical_start` | Первый physical byte, занятый boot-owned range. |
| `byte_size` | Точная длина занятого range; до allocation Pulse расширяет range наружу до целых frame по 4 KiB. |

UEFI producer получает map через `GetMemoryMap`, преобразует UEFI conventional memory в `DAWN_MEMORY_USABLE` и консервативно отмечает все остальные descriptor как reserved перед `ExitBootServices`.[2] Он публикует отсортированные непересекающиеся reservation для fixed Pulse allocation и early stack. Legacy BIOS producer отображает E820 usable entry в тот же `DAWN_MEMORY_USABLE`, отмечает все остальные entry как reserved и публикует первый MiB вместе с загруженным Pulse image как boot-owned range. Первый range защищает active loader, Dawn Context, stack, VBE data и transition page table, даже когда E820 описывает часть памяти как RAM. В QEMU Legacy BIOS profile linear framebuffer VBE `0x118` передаётся как `BGR888`; этот format переносится тем же contract, а не BIOS-specific side channel. Поэтому Pulse потребляет одну architecture-neutral shape независимо от firmware path.

Prelude находит RSDP до final firmware exit. UEFI Prelude ищет EFI configuration table, сначала выбирая GUID для ACPI 2.0 или новее, а затем используя GUID ACPI 1.0 как fallback. Legacy BIOS Prelude сканирует 16-byte boundary в первом KiB EBDA, затем `0xe0000`–`0xfffff` — ровно по правилам IA-PC discovery.[1] Оба пути проверяют signature `RSD PTR ` и ACPI checksum; для revision 2 и выше дополнительно обязателен extended checksum 36-byte RSDP. Текущий x86_64 bootstrap требует, чтобы address укладывался в identity-mapped lower 4 GiB, и передаёт physical address через тот же Dawn shape.

> Этот milestone **не** разбирает RSDT/XSDT или какую-либо другую ACPI table. Он не выполняет AML, не включает ACPI, не управляет power state, не назначает PCI resource, не программирует interrupt routing и не заявляет совместимость с реальным hardware.

## Правила контракта

Producer должен заполнить все required field перед transfer control и никогда не раскрывать Pulse live firmware service. Reservation range должны быть nonempty, non-overlapping, упорядочены по physical start и не иметь integer overflow. Consumer проверяет magic, version, minimum size, normalized descriptor stride/version, nonempty map, valid reservation metadata, valid stack, valid framebuffer, nonzero RSDP address и RSDP signature/checksum до использования поля. Unknown future memory kind остаются unusable, пока следующий Pulse contract revision явно их не примет.

## Источники

[1] [ACPI Specification 6.5, §5.2.5: Root System Description Pointer](https://uefi.org/specs/ACPI/6.5/05_ACPI_Software_Programming_Model.html)

[2] [Спецификация UEFI: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)
