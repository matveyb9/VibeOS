<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_INTERRUPTS_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse x86_64 interrupt bootstrap

**Status:** Implemented and verified in the first x86_64 QEMU UEFI profile.

Pulse establishes a complete 256-entry Interrupt Descriptor Table immediately after its own early paging becomes active. Every vector initially targets a no-return diagnostic handler, while vector `3` uses a dedicated breakpoint handler. The code selector is read from the active CPU context rather than hard-coded. Intel's system programming guide covers the IDT, gate descriptors, and interrupt/exception delivery in IA-32e mode.[1]

## Verification boundary

Maskable interrupts remain disabled. Pulse deliberately executes `INT3` after loading `IDTR`; the breakpoint handler writes `PULSE: breakpoint trap handled` to QEMU's test channel and terminates the emulator. This proves the active IDT, gate offset, selector, and exception control transfer without prematurely enabling device interrupts.

Pulse also has a separate pure controller-selection policy. It consumes only the already-validated bounded MADT metadata: PIC remains the active bootstrap controller, while the presence of at least one Local APIC and one I/O APIC record marks only a future APIC handoff as eligible. When there is an enabled Local APIC record, a nonzero MADT local-controller address, and a nonzero I/O APIC address, it may form an immutable handoff plan containing those validated identifiers and the first I/O APIC GSI base. The policy performs no port I/O, does not change PIC masks, and never maps or programs an APIC.

## Current limits

The bootstrap has no register-frame capture, error-code normalization, return path, IST stack, PIC/APIC routing, timer tick, or recovery UI. Its default handler is intentionally terminal. APIC eligibility is not APIC enablement, interrupt rerouting, processor startup, or SMP support. These capabilities will be added before Pulse enables external interrupts or schedules work.

## Reference

[1] [Intel 64 and IA-32 Architectures Software Developer's Manual, Volume 3](https://cdrdv2-public.intel.com/774493/325384-sdm-vol-3abcd.pdf)
