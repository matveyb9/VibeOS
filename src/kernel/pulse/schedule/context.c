/*
 * VibeOS Pulse — first cooperative context-switch proof.
 *
 * The stage demonstrates an actual independently stacked kernel execution
 * context. It remains cooperative: interrupts do not schedule it yet.
 */

#include "context.h"

#define PULSE_CONTEXT_MIN_STACK_BYTES UINT64_C(16)
#define PULSE_CONTEXT_PROBE_STACK_BYTES UINT64_C(4096)

static PULSE_X86_CONTEXT probe_boot_context;
static PULSE_X86_CONTEXT probe_worker_context;
static uint8_t probe_worker_stack[PULSE_CONTEXT_PROBE_STACK_BYTES];
static volatile uint32_t probe_worker_completed;

static void pulse_context_probe_putc(char character) {
    __asm__ volatile("outb %0, %w1" : : "a"(character), "d"((uint16_t)0x402));
}

static void pulse_context_probe_write(const char *message) {
    while (*message != '\0') {
        pulse_context_probe_putc(*message);
        ++message;
    }
}

__attribute__((noreturn)) static void pulse_context_probe_worker(void) {
    probe_worker_completed = 1U;
    pulse_context_probe_write("PULSE: task context verified\n");
    pulse_context_switch(&probe_worker_context, &probe_boot_context);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

int pulse_context_prepare(
    PULSE_X86_CONTEXT *context,
    void *stack_base,
    uint64_t stack_size,
    PULSE_CONTEXT_ENTRY entry) {
    uintptr_t stack_top;

    if (context == (void *)0 || stack_base == (void *)0 || entry == (void *)0 ||
        stack_size < PULSE_CONTEXT_MIN_STACK_BYTES) {
        return 0;
    }

    context->rbx = 0;
    context->rbp = 0;
    context->r12 = 0;
    context->r13 = 0;
    context->r14 = 0;
    context->r15 = 0;
    stack_top = (uintptr_t)stack_base + (uintptr_t)stack_size;
    stack_top &= ~(uintptr_t)0x0fU;
    context->stack_pointer = (uint64_t)(stack_top - (uintptr_t)8U);
    context->instruction_pointer = (uint64_t)(uintptr_t)entry;
    return 1;
}

int pulse_context_run_probe(void) {
    probe_worker_completed = 0U;
    if (!pulse_context_prepare(
            &probe_worker_context,
            probe_worker_stack,
            sizeof(probe_worker_stack),
            pulse_context_probe_worker)) {
        return 0;
    }

    pulse_context_switch(&probe_boot_context, &probe_worker_context);
    return probe_worker_completed == 1U;
}
