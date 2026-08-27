<p align="center">
  <a href="../../en/specs/PULSE_TIMER_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap внешнего timer Pulse

**Статус:** Реализован и проверен в начальном профиле x86_64 QEMU.

Теперь Pulse проверяет первый маршрут external interrupt. После возврата cooperative context probe он переназначает legacy PIC, маскирует все IRQ кроме IRQ0, настраивает PIT на 100 Гц, включает maskable interrupts и останавливается. Следующий timer tick приходит в IDT vector `32`; выделенный terminal handler выводит `PULSE: timer interrupt handled` и завершает QEMU.

| Свойство | Начальное поведение |
|---|---|
| Источник interrupt | Legacy PIT channel 0 через master PIC IRQ0 |
| Vector | `32` (`0x20`) |
| Частота probe | 100 Гц, divisor `11931` |
| Включённые IRQ | Только IRQ0 |
| Handler | Terminal verification handler |
| Production timer | Пока не реализован |

Intel System Programming Guide описывает доставку interrupts, выбор gate и управление interrupt-enable; Pulse использует timer только после проверки IDT, собственного paging и terminal failure path.[1] Временный PIC/PIT route не является финальной timer architecture VibeOS. Pulse перейдёт на APIC-based routing, acknowledge interrupts, сбор tick и возврат из normalised trap frame до разрешения preemptive scheduling.

## Источник

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
