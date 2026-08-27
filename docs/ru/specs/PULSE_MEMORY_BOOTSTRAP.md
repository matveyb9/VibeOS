<p align="center">
  <a href="../../en/specs/PULSE_MEMORY_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Ранний bootstrap памяти Pulse

**Статус:** Реализован для первого профиля x86_64 QEMU UEFI.

Первый memory layer Pulse намеренно является bootstrap, а не готовым универсальным allocator. Он принимает UEFI memory map из Dawn Context, запоминает до 32 пригодных descriptor `EfiConventionalMemory` и выделяет физические frame по 4 КиБ в порядке возрастания descriptor. Первая загрузка проверяет handoff выделением двух соседних frame.

UEFI memory map описывает установленную RAM и зарезервированные firmware физические диапазоны; типы дескрипторов определяют, как операционная система должна обрабатывать каждый диапазон.[1] Поэтому на этом этапе Pulse выделяет память только из descriptor type `7`, `EfiConventionalMemory`, а остальные диапазоны считает недоступными.

## Состояние

| Поле | Назначение |
|---|---|
| `selected_region_base` | Начало текущей conventional-memory region. |
| `selected_region_limit` | Исключающая конец текущей области. |
| `next_free_frame` | Следующий frame, который выдаст bootstrap allocator. |
| `usable_page_count` | Общее количество учитываемых frame по 4 КиБ во всех сохранённых областях. |
| `region_count` | Количество сохранённых conventional-memory region, не более 32. |

## Явные ограничения

Bootstrap не объединяет смежные области, не резервирует страницы, которые будут заняты поздними подсистемами Pulse, не поддерживает освобождение и не предоставляет ownership bitmap. Это узкая предпосылка следующих этапов x86_64 page table и interrupts. Будущая подсистема памяти Pulse заменит его physical allocator с учётом владения, сохранив границу ABI Dawn Context.

Цель `make test` в репозитории запускает host-side unit-тесты проверки дескрипторов, выравнивания страниц, смежности и обработки конца области до QEMU integration probe.

## Источник

[1] [Спецификация ACPI: UEFI GetMemoryMap() Boot Services Function](https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/uefi-getmemorymap-boot-services-function.html)
