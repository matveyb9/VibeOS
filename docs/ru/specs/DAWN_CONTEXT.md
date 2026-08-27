<p align="center">
  <a href="../../en/specs/DAWN_CONTEXT.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Dawn Context v3

**Статус:** Version 3 реализован и для UEFI Prelude profile, и для воспроизводимого QEMU/SeaBIOS Legacy BIOS profile.

> **Dawn Context** — единая immutable handoff record от Prelude к Pulse. Она не содержит callable firmware interface и использует только physical address.

Version 3 заменяет UEFI memory-descriptor layout на принадлежащий VibeOS массив `DAWN_MEMORY_DESCRIPTOR`. Благодаря этому Pulse и следующие boot path не зависят от UEFI descriptor type, padding и version. Prelude преобразует каждую firmware memory record до final firmware exit; затем Pulse доверяет только normalized result.[1]

## Бинарная структура

| Поле | Тип | Значение |
|---|---:|---|
| `magic` | `uint64_t` | Константа `DAWN_CONTEXT_MAGIC`. |
| `version` | `uint32_t` | Текущая contract version: `3`. |
| `size` | `uint32_t` | Размер структуры producer; layout append-only. |
| `memory_map_physical_address` | `uint64_t` | Physical address VibeOS-owned memory descriptor. |
| `memory_map_size` | `uint64_t` | Точный byte size этого descriptor array. |
| `memory_map_key` | `uint64_t` | Producer-private final-map token; UEFI Prelude использует его только при firmware exit. |
| `memory_descriptor_size` | `uint64_t` | Должен быть равен `sizeof(DAWN_MEMORY_DESCRIPTOR)`. |
| `memory_descriptor_version` | `uint32_t` | Должен быть равен `DAWN_MEMORY_DESCRIPTOR_VERSION`. |
| `kernel_stack_top` / `kernel_stack_size` | `uint64_t` | Location early Pulse stack и byte capacity. |
| framebuffer fields | mixed | Physical framebuffer address, byte size, dimension, pixel stride и format `RGBX8888`, `BGRX8888` или `BGR888`. |

| Поле `DAWN_MEMORY_DESCRIPTOR` | Значение |
|---|---|
| `physical_start` | Первый physical byte range. |
| `byte_size` | Точная длина в byte, не page count. |
| `kind` | `DAWN_MEMORY_USABLE` или `DAWN_MEMORY_RESERVED`. Unknown source type становятся reserved. |
| `attributes` | Зарезервировано для будущей VibeOS-owned memory metadata; сейчас zero. |

UEFI producer получает map через `GetMemoryMap`, преобразует UEFI conventional memory в `DAWN_MEMORY_USABLE` и консервативно отмечает все остальные descriptor как reserved перед `ExitBootServices`.[1] Legacy BIOS producer отображает E820 usable entry в тот же `DAWN_MEMORY_USABLE`, отмечает все остальные entry как reserved и резервирует первый MiB, даже если E820 описывает его как RAM. Это bootstrap-резервирование защищает active loader, Dawn Context, stack, VBE data и transition page table. В QEMU Legacy BIOS profile linear framebuffer VBE `0x118` передаётся как `BGR888`; этот format переносится тем же contract, а не BIOS-specific side channel. Поэтому Pulse потребляет одну architecture-neutral shape независимо от firmware path.

## Правила контракта

Producer должен заполнить все required field перед transfer control и никогда не раскрывать Pulse live firmware service. Consumer проверяет magic, version, minimum size, normalized descriptor stride/version, nonempty map, valid stack и valid framebuffer до использования address. Unknown future memory kind остаются unusable, пока следующий Pulse contract revision явно их не примет.

## Источники

[1] [Спецификация UEFI: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)
