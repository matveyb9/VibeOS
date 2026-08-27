<p align="center">
  <a href="../../en/specs/PULSE_PAGING_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap paging Pulse x86_64

**Статус:** Реализован и проверен в начальном профиле x86_64 QEMU UEFI.

После создания собственного stack и раннего physical-frame source Pulse формирует four-level page-table hierarchy и загружает свой адрес PML4 в `CR3`. Intel System Programming Guide описывает four-level paging, root `CR3`, paging-structure entry и их translation semantics.[1]

## Начальное mapping

| Свойство | Значение |
|---|---|
| Virtual range | `0x00000000` до `0xffffffff` |
| Physical range | Идентичен virtual range |
| Размер | 4 ГиБ |
| Structure | PML4 → PDPT → четыре page directory |
| Leaf size | Page по 2 МиБ |
| Permissions | Present и writable; executable, пока действует ранний bootstrap |
| LA57 | Явно отклоняется в этой первой four-level implementation |

Identity range четырёх ГиБ сохраняет доступ к Pulse на 2 МиБ, Dawn Context, раннему stack, активным page table, low I/O layout QEMU и firmware framebuffer range для первой Prism software-renderer milestone. Он намеренно широк и включает region, которые будущие mapping Pulse классифицируют как RAM или MMIO до применения более узких permissions.

## Ограничения и следующее изменение

Это не финальный virtual-memory layout и не финальная permission model. Здесь нет higher-half kernel mapping, user address space, guard page, kernel mapping isolation и write/execute separation. Следующий memory step зарезервирует boot-owned range и построит ownership-aware physical allocator до введения более узких mapping.

## Источник

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
