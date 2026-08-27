/* VibeOS Pulse — host checks for x86_64 IDT gate construction. */

#include <stdint.h>
#include <stdio.h>

#include "interrupts.h"

void pulse_x86_default_interrupt(void) {}
void pulse_x86_breakpoint_interrupt(void) {}

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

static uintptr_t gate_target(const PULSE_X86_IDT_GATE *gate) {
    return (uintptr_t)gate->offset_low | ((uintptr_t)gate->offset_middle << 16U) |
           ((uintptr_t)gate->offset_high << 32U);
}

int main(void) {
    PULSE_X86_IDT_GATE table[PULSE_X86_IDT_ENTRIES];
    const uintptr_t default_handler = (uintptr_t)UINT64_C(0x0000000000201100);
    const uintptr_t breakpoint_handler = (uintptr_t)UINT64_C(0x0000000000201200);

    if (!expect(!pulse_interrupts_build_table(table, 0U, default_handler, breakpoint_handler),
                    "zero code selector is rejected") ||
        !expect(pulse_interrupts_build_table(table, UINT16_C(0x08), default_handler, breakpoint_handler),
                    "valid IDT table initializes")) {
        return 1;
    }

    if (!expect(gate_target(&table[0]) == default_handler, "default vector target is preserved") ||
        !expect(gate_target(&table[PULSE_X86_BREAKPOINT_VECTOR]) == breakpoint_handler,
                    "breakpoint vector target is specialized") ||
        !expect(table[0].selector == UINT16_C(0x08), "code selector is stored") ||
        !expect(table[PULSE_X86_BREAKPOINT_VECTOR].attributes == UINT8_C(0x8e),
                    "present interrupt-gate attributes are stored") ||
        !expect(table[PULSE_X86_BREAKPOINT_VECTOR].reserved == 0U, "reserved gate bits remain clear")) {
        return 1;
    }

    puts("Pulse interrupts bootstrap unit tests passed.");
    return 0;
}
