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
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x0f)), "Tab scancode is queued") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x1c)), "Enter scancode is queued") ||
        !expect(atlas_keyboard_pending_event_count() == 4U, "queued event count is exact") ||
        !expect(atlas_keyboard_next_event(&event) && event.scancode == UINT8_C(0x23) &&
                    event.pressed == 1U && event.ascii == 'H' && event.key == ATLAS_KEY_NONE,
                "text make event decodes") ||
        !expect(atlas_keyboard_next_event(&event) && event.pressed == 0U && event.ascii == 'H' &&
                    event.key == ATLAS_KEY_NONE,
                    "break event preserves normalized key") ||
        !expect(atlas_keyboard_next_event(&event) && event.key == ATLAS_KEY_TAB && event.ascii == '\0',
                "Tab retains semantic identity without text") ||
        !expect(atlas_keyboard_next_event(&event) && event.key == ATLAS_KEY_ENTER && event.ascii == '\0',
                "Enter retains semantic identity without text") ||
        !expect(atlas_keyboard_pending_event_count() == 0U, "queue drains")) {
        return 1;
    }
    atlas_keyboard_initialize();
    if (!expect(atlas_keyboard_receive_scancode(UINT8_C(0x2a)), "Shift press is queued") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x0f)), "Shift+Tab is queued") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0xaa)), "Shift release is queued") ||
        !expect(atlas_keyboard_next_event(&event) && event.modifiers == ATLAS_KEY_MODIFIER_SHIFT,
                "Shift press sets the following modifier state") ||
        !expect(atlas_keyboard_next_event(&event) && event.key == ATLAS_KEY_TAB &&
                    event.modifiers == ATLAS_KEY_MODIFIER_SHIFT,
                "Tab retains active Shift modifier") ||
        !expect(atlas_keyboard_next_event(&event) && event.modifiers == ATLAS_KEY_NONE,
                "Shift release clears modifier state") ||
        !expect(atlas_keyboard_pending_event_count() == 0U, "modifier queue drains")) {
        return 1;
    }
    atlas_keyboard_initialize();
    if (!expect(!atlas_keyboard_receive_scancode(UINT8_C(0xe0)), "E0 prefix is held outside the queue") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x4d)), "E0 right-arrow code is queued") ||
        !expect(atlas_keyboard_next_event(&event) && event.key == ATLAS_KEY_ARROW_RIGHT && event.ascii == '\0',
                "E0 right-arrow decodes to semantic key") ||
        !expect(!atlas_keyboard_receive_scancode(UINT8_C(0xe0)), "second E0 prefix is held outside the queue") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x4b)), "E0 left-arrow code is queued") ||
        !expect(atlas_keyboard_next_event(&event) && event.key == ATLAS_KEY_ARROW_LEFT && event.ascii == '\0',
                "E0 left-arrow decodes to semantic key") ||
        !expect(atlas_keyboard_pending_event_count() == 0U, "extended key queue drains")) {
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
