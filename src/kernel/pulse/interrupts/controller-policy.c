/* VibeOS Pulse — metadata-only controller policy; it performs no I/O. */

#include "interrupt-controller.h"

int pulse_interrupt_controller_select(
    const DAWN_ACPI_MADT_X86_INVENTORY *madt,
    PULSE_INTERRUPT_CONTROLLER_SELECTION *selection) {
    if (madt == (const void *)0 || selection == (void *)0) {
        return 0;
    }
    selection->active_controller = PULSE_INTERRUPT_CONTROLLER_PIC;
    selection->apic_handoff_eligible = madt->local_apic_count != 0U && madt->io_apic_count != 0U;
    return 1;
}
