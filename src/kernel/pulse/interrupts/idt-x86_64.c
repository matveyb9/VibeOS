/*
 * VibeOS Pulse — first x86_64 interrupt descriptor table.
 *
 * Interrupt delivery stays disabled. The bootstrap loads a complete IDT, then
 * intentionally invokes vector 3 so QEMU can prove that Pulse owns the fault
 * boundary before PIC/APIC, timer, and external interrupt work begins.
 */

#include "interrupts.h"

#define PULSE_X86_IDT_PRESENT_INTERRUPT_GATE UINT8_C(0x8e)

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uintptr_t base;
} PULSE_X86_IDTR;

extern void pulse_x86_default_interrupt(void);
extern void pulse_x86_breakpoint_interrupt(void);
extern void pulse_x86_timer_interrupt(void);

static PULSE_X86_IDT_GATE early_idt[PULSE_X86_IDT_ENTRIES];

static uint16_t pulse_read_code_selector(void) {
    uint16_t selector;
    __asm__ volatile("mov %%cs, %0" : "=r"(selector));
    return selector;
}

static void pulse_load_idt(const PULSE_X86_IDTR *idtr) {
    __asm__ volatile("lidt %0" : : "m"(*idtr) : "memory");
}

static void pulse_set_gate(PULSE_X86_IDT_GATE *gate, uint16_t selector, uintptr_t handler) {
    gate->offset_low = (uint16_t)(handler & (uintptr_t)0xffffU);
    gate->selector = selector;
    gate->ist = 0;
    gate->attributes = PULSE_X86_IDT_PRESENT_INTERRUPT_GATE;
    gate->offset_middle = (uint16_t)((handler >> 16U) & (uintptr_t)0xffffU);
    gate->offset_high = (uint32_t)(handler >> 32U);
    gate->reserved = 0;
}

int pulse_interrupts_build_table(
    PULSE_X86_IDT_GATE *table,
    uint16_t code_selector,
    uintptr_t default_handler,
    uintptr_t breakpoint_handler) {
    uint16_t vector;

    if (table == (void *)0 || code_selector == 0U || default_handler == 0U || breakpoint_handler == 0U) {
        return 0;
    }

    for (vector = 0; vector < PULSE_X86_IDT_ENTRIES; ++vector) {
        pulse_set_gate(&table[vector], code_selector, default_handler);
    }
    pulse_set_gate(&table[PULSE_X86_BREAKPOINT_VECTOR], code_selector, breakpoint_handler);
    pulse_set_gate(&table[PULSE_X86_TIMER_VECTOR], code_selector, (uintptr_t)pulse_x86_timer_interrupt);
    return 1;
}

int pulse_interrupts_initialize(void) {
    PULSE_X86_IDTR idtr;

    if (!pulse_interrupts_build_table(
            early_idt,
            pulse_read_code_selector(),
            (uintptr_t)pulse_x86_default_interrupt,
            (uintptr_t)pulse_x86_breakpoint_interrupt)) {
        return 0;
    }

    idtr.limit = (uint16_t)(sizeof(early_idt) - 1U);
    idtr.base = (uintptr_t)early_idt;
    pulse_load_idt(&idtr);
    return 1;
}

void pulse_interrupts_trigger_breakpoint(void) {
    __asm__ volatile("int3");
}
