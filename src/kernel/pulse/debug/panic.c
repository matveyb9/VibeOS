/*
 * VibeOS Pulse — terminal early panic path.
 *
 * The current bootstrap cannot safely recover from an unexpected trap. It
 * therefore emits a deterministic QEMU diagnostic, requests debug-exit, and
 * halts with interrupts disabled. Rich crash records and recovery UI follow.
 */

#include "panic.h"

static void pulse_panic_putc(char character) {
    __asm__ volatile("outb %0, %w1" : : "a"(character), "d"((uint16_t)0x402));
}

static void pulse_panic_write(const char *message) {
    while (*message != '\0') {
        pulse_panic_putc(*message);
        ++message;
    }
}

__attribute__((noreturn)) void pulse_panic_unhandled_interrupt(uint32_t vector) {
    (void)vector;
    pulse_panic_write("PULSE PANIC: unhandled interrupt\n");
    __asm__ volatile("outl %0, %w1" : : "a"(0), "d"((uint16_t)0x0f4));

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
