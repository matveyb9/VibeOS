<p align="center">
  <a href="../../en/specs/PULSE_PAGING_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap paging Pulse для x86_64

**Статус:** Реализован и проверен в первом профиле x86_64 QEMU UEFI.

После настройки собственного стека и раннего источника physical frame Pulse создаёт четырёхуровневую иерархию page table и загружает адрес собственного PML4 в `CR3`. Так управление page table переходит от firmware к намеренно небольшому адресному пространству Pulse. Intel System Programming Guide описывает четырёхуровневый paging, корень `CR3`, записи paging-structure и семантику их трансляции.[1]

## Начальное отображение

| Свойство | Значение |
|---|---|
| Виртуальный диапазон | от `0x00000000` до `0x3fffffff` |
| Физический диапазон | Идентичен виртуальному диапазону |
| Размер | 1 ГиБ |
| Структура | PML4 → PDPT → page directory |
| Размер leaf | Страницы по 2 МиБ |
| Права | Present и writable; executable, пока активен ранний bootstrap |
| LA57 | Явно отклоняется в этой первой четырёхуровневой реализации |

Первый ГиБ намеренно сохраняет identity access к Pulse по адресу 2 МиБ, Dawn Context от loader, раннему стеку, активным page table и low I/O layout QEMU. Загрузка `CR3` сбрасывает активный translation context перед тем, как Pulse выведет проверяемую diagnostic marker.

## Ограничения и следующее изменение

Это не финальная virtual-memory layout и не финальная permission model. Здесь нет higher-half kernel mapping, user address space, guard page, kernel mapping isolation и write/execute separation. На следующем шаге памяти будут зарезервированы все boot-owned range и создан physical allocator с учётом владения, до появления более узких mappings.

## Источник

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
