/* VibeOS Pulse — legacy x86_64 PIC/PIT timer bootstrap interface. */

#ifndef VIBEOS_PULSE_TIMER_H
#define VIBEOS_PULSE_TIMER_H

#include <stdint.h>

#define PULSE_X86_TIMER_VECTOR UINT8_C(32)

uint16_t pulse_timer_divisor_for_hz(uint32_t frequency_hz);
void pulse_timer_start_probe(void);

#endif
