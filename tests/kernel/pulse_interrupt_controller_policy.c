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
    DAWN_ACPI_MADT_INVENTORY madt_header = {0};
    PULSE_INTERRUPT_CONTROLLER_SELECTION selection;
    PULSE_X86_APIC_HANDOFF_PLAN plan;

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
    madt_header.local_interrupt_controller_address = UINT32_C(0xfee00000);
    madt.local_apics[0].local_apic_id = 1U;
    madt.local_apics[0].flags = 1U;
    madt.io_apics[0].io_apic_id = 2U;
    madt.io_apics[0].io_apic_physical_address = UINT32_C(0xfec00000);
    madt.io_apics[0].global_system_interrupt_base = 32U;
    if (!expect(pulse_interrupt_controller_select(&madt, &selection) &&
                    selection.active_controller == PULSE_INTERRUPT_CONTROLLER_PIC &&
                    selection.apic_handoff_eligible == 1U,
                    "complete metadata marks only a future APIC handoff as eligible") ||
        !expect(pulse_x86_apic_handoff_plan_build(&madt_header, &madt, &plan) &&
                    plan.local_apic_id == 1U && plan.local_interrupt_controller_address == UINT32_C(0xfee00000) &&
                    plan.io_apic_id == 2U && plan.io_apic_physical_address == UINT32_C(0xfec00000) &&
                    plan.global_system_interrupt_base == 32U,
                    "valid metadata forms immutable handoff plan") ||
        !expect(pulse_x86_apic_handoff_plan_is_ready(&plan), "aligned immutable plan is ready") ||
        !expect(!pulse_x86_apic_handoff_plan_is_ready((const void *)0), "null handoff plan is rejected")) {
        return 1;
    }
    plan.io_apic_physical_address = UINT32_C(0xfec00001);
    if (!expect(!pulse_x86_apic_handoff_plan_is_ready(&plan), "unaligned I/O APIC address is rejected")) {
        return 1;
    }
    madt.local_apics[0].flags = 0U;
    if (!expect(!pulse_x86_apic_handoff_plan_build(&madt_header, &madt, &plan),
                    "no enabled Local APIC rejects handoff plan")) {
        return 1;
    }
    madt.local_apics[0].flags = 1U;
    madt.io_apics[0].io_apic_physical_address = 0U;
    if (!expect(!pulse_x86_apic_handoff_plan_build(&madt_header, &madt, &plan),
                    "zero I/O APIC address rejects handoff plan")) {
        return 1;
    }
    puts("Pulse interrupt-controller policy tests passed.");
    return 0;
}
