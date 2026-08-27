/* VibeOS Atlas — bounded early keyboard-event boundary. */

#ifndef VIBEOS_ATLAS_INPUT_H
#define VIBEOS_ATLAS_INPUT_H

#include <stdint.h>

#define ATLAS_KEYBOARD_QUEUE_CAPACITY UINT32_C(32)

typedef enum {
    ATLAS_KEY_NONE = 0,
    ATLAS_KEY_TAB = 1,
    ATLAS_KEY_ENTER = 2
} ATLAS_KEY;

#define ATLAS_KEY_MODIFIER_SHIFT UINT8_C(0x01)

typedef struct {
    uint8_t scancode;
    uint8_t pressed;
    char ascii;
    uint8_t key;
    uint8_t modifiers;
} ATLAS_KEY_EVENT;

void atlas_keyboard_initialize(void);
int atlas_keyboard_receive_scancode(uint8_t scancode);
int atlas_keyboard_next_event(ATLAS_KEY_EVENT *event);
uint32_t atlas_keyboard_pending_event_count(void);
int atlas_keyboard_runtime_probe(void);
int atlas_i8042_keyboard_prepare_irq1(void);
int atlas_i8042_keyboard_handle_irq1(void);

#endif
