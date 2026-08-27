<p align="center">
  <a href="../../en/specs/ATLAS_PCI_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Atlas PCI

**Статус:** Реализован как ограниченный read-only inventory PCI configuration space для воспроизводимых x86_64 QEMU profile.

Atlas отделяет transport-зависимое чтение configuration от нейтрального inventory scan. x86_64 transport использует PCI configuration mechanism #1: записывает enabled DWORD-aligned address в `0xCF8` и читает результат из `0xCFC`. Scanner инвентаризирует function на bus zero, считает vendor ID `0xffff` отсутствующей function и читает identity, class, revision и header type без изменения device state.[1] [2]

| Свойство | Текущее поведение |
|---|---|
| Inventory | До 32 найденных function в fixed caller-owned buffer |
| Function | Function zero каждого root-bus device; function 1–7 только при multi-function header flag |
| Поля | Bus/device/function, vendor/device ID, class, subclass, programming interface, revision и header type |
| Topology | Bounded breadth-first scan bus zero и уже пронумерованных secondary bus PCI-to-PCI bridge; visited map на 256 byte предотвращает повторные циклы |
| I/O | x86_64-only `CF8/CFC` read transport, изолированный C17 callback |
| Проверка | Детерминированный host fake-config test и runtime marker UEFI/OVMF, Legacy BIOS/SeaBIOS |

Bootstrap выполняет **только read-only inventory**. Он следует за Type-1 PCI-to-PCI bridge лишь когда firmware уже предоставляет ненулевой secondary bus number; он никогда не назначает, не изменяет и не исправляет нумерацию bus. Он не разбирает ACPI/MCFG, не настраивает BAR, не включает decoding или bus mastering, не настраивает DMA, MSI/MSI-X либо INTx и не привязывает driver. Эти функции требуют будущей Atlas resource model и platform policy; их отсутствие на этом шаге намеренно.

## Источники

[1] [UEFI Platform Initialization Specification 1.8: PCI configuration space](https://uefi.org/specs/PI/1.8/V5_Introduction.html)

[2] [OSDev Wiki: PCI configuration space access mechanism #1](https://wiki.osdev.org/PCI)
