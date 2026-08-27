/*
 * VibeOS Pulse — first freestanding x86_64 kernel entry.
 *
 * Pulse validates Dawn Context before it trusts firmware-supplied memory data.
 * Paging, physical allocation, interrupts, and tasking are deliberately later
 * responsibilities; this step proves the native Prelude-to-Pulse boundary.
 */

#include <dawn.h>

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
           context->memory_descriptor_size >= 40U;
}

__attribute__((noreturn)) void pulse_entry(const DAWN_CONTEXT *context) {
    if (pulse_context_is_valid(context)) {
        pulse_debug_write("PULSE: Dawn Context v1 accepted\n");
    } else {
        pulse_debug_write("PULSE: Dawn Context rejected\n");
    }

    pulse_debug_exit();

    for (;;) {
        __asm__ volatile("hlt");
    }
}
