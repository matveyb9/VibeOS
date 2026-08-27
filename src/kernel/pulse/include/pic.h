/* VibeOS Pulse — early legacy PIC ownership boundary for x86_64. */

#ifndef VIBEOS_PULSE_PIC_H
#define VIBEOS_PULSE_PIC_H

#include <stdint.h>

void pulse_pic_remap_and_set_mask(uint8_t master_mask, uint8_t slave_mask);
void pulse_pic_end_of_interrupt(uint8_t irq);

#endif
