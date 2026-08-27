/* VibeOS Atlas — constrained i8042 adapter for the reproducible QEMU IRQ1 path. */

#include "atlas_input.h"
#include "pic.h"

#define ATLAS_I8042_DATA_PORT UINT16_C(0x60)
#define ATLAS_I8042_STATUS_PORT UINT16_C(0x64)
#define ATLAS_I8042_COMMAND_PORT UINT16_C(0x64)
#define ATLAS_I8042_STATUS_OUTPUT_FULL UINT8_C(0x01)
#define ATLAS_I8042_STATUS_INPUT_FULL UINT8_C(0x02)
#define ATLAS_I8042_STATUS_AUXILIARY UINT8_C(0x20)
#define ATLAS_I8042_COMMAND_READ_CONFIGURATION UINT8_C(0x20)
#define ATLAS_I8042_COMMAND_WRITE_CONFIGURATION UINT8_C(0x60)
#define ATLAS_I8042_COMMAND_ENABLE_FIRST_PORT UINT8_C(0xae)
#define ATLAS_I8042_CONFIGURATION_IRQ1 UINT8_C(0x01)
#define ATLAS_I8042_CONFIGURATION_FIRST_PORT_DISABLED UINT8_C(0x10)
#define ATLAS_I8042_CONFIGURATION_TRANSLATION UINT8_C(0x40)
#define ATLAS_I8042_SPIN_LIMIT UINT32_C(1000000)

static uint8_t atlas_i8042_in8(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %w1, %0" : "=a"(value) : "d"(port));
    return value;
}

static void atlas_i8042_out8(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %w1" : : "a"(value), "d"(port));
}

static int atlas_i8042_wait_input_empty(void) {
    uint32_t spin;

    for (spin = 0; spin < ATLAS_I8042_SPIN_LIMIT; ++spin) {
        if ((atlas_i8042_in8(ATLAS_I8042_STATUS_PORT) & ATLAS_I8042_STATUS_INPUT_FULL) == 0U) {
            return 1;
        }
    }
    return 0;
}

static int atlas_i8042_wait_output_full(void) {
    uint32_t spin;

    for (spin = 0; spin < ATLAS_I8042_SPIN_LIMIT; ++spin) {
        if ((atlas_i8042_in8(ATLAS_I8042_STATUS_PORT) & ATLAS_I8042_STATUS_OUTPUT_FULL) != 0U) {
            return 1;
        }
    }
    return 0;
}

static void atlas_i8042_flush_output(void) {
    uint32_t spin;

    for (spin = 0; spin < ATLAS_I8042_SPIN_LIMIT; ++spin) {
        if ((atlas_i8042_in8(ATLAS_I8042_STATUS_PORT) & ATLAS_I8042_STATUS_OUTPUT_FULL) == 0U) {
            return;
        }
        (void)atlas_i8042_in8(ATLAS_I8042_DATA_PORT);
    }
}

int atlas_i8042_keyboard_prepare_irq1(void) {
    uint8_t configuration;

    atlas_keyboard_initialize();
    atlas_i8042_flush_output();
    if (!atlas_i8042_wait_input_empty()) {
        return 0;
    }
    atlas_i8042_out8(ATLAS_I8042_COMMAND_PORT, ATLAS_I8042_COMMAND_READ_CONFIGURATION);
    if (!atlas_i8042_wait_output_full()) {
        return 0;
    }
    configuration = atlas_i8042_in8(ATLAS_I8042_DATA_PORT);
    configuration |= ATLAS_I8042_CONFIGURATION_IRQ1 | ATLAS_I8042_CONFIGURATION_TRANSLATION;
    configuration &= (uint8_t)~ATLAS_I8042_CONFIGURATION_FIRST_PORT_DISABLED;
    if (!atlas_i8042_wait_input_empty()) {
        return 0;
    }
    atlas_i8042_out8(ATLAS_I8042_COMMAND_PORT, ATLAS_I8042_COMMAND_WRITE_CONFIGURATION);
    if (!atlas_i8042_wait_input_empty()) {
        return 0;
    }
    atlas_i8042_out8(ATLAS_I8042_DATA_PORT, configuration);
    if (!atlas_i8042_wait_input_empty()) {
        return 0;
    }
    atlas_i8042_out8(ATLAS_I8042_COMMAND_PORT, ATLAS_I8042_COMMAND_ENABLE_FIRST_PORT);
    pulse_pic_remap_and_set_mask(UINT8_C(0xfd), UINT8_C(0xff));
    return 1;
}

int atlas_i8042_keyboard_handle_irq1(void) {
    uint8_t status = atlas_i8042_in8(ATLAS_I8042_STATUS_PORT);
    int accepted = 0;

    if ((status & ATLAS_I8042_STATUS_OUTPUT_FULL) != 0U &&
        (status & ATLAS_I8042_STATUS_AUXILIARY) == 0U) {
        accepted = atlas_keyboard_receive_scancode(atlas_i8042_in8(ATLAS_I8042_DATA_PORT));
    }
    pulse_pic_end_of_interrupt(UINT8_C(1));
    return accepted;
}
