<p align="center">
  <a href="../../en/specs/DAWN_CONTEXT.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Dawn Context v4

**Статус:** Version 4 реализован и для UEFI Prelude profile, и для воспроизводимого QEMU/SeaBIOS Legacy BIOS profile.

> **Dawn Context** — единая immutable handoff record от Prelude к Pulse. Она не содержит callable firmware interface и использует только physical address.

Version 3 заменил UEFI memory-descriptor layout на принадлежащий VibeOS массив `DAWN_MEMORY_DESCRIPTOR`. Version 4 добавляет вторым append-only массив `DAWN_MEMORY_RANGE` для boot-owned physical reservation. Благодаря этому Pulse и следующие boot path не зависят от firmware descriptor type, padding и version. Prelude преобразует каждую firmware memory record до final firmware exit; затем Pulse доверяет только normalized result.[1]

## Бинарная структура

| Поле | Тип | Значение |
|---|---:|---|
| `magic` | `uint64_t` | Константа `DAWN_CONTEXT_MAGIC`. |
| `version` | `uint32_t` | Текущая contract version: `4`. |
| `size` | `uint32_t` | Размер структуры producer; layout append-only. |
| `memory_map_physical_address` | `uint64_t` | Physical address VibeOS-owned memory descriptor. |
| `memory_map_size` | `uint64_t` | Точный byte size этого descriptor array. |
| `memory_map_key` | `uint64_t` | Producer-private final-map token; UEFI Prelude использует его только при firmware exit. |
| `memory_descriptor_size` | `uint64_t` | Должен быть равен `sizeof(DAWN_MEMORY_DESCRIPTOR)`. |
| `memory_descriptor_version` | `uint32_t` | Должен быть равен `DAWN_MEMORY_DESCRIPTOR_VERSION`. |
| `kernel_stack_top` / `kernel_stack_size` | `uint64_t` | Location early Pulse stack и byte capacity. |
| framebuffer fields | mixed | Physical framebuffer address, byte size, dimension, pixel stride и format `RGBX8888`, `BGRX8888` или `BGR888`. |
| reservation fields | mixed | Physical address, byte size, descriptor stride/version и count упорядоченных boot-owned range. |

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

UEFI producer получает map через `GetMemoryMap`, преобразует UEFI conventional memory в `DAWN_MEMORY_USABLE` и консервативно отмечает все остальные descriptor как reserved перед `ExitBootServices`.[1] Он публикует отсортированные непересекающиеся reservation для fixed Pulse allocation и early stack. Legacy BIOS producer отображает E820 usable entry в тот же `DAWN_MEMORY_USABLE`, отмечает все остальные entry как reserved и публикует первый MiB вместе с загруженным Pulse image как boot-owned range. Первый range защищает active loader, Dawn Context, stack, VBE data и transition page table, даже когда E820 описывает часть памяти как RAM. В QEMU Legacy BIOS profile linear framebuffer VBE `0x118` передаётся как `BGR888`; этот format переносится тем же contract, а не BIOS-specific side channel. Поэтому Pulse потребляет одну architecture-neutral shape независимо от firmware path.

## Правила контракта

Producer должен заполнить все required field перед transfer control и никогда не раскрывать Pulse live firmware service. Reservation range должны быть nonempty, non-overlapping, упорядочены по physical start и не иметь integer overflow. Consumer проверяет magic, version, minimum size, normalized descriptor stride/version, nonempty map, valid reservation metadata, valid stack и valid framebuffer до использования address. Unknown future memory kind остаются unusable, пока следующий Pulse contract revision явно их не примет.

## Источники

[1] [Спецификация UEFI: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)
