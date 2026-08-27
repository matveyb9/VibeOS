/* VibeOS Pulse — transitional 8259 PIC setup for early external IRQ paths. */

#include "pic.h"

#define PULSE_PIC_MASTER_COMMAND UINT16_C(0x20)
#define PULSE_PIC_MASTER_DATA UINT16_C(0x21)
#define PULSE_PIC_SLAVE_COMMAND UINT16_C(0xa0)
#define PULSE_PIC_SLAVE_DATA UINT16_C(0xa1)
#define PULSE_PIC_INITIALIZE UINT8_C(0x11)
#define PULSE_PIC_8086_MODE UINT8_C(0x01)
#define PULSE_PIC_END_OF_INTERRUPT UINT8_C(0x20)

static void pulse_pic_out8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %w1" : : "a"(value), "d"(port));
}

static void pulse_pic_io_wait(void) {
    pulse_pic_out8(UINT16_C(0x80), 0U);
}

void pulse_pic_remap_and_set_mask(uint8_t master_mask, uint8_t slave_mask) {
    pulse_pic_out8(PULSE_PIC_MASTER_COMMAND, PULSE_PIC_INITIALIZE);
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_SLAVE_COMMAND, PULSE_PIC_INITIALIZE);
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_MASTER_DATA, UINT8_C(0x20));
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_SLAVE_DATA, UINT8_C(0x28));
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_MASTER_DATA, UINT8_C(0x04));
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_SLAVE_DATA, UINT8_C(0x02));
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_MASTER_DATA, PULSE_PIC_8086_MODE);
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_SLAVE_DATA, PULSE_PIC_8086_MODE);
    pulse_pic_io_wait();
    pulse_pic_out8(PULSE_PIC_MASTER_DATA, master_mask);
    pulse_pic_out8(PULSE_PIC_SLAVE_DATA, slave_mask);
}

void pulse_pic_end_of_interrupt(uint8_t irq) {
    if (irq >= UINT8_C(8)) {
        pulse_pic_out8(PULSE_PIC_SLAVE_COMMAND, PULSE_PIC_END_OF_INTERRUPT);
    }
    pulse_pic_out8(PULSE_PIC_MASTER_COMMAND, PULSE_PIC_END_OF_INTERRUPT);
}
