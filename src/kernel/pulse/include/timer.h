/* VibeOS Pulse — legacy x86_64 PIC/PIT timer bootstrap interface. */

#ifndef VIBEOS_PULSE_TIMER_H
#define VIBEOS_PULSE_TIMER_H

#include <stdint.h>

#include "interrupt-controller.h"

#define PULSE_X86_TIMER_VECTOR UINT8_C(32)

typedef enum {
    PULSE_TIMER_SOURCE_PIT = 1
} PULSE_TIMER_SOURCE;

typedef struct {
    uint32_t active_source;
    uint32_t apic_timer_handoff_eligible;
} PULSE_TIMER_SOURCE_SELECTION;

uint16_t pulse_timer_divisor_for_hz(uint32_t frequency_hz);
int pulse_timer_source_select(
    const PULSE_INTERRUPT_CONTROLLER_SELECTION *controller,
    const PULSE_X86_APIC_HANDOFF_PLAN *apic_plan,
    PULSE_TIMER_SOURCE_SELECTION *selection);
void pulse_timer_start_probe(void);

#endif
