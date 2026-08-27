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

int pulse_x86_apic_handoff_plan_build(
    const DAWN_ACPI_MADT_INVENTORY *madt,
    const DAWN_ACPI_MADT_X86_INVENTORY *x86_madt,
    PULSE_X86_APIC_HANDOFF_PLAN *plan) {
    uint32_t local_index;
    uint32_t io_index;

    if (madt == (const void *)0 || x86_madt == (const void *)0 || plan == (void *)0 ||
        madt->local_interrupt_controller_address == 0U || x86_madt->local_apic_count == 0U ||
        x86_madt->io_apic_count == 0U || x86_madt->local_apic_count > DAWN_ACPI_MADT_X86_RECORD_CAPACITY ||
        x86_madt->io_apic_count > DAWN_ACPI_MADT_X86_RECORD_CAPACITY) {
        return 0;
    }
    for (local_index = 0U; local_index < x86_madt->local_apic_count; ++local_index) {
        if ((x86_madt->local_apics[local_index].flags & 1U) != 0U) {
            break;
        }
    }
    if (local_index == x86_madt->local_apic_count) {
        return 0;
    }
    for (io_index = 0U; io_index < x86_madt->io_apic_count; ++io_index) {
        if (x86_madt->io_apics[io_index].io_apic_physical_address != 0U) {
            break;
        }
    }
    if (io_index == x86_madt->io_apic_count) {
        return 0;
    }
    plan->local_apic_id = x86_madt->local_apics[local_index].local_apic_id;
    plan->local_interrupt_controller_address = madt->local_interrupt_controller_address;
    plan->io_apic_id = x86_madt->io_apics[io_index].io_apic_id;
    plan->io_apic_physical_address = x86_madt->io_apics[io_index].io_apic_physical_address;
    plan->global_system_interrupt_base = x86_madt->io_apics[io_index].global_system_interrupt_base;
    return 1;
}
