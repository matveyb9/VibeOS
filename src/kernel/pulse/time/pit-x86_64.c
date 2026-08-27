/*
 * VibeOS Pulse — legacy PIC/PIT timer probe for the first external IRQ path.
 *
 * This is a transitional x86_64 implementation. It exposes only IRQ0 at
 * vector 32 and is replaced by local-APIC timing in a later Pulse stage.
 */

#include "timer.h"

#define PULSE_PIT_INPUT_HZ UINT32_C(1193182)
#define PULSE_TIMER_PROBE_HZ UINT32_C(100)

static void pulse_out8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %w1" : : "a"(value), "d"(port));
}

static void pulse_io_wait(void) {
    pulse_out8(UINT16_C(0x80), 0U);
}

uint16_t pulse_timer_divisor_for_hz(uint32_t frequency_hz) {
    uint32_t divisor;

    if (frequency_hz == 0U || frequency_hz > PULSE_PIT_INPUT_HZ) {
        return 0U;
    }

    divisor = PULSE_PIT_INPUT_HZ / frequency_hz;
    if (divisor == 0U || divisor > UINT16_MAX) {
        return 0U;
    }
    return (uint16_t)divisor;
}

static void pulse_pic_remap_and_unmask_timer(void) {
    pulse_out8(UINT16_C(0x20), UINT8_C(0x11));
    pulse_io_wait();
    pulse_out8(UINT16_C(0xa0), UINT8_C(0x11));
    pulse_io_wait();
    pulse_out8(UINT16_C(0x21), UINT8_C(0x20));
    pulse_io_wait();
    pulse_out8(UINT16_C(0xa1), UINT8_C(0x28));
    pulse_io_wait();
    pulse_out8(UINT16_C(0x21), UINT8_C(0x04));
    pulse_io_wait();
    pulse_out8(UINT16_C(0xa1), UINT8_C(0x02));
    pulse_io_wait();
    pulse_out8(UINT16_C(0x21), UINT8_C(0x01));
    pulse_io_wait();
    pulse_out8(UINT16_C(0xa1), UINT8_C(0x01));
    pulse_io_wait();
    pulse_out8(UINT16_C(0x21), UINT8_C(0xfe));
    pulse_out8(UINT16_C(0xa1), UINT8_C(0xff));
}

void pulse_timer_start_probe(void) {
    uint16_t divisor = pulse_timer_divisor_for_hz(PULSE_TIMER_PROBE_HZ);

    if (divisor == 0U) {
        return;
    }

    pulse_pic_remap_and_unmask_timer();
    pulse_out8(UINT16_C(0x43), UINT8_C(0x34));
    pulse_out8(UINT16_C(0x40), (uint8_t)(divisor & UINT16_C(0xff)));
    pulse_out8(UINT16_C(0x40), (uint8_t)(divisor >> 8U));
    __asm__ volatile("sti" : : : "memory");
}
