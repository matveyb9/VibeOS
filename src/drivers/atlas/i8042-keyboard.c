/*
 * VibeOS Atlas — early i8042-compatible keyboard event decoder.
 *
 * Hardware reads and IRQ delivery remain a separate platform adapter. This
 * module owns only normalized set-1 scancode decoding and its bounded queue.
 */

#include "atlas_input.h"

typedef struct {
    ATLAS_KEY_EVENT events[ATLAS_KEYBOARD_QUEUE_CAPACITY];
    uint32_t read_index;
    uint32_t write_index;
    uint32_t event_count;
} ATLAS_KEYBOARD_QUEUE;

static ATLAS_KEYBOARD_QUEUE early_keyboard_queue;

static char atlas_keyboard_decode_set_1(uint8_t make_code) {
    switch (make_code) {
        case UINT8_C(0x10): return 'Q';
        case UINT8_C(0x11): return 'W';
        case UINT8_C(0x12): return 'E';
        case UINT8_C(0x13): return 'R';
        case UINT8_C(0x14): return 'T';
        case UINT8_C(0x15): return 'Y';
        case UINT8_C(0x16): return 'U';
        case UINT8_C(0x17): return 'I';
        case UINT8_C(0x18): return 'O';
        case UINT8_C(0x19): return 'P';
        case UINT8_C(0x1e): return 'A';
        case UINT8_C(0x1f): return 'S';
        case UINT8_C(0x20): return 'D';
        case UINT8_C(0x21): return 'F';
        case UINT8_C(0x22): return 'G';
        case UINT8_C(0x23): return 'H';
        case UINT8_C(0x24): return 'J';
        case UINT8_C(0x25): return 'K';
        case UINT8_C(0x26): return 'L';
        case UINT8_C(0x2c): return 'Z';
        case UINT8_C(0x2d): return 'X';
        case UINT8_C(0x2e): return 'C';
        case UINT8_C(0x2f): return 'V';
        case UINT8_C(0x30): return 'B';
        case UINT8_C(0x31): return 'N';
        case UINT8_C(0x32): return 'M';
        case UINT8_C(0x39): return ' ';
        default: return '\0';
    }
}

void atlas_keyboard_initialize(void) {
    uint32_t index;

    for (index = 0; index < ATLAS_KEYBOARD_QUEUE_CAPACITY; ++index) {
        early_keyboard_queue.events[index].scancode = 0U;
        early_keyboard_queue.events[index].pressed = 0U;
        early_keyboard_queue.events[index].ascii = '\0';
    }
    early_keyboard_queue.read_index = 0U;
    early_keyboard_queue.write_index = 0U;
    early_keyboard_queue.event_count = 0U;
}

int atlas_keyboard_receive_scancode(uint8_t scancode) {
    ATLAS_KEY_EVENT *event;
    uint8_t make_code;

    if (scancode == UINT8_C(0xe0) || scancode == UINT8_C(0xe1) ||
        early_keyboard_queue.event_count >= ATLAS_KEYBOARD_QUEUE_CAPACITY) {
        return 0;
    }
    event = &early_keyboard_queue.events[early_keyboard_queue.write_index];
    make_code = scancode & UINT8_C(0x7f);
    event->scancode = make_code;
    event->pressed = (scancode & UINT8_C(0x80)) == 0U ? 1U : 0U;
    event->ascii = atlas_keyboard_decode_set_1(make_code);
    early_keyboard_queue.write_index =
        (early_keyboard_queue.write_index + 1U) % ATLAS_KEYBOARD_QUEUE_CAPACITY;
    ++early_keyboard_queue.event_count;
    return 1;
}

int atlas_keyboard_next_event(ATLAS_KEY_EVENT *event) {
    if (event == (void *)0 || early_keyboard_queue.event_count == 0U) {
        return 0;
    }
    *event = early_keyboard_queue.events[early_keyboard_queue.read_index];
    early_keyboard_queue.read_index =
        (early_keyboard_queue.read_index + 1U) % ATLAS_KEYBOARD_QUEUE_CAPACITY;
    --early_keyboard_queue.event_count;
    return 1;
}

uint32_t atlas_keyboard_pending_event_count(void) {
    return early_keyboard_queue.event_count;
}

int atlas_keyboard_runtime_probe(void) {
    ATLAS_KEY_EVENT event;

    atlas_keyboard_initialize();
    return atlas_keyboard_receive_scancode(UINT8_C(0x23)) &&
           atlas_keyboard_receive_scancode(UINT8_C(0xa3)) &&
           atlas_keyboard_pending_event_count() == 2U && atlas_keyboard_next_event(&event) &&
           event.scancode == UINT8_C(0x23) && event.pressed == 1U && event.ascii == 'H' &&
           atlas_keyboard_next_event(&event) && event.pressed == 0U && event.ascii == 'H' &&
           atlas_keyboard_pending_event_count() == 0U;
}
