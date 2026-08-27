/*
 * VibeOS Pulse — first freestanding x86_64 kernel entry.
 *
 * Pulse validates Dawn Context before it trusts firmware-supplied memory data.
 * Paging, physical allocation, interrupts, and tasking are deliberately later
 * responsibilities; this step proves the native Prelude-to-Pulse boundary.
 */

#include "memory.h"
#include "paging.h"
#include "interrupts.h"
#include "scheduler.h"
#include "context.h"

static void pulse_debug_putc(char character) {
    __asm__ volatile("outb %0, %w1" : : "a"(character), "d"((uint16_t)0x402));
}

static void pulse_debug_write(const char *message) {
    while (*message != '\0') {
        pulse_debug_putc(*message);
        ++message;
    }
}

static void pulse_debug_exit(void) {
    __asm__ volatile("outl %0, %w1" : : "a"(0), "d"((uint16_t)0x0f4));
}

static int pulse_context_is_valid(const DAWN_CONTEXT *context) {
    return context != (void *)0 && context->magic == DAWN_CONTEXT_MAGIC &&
           context->version == DAWN_CONTEXT_VERSION && context->size >= sizeof(DAWN_CONTEXT) &&
           context->memory_map_physical_address != 0 && context->memory_map_size != 0 &&
           context->memory_descriptor_size >= 40U && context->kernel_stack_top != 0 &&
           context->kernel_stack_size >= 4096U;
}

__attribute__((noreturn)) void pulse_entry(const DAWN_CONTEXT *context) {
    uint64_t first_frame;
    uint64_t second_frame;
    uint32_t first_task;
    uint32_t second_task;
    uint32_t selected_task;

    if (pulse_context_is_valid(context) && pulse_memory_initialize(context) &&
        pulse_memory_take_frame(&first_frame) && pulse_memory_take_frame(&second_frame) &&
        second_frame == first_frame + 4096U && pulse_paging_initialize() &&
        pulse_interrupts_initialize()) {
        pulse_scheduler_initialize();
        if (!pulse_scheduler_create_ready_task(&first_task) ||
            !pulse_scheduler_create_ready_task(&second_task) ||
            !pulse_scheduler_select_next(&selected_task) || selected_task != first_task ||
            !pulse_scheduler_select_next(&selected_task) || selected_task != second_task) {
            pulse_debug_write("PULSE: early scheduler bootstrap failed\n");
            pulse_debug_exit();
        }
#if defined(PULSE_PROBE_panic)
        __asm__ volatile("ud2");
        pulse_debug_write("PULSE: invalid opcode unexpectedly returned\n");
#else
        if (!pulse_context_run_probe()) {
            pulse_debug_write("PULSE: task context bootstrap failed\n");
            pulse_debug_exit();
        }
        pulse_interrupts_trigger_breakpoint();
        pulse_debug_write("PULSE: breakpoint dispatch unexpectedly returned\n");
#endif
    } else {
        pulse_debug_write("PULSE: early interrupt bootstrap failed\n");
    }

    pulse_debug_exit();

    for (;;) {
        __asm__ volatile("hlt");
    }
}
