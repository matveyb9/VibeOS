/* Pulse controller policy test — metadata-only deterministic selection. */

#include <stdio.h>

#include "interrupt-controller.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    DAWN_ACPI_MADT_X86_INVENTORY madt = {0};
    PULSE_INTERRUPT_CONTROLLER_SELECTION selection;

    if (!expect(!pulse_interrupt_controller_select((const void *)0, &selection), "null MADT is rejected") ||
        !expect(!pulse_interrupt_controller_select(&madt, (void *)0), "null selection is rejected") ||
        !expect(pulse_interrupt_controller_select(&madt, &selection) &&
                    selection.active_controller == PULSE_INTERRUPT_CONTROLLER_PIC &&
                    selection.apic_handoff_eligible == 0U,
                    "PIC remains active without complete APIC metadata")) {
        return 1;
    }
    madt.local_apic_count = 1U;
    madt.io_apic_count = 1U;
    if (!expect(pulse_interrupt_controller_select(&madt, &selection) &&
                    selection.active_controller == PULSE_INTERRUPT_CONTROLLER_PIC &&
                    selection.apic_handoff_eligible == 1U,
                    "complete metadata marks only a future APIC handoff as eligible")) {
        return 1;
    }
    puts("Pulse interrupt-controller policy tests passed.");
    return 0;
}
