<p align="center">
  <a href="../../en/specs/DAWN_CONTEXT.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Dawn Context v1

**Статус:** Реализован в Prelude и проверяется первой точкой входа Pulse для x86_64.

`Dawn Context` — компактный версионированный boot-контракт, через который Prelude передаёт состояние платформы Pulse. Версия 1 содержит только UEFI memory map. Она намеренно не передаёт Pulse действующие UEFI Boot Services.

UEFI Boot Services доступны только до успешного вызова `ExitBootServices()`; после этого перехода OS loader принимает управление платформой.[1] Prelude получает memory map через `GetMemoryMap()`, который предоставляет описание ресурсов памяти, передаваемое OS loader дальше.[2]

## Бинарная структура

| Поле | Тип | Назначение |
|---|---|---|
| `magic` | `UINT64` | `DAWN_CONTEXT_MAGIC`, идентификатор бинарного контракта. |
| `version` | `UINT32` | Версия контракта, изначально `1`. |
| `size` | `UINT32` | Полный размер структуры на стороне producer. |
| `memory_map_physical_address` | `uint64_t` | Физический адрес UEFI memory descriptors, сохранённых для Pulse. |
| `memory_map_size` | `uint64_t` | Занятый размер сохранённой карты в байтах. |
| `memory_map_key` | `UINTN` | Ключ, с которым Boot Services были успешно закрыты. |
| `memory_descriptor_size` | `UINTN` | Шаг между дескрипторами. |
| `memory_descriptor_version` | `UINT32` | Версия UEFI-дескрипторов. |
| `reserved` | `UINT32` | В v1 содержит ноль. |

## Правила producer

Сначала Prelude запрашивает размер карты, выделяет `LoaderData`-буфер с дополнительной ёмкостью под дескрипторы, получает актуальные карту и ключ, затем вызывает `ExitBootServices()`. Если firmware отклоняет ключ из-за изменения карты, Prelude получает новую карту и повторяет попытку не более трёх раз. После успеха он не вызывает функции Boot Services.

Структура **только дополняется в конец**. Будущая сборка Pulse должна отклонять неизвестный `magic`, неподдерживаемую новую `version` и `size`, недостаточный для используемых полей. Будущий Prelude сможет добавлять поля без нарушения совместимости со старым Pulse, учитывающим размер.

## Текущая граница handoff

Этот этап запечатывает контекст, загружает первый нативный образ Pulse по физическому адресу `0x00200000` и подтверждает прямой переход в QEMU. Сейчас Pulse проверяет только версию контракта и метаданные memory map; physical memory allocation, paging, interrupts и task scheduling остаются следующими этапами Pulse.

## Источники

[1] [Спецификация UEFI: Boot Services](https://uefi.org/specs/UEFI/2.9_A/07_Services_Boot_Services.html)

[2] [Спецификация ACPI: UEFI GetMemoryMap() Boot Services Function](https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/15_System_Address_Map_Interfaces/uefi-getmemorymap-boot-services-function.html)
