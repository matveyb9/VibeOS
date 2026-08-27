/* VibeOS Atlas — host checks for bounded early keyboard-event decoding. */

#include <stdint.h>
#include <stdio.h>

#include <atlas_input.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    ATLAS_KEY_EVENT event;
    uint32_t index;

    atlas_keyboard_initialize();
    if (!expect(!atlas_keyboard_next_event(&event), "empty queue has no event") ||
        !expect(!atlas_keyboard_receive_scancode(UINT8_C(0xe0)), "extended prefix is deferred") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x23)), "make scancode is queued") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0xa3)), "break scancode is queued") ||
        !expect(atlas_keyboard_pending_event_count() == 2U, "queued event count is exact") ||
        !expect(atlas_keyboard_next_event(&event) && event.scancode == UINT8_C(0x23) &&
                    event.pressed == 1U && event.ascii == 'H', "make event decodes") ||
        !expect(atlas_keyboard_next_event(&event) && event.pressed == 0U && event.ascii == 'H',
                    "break event preserves normalized key") ||
        !expect(atlas_keyboard_pending_event_count() == 0U, "queue drains")) {
        return 1;
    }
    for (index = 0; index < ATLAS_KEYBOARD_QUEUE_CAPACITY; ++index) {
        if (!expect(atlas_keyboard_receive_scancode(UINT8_C(0x1e)), "queue accepts bounded event")) {
            return 1;
        }
    }
    if (!expect(!atlas_keyboard_receive_scancode(UINT8_C(0x1e)), "queue rejects overflow")) {
        return 1;
    }
    puts("Atlas keyboard bootstrap unit tests passed.");
    return 0;
}
