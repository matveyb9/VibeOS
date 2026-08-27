/*
 * VibeOS Pulse — legacy PIC/PIT timer probe for the first external IRQ path.
 *
 * This is a transitional x86_64 implementation. It exposes only IRQ0 at
 * vector 32 and is replaced by local-APIC timing in a later Pulse stage.
 */

#include "timer.h"
#include "pic.h"

#define PULSE_PIT_INPUT_HZ UINT32_C(1193182)
#define PULSE_TIMER_PROBE_HZ UINT32_C(100)

static void pulse_out8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %w1" : : "a"(value), "d"(port));
}

uint16_t pulse_timer_divisor_for_hz(uint32_t frequency_hz) {
    uint32_t divisor;

    if (frequency_hz == 0U || frequency_hz > PULSE_PIT_INPUT_HZ) {
        return 0U;
    }

    divisor = PULSE_PIT_INPUT_HZ / frequency_hz;
    if (divisor == 0U || divisor > UINT16_MAX) {
        return 0U;
    }
    return (uint16_t)divisor;
}

int pulse_timer_source_select(
    const PULSE_INTERRUPT_CONTROLLER_SELECTION *controller,
    const PULSE_X86_APIC_HANDOFF_PLAN *apic_plan,
    PULSE_TIMER_SOURCE_SELECTION *selection) {
    if (controller == (const void *)0 || apic_plan == (const void *)0 || selection == (void *)0) {
        return 0;
    }
    selection->active_source = PULSE_TIMER_SOURCE_PIT;
    selection->apic_timer_handoff_eligible =
        controller->apic_handoff_eligible != 0U && pulse_x86_apic_handoff_plan_is_ready(apic_plan);
    return 1;
}

int pulse_timer_capabilities_describe(
    const PULSE_TIMER_SOURCE_SELECTION *selection,
    PULSE_TIMER_CAPABILITIES *capabilities) {
    if (selection == (const void *)0 || capabilities == (void *)0 ||
        selection->active_source != PULSE_TIMER_SOURCE_PIT) {
        return 0;
    }
    capabilities->pit_legacy_available = 1U;
    capabilities->apic_timer_metadata_eligible = selection->apic_timer_handoff_eligible != 0U;
    return 1;
}

void pulse_timer_start_probe(void) {
    uint16_t divisor = pulse_timer_divisor_for_hz(PULSE_TIMER_PROBE_HZ);

    if (divisor == 0U) {
        return;
    }

    pulse_pic_remap_and_set_mask(UINT8_C(0xfe), UINT8_C(0xff));
    pulse_out8(UINT16_C(0x43), UINT8_C(0x34));
    pulse_out8(UINT16_C(0x40), (uint8_t)(divisor & UINT16_C(0xff)));
    pulse_out8(UINT16_C(0x40), (uint8_t)(divisor >> 8U));
    __asm__ volatile("sti" : : : "memory");
}
