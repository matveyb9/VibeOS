/* VibeOS Pulse — pure interrupt-controller selection policy. */

#ifndef VIBEOS_PULSE_INTERRUPT_CONTROLLER_H
#define VIBEOS_PULSE_INTERRUPT_CONTROLLER_H

#include <dawn_acpi.h>

typedef enum {
    PULSE_INTERRUPT_CONTROLLER_PIC = 1
} PULSE_INTERRUPT_CONTROLLER;

typedef struct {
    uint32_t active_controller;
    uint32_t apic_handoff_eligible;
} PULSE_INTERRUPT_CONTROLLER_SELECTION;

typedef struct {
    uint8_t local_apic_id;
    uint32_t local_interrupt_controller_address;
    uint8_t io_apic_id;
    uint32_t io_apic_physical_address;
    uint32_t global_system_interrupt_base;
} PULSE_X86_APIC_HANDOFF_PLAN;

int pulse_interrupt_controller_select(
    const DAWN_ACPI_MADT_X86_INVENTORY *madt,
    PULSE_INTERRUPT_CONTROLLER_SELECTION *selection);
int pulse_x86_apic_handoff_plan_build(
    const DAWN_ACPI_MADT_INVENTORY *madt,
    const DAWN_ACPI_MADT_X86_INVENTORY *x86_madt,
    PULSE_X86_APIC_HANDOFF_PLAN *plan);
int pulse_x86_apic_handoff_plan_is_ready(const PULSE_X86_APIC_HANDOFF_PLAN *plan);

#endif
