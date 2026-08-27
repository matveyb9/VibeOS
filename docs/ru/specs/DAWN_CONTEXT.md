<p align="center">
  <a href="../../en/specs/DAWN_CONTEXT.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Dawn Context v2

**Статус:** Реализован в Prelude и проверен начальной точкой входа Pulse x86_64.

`Dawn Context` — компактный версионированный boot-контракт, через который Prelude передаёт состояние платформы Pulse. Версия 2 содержит UEFI memory map, descriptor раннего kernel stack и descriptor framebuffer Graphics Output Protocol, выбранного firmware. Она намеренно не передаёт Pulse действующие UEFI Boot Services.

UEFI Boot Services доступны только до успешного вызова `ExitBootServices()`; после этого перехода OS loader принимает управление платформой.[1] Prelude получает memory map через `GetMemoryMap()`, который предоставляет описание ресурсов памяти, передаваемое OS loader дальше.[2]

## Бинарная структура

| Поле | Тип | Назначение |
|---|---|---|
| `magic` | `UINT64` | `DAWN_CONTEXT_MAGIC`, идентификатор бинарного контракта. |
| `version` | `UINT32` | Текущая версия контракта: `2`. |
| `size` | `UINT32` | Полный размер структуры на стороне producer. |
| `memory_map_physical_address` | `uint64_t` | Физический адрес UEFI memory descriptors, сохранённых для Pulse. |
| `memory_map_size` | `uint64_t` | Занятый размер сохранённой карты в байтах. |
| `memory_map_key` | `UINTN` | Ключ, с которым Boot Services были успешно закрыты. |
| `memory_descriptor_size` | `UINTN` | Шаг между дескрипторами. |
| `memory_descriptor_version` | `UINT32` | Версия UEFI-дескрипторов. |
| `reserved` | `UINT32` | Нуль в v2. |
| `kernel_stack_top` | `uint64_t` | Физический адрес вершины начального стека Pulse. |
| `kernel_stack_size` | `uint64_t` | Размер начального стека в байтах. |
| `framebuffer_physical_address` | `uint64_t` | Physical base GOP framebuffer. |
| `framebuffer_byte_size` | `uint64_t` | Допустимый byte size GOP framebuffer. |
| `framebuffer_width` / `framebuffer_height` | `uint32_t` | Current visible pixel dimension. |
| `framebuffer_pixels_per_scan_line` | `uint32_t` | Physical scan-line stride в pixel. |
| `framebuffer_pixel_format` | `uint32_t` | Framebuffer format v2: `RGBX8888` или `BGRX8888`. |

## Правила producer

Сначала Prelude запрашивает размер карты, выделяет `LoaderData`-буфер с дополнительной ёмкостью под дескрипторы, получает актуальные карту и ключ, затем вызывает `ExitBootServices()`. Если firmware отклоняет ключ из-за изменения карты, Prelude получает новую карту и повторяет попытку не более трёх раз. После успеха он не вызывает функции Boot Services.

Структура **только дополняется в конец**. Будущая сборка Pulse должна отклонять неизвестный `magic`, неподдерживаемую новую `version` и `size`, недостаточный для используемых полей. Будущий Prelude сможет добавлять поля без нарушения совместимости со старым Pulse, учитывающим размер.

## Текущая граница handoff

Этот этап запечатывает context, загружает первый нативный образ Pulse по физическому адресу `0x00200000` и подтверждает прямой переход в QEMU. Prelude выделяет отдельный stack Pulse размером 128 КиБ и захватывает active GOP framebuffer до получения final memory map. Pulse валидирует contract, запускает ранний software framebuffer path, выбирает EFI conventional memory, строит собственные page table и начинает interrupt bootstrap.

## Источники

[1] [Спецификация UEFI: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)

[2] [Спецификация ACPI: UEFI GetMemoryMap() Boot Services Function](https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/uefi-getmemorymap-boot-services-function.html)
