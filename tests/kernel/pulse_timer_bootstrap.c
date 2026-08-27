/* VibeOS Pulse — host checks for deterministic PIT timer divisor calculation. */

#include <stdint.h>
#include <stdio.h>

#include "timer.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    if (!expect(pulse_timer_divisor_for_hz(0U) == 0U, "zero frequency is rejected") ||
        !expect(pulse_timer_divisor_for_hz(UINT32_C(1193183)) == 0U, "frequency above PIT input is rejected") ||
        !expect(pulse_timer_divisor_for_hz(100U) == UINT16_C(11931), "100 Hz divisor is deterministic") ||
        !expect(pulse_timer_divisor_for_hz(UINT32_C(1193182)) == 1U, "PIT input frequency yields divisor one")) {
        return 1;
    }

    puts("Pulse timer bootstrap unit tests passed.");
    return 0;
}
