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

int pulse_interrupt_controller_select(
    const DAWN_ACPI_MADT_X86_INVENTORY *madt,
    PULSE_INTERRUPT_CONTROLLER_SELECTION *selection);

#endif
