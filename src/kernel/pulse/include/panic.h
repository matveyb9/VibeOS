/* VibeOS Pulse — terminal diagnostics for unrecoverable early failures. */

#ifndef VIBEOS_PULSE_PANIC_H
#define VIBEOS_PULSE_PANIC_H

#include <stdint.h>

__attribute__((noreturn)) void pulse_panic_unhandled_interrupt(uint32_t vector);

#endif
