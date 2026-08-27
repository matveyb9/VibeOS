/* VibeOS Pulse — x86_64 IDT bootstrap interface. */

#ifndef VIBEOS_PULSE_INTERRUPTS_H
#define VIBEOS_PULSE_INTERRUPTS_H

#include <stdint.h>

#define PULSE_X86_IDT_ENTRIES UINT16_C(256)
#define PULSE_X86_BREAKPOINT_VECTOR UINT8_C(3)
#define PULSE_X86_TIMER_VECTOR UINT8_C(32)

typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} PULSE_X86_IDT_GATE;

int pulse_interrupts_build_table(
    PULSE_X86_IDT_GATE *table,
    uint16_t code_selector,
    uintptr_t default_handler,
    uintptr_t breakpoint_handler);
int pulse_interrupts_initialize(void);
void pulse_interrupts_trigger_breakpoint(void);

#endif
