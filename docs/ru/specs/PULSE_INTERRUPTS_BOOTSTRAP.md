<p align="center">
  <a href="../../en/specs/PULSE_INTERRUPTS_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap interrupts Pulse для x86_64

**Статус:** Реализован и проверен в первом профиле x86_64 QEMU UEFI.

Pulse настраивает полную Interrupt Descriptor Table из 256 записей сразу после включения собственного раннего paging. Все vector изначально указывают на no-return diagnostic handler, а vector `3` использует выделенный breakpoint handler. Code selector считывается из активного CPU context, а не жёстко задаётся. Intel System Programming Guide описывает IDT, gate descriptor и доставку interrupts/exceptions в IA-32e mode.[1]

## Граница проверки

Maskable interrupts остаются выключенными. После загрузки `IDTR` Pulse намеренно выполняет `INT3`; breakpoint handler выводит в тестовый канал QEMU строку `PULSE: breakpoint trap handled` и завершает emulator. Это подтверждает активную IDT, gate offset, selector и exception control transfer без преждевременного включения device interrupts.

## Текущие ограничения

В bootstrap пока нет register-frame capture, normalisation error code, return path, IST stack, PIC/APIC routing, timer tick и recovery UI. Default handler намеренно является terminal. Эти возможности будут добавлены до того, как Pulse включит external interrupts или scheduling work.

## Источник

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
