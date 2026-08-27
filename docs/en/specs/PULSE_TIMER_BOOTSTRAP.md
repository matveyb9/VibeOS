<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_TIMER_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse external timer bootstrap

**Status:** Implemented and verified in the initial x86_64 QEMU profile.

Pulse now verifies its first external interrupt route. After the cooperative context probe returns, it remaps the legacy PIC, masks every IRQ except IRQ0, configures the PIT for 100 Hz, enables maskable interrupts, and halts. The next timer tick arrives at IDT vector `32`; a dedicated terminal handler emits `PULSE: timer interrupt handled` and exits QEMU.

| Property | Initial behavior |
|---|---|
| Interrupt source | Legacy PIT channel 0 through master PIC IRQ0 |
| Vector | `32` (`0x20`) |
| Probe rate | 100 Hz, divisor `11931` |
| Enabled IRQs | Only IRQ0 |
| Handler | Terminal verification handler |
| Production timer | Not yet implemented |

The Intel system programming guide documents interrupt delivery, gate selection, and interrupt-enable control; Pulse uses the timer only after the IDT, own paging, and terminal failure path are verified.[1] This temporary PIC/PIT route is not the final VibeOS timer architecture. Pulse will move to APIC-based routing, acknowledge interrupts, collect ticks, and return from a normalised trap frame before it allows preemptive scheduling.

Pulse now also has a pure timer-source selection policy. It keeps PIT as the active source and may mark an APIC timer handoff as eligible only when the separately validated APIC metadata plan is ready. This is not local-APIC timer programming, vector configuration, interrupt rerouting, or a change to the PIT/PIC probe.

The accompanying bounded capability inventory reports the active legacy PIT availability and a separate APIC-timer metadata eligibility flag. It is an observation record only: it does not calibrate a timer, access APIC timer registers, install an APIC vector, or alter scheduling policy.

## Reference

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
