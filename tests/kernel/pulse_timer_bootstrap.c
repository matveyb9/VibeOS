/* VibeOS Pulse — host checks for deterministic PIT timer divisor calculation. */

#include <stdint.h>
#include <stdio.h>

#include "timer.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    PULSE_INTERRUPT_CONTROLLER_SELECTION controller = {PULSE_INTERRUPT_CONTROLLER_PIC, 0U};
    PULSE_X86_APIC_HANDOFF_PLAN apic_plan = {1U, UINT32_C(0xfee00000), 2U, UINT32_C(0xfec00000), 32U};
    PULSE_TIMER_SOURCE_SELECTION selection;
    PULSE_TIMER_CAPABILITIES capabilities;

    if (!expect(pulse_timer_divisor_for_hz(0U) == 0U, "zero frequency is rejected") ||
        !expect(pulse_timer_divisor_for_hz(UINT32_C(1193183)) == 0U, "frequency above PIT input is rejected") ||
        !expect(pulse_timer_divisor_for_hz(100U) == UINT16_C(11931), "100 Hz divisor is deterministic") ||
        !expect(pulse_timer_divisor_for_hz(UINT32_C(1193182)) == 1U, "PIT input frequency yields divisor one") ||
        !expect(!pulse_timer_source_select((const void *)0, &apic_plan, &selection), "null controller is rejected") ||
        !expect(pulse_timer_source_select(&controller, &apic_plan, &selection) &&
                    selection.active_source == PULSE_TIMER_SOURCE_PIT && selection.apic_timer_handoff_eligible == 0U,
                    "PIT remains active without APIC eligibility")) {
        return 1;
    }
    controller.apic_handoff_eligible = 1U;
    if (!expect(pulse_timer_source_select(&controller, &apic_plan, &selection) &&
                    selection.active_source == PULSE_TIMER_SOURCE_PIT && selection.apic_timer_handoff_eligible == 1U,
                    "ready APIC plan is only eligible while PIT stays active") ||
        !expect(pulse_timer_capabilities_describe(&selection, &capabilities) &&
                    capabilities.pit_legacy_available == 1U && capabilities.apic_timer_metadata_eligible == 1U,
                    "timer capability inventory retains active and future options")) {
        return 1;
    }

    puts("Pulse timer bootstrap unit tests passed.");
    return 0;
}
