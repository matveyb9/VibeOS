/* Horizon input adapter test — consumes normalized events without device access. */

#include <stdio.h>

#include "horizon_input.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    HORIZON_DESKTOP_STATE state;
    HORIZON_INPUT_PUMP_RESULT result;
    uint32_t handled = UINT32_MAX;

    atlas_keyboard_initialize();
    if (!expect(horizon_desktop_state_initialize(&state, 3U), "state initializes") ||
        !expect(!horizon_input_apply_event(&state, (void *)0, &handled), "null event is rejected") ||
        !expect(!horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x0f), 1U, '\0', ATLAS_KEY_TAB}, (void *)0),
                "null handled output is rejected") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x0f), 1U, '\0', ATLAS_KEY_TAB}, &handled) &&
                    handled == 1U && state.focused_window == 1U,
                "Tab press advances focus") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x0f), 0U, '\0', ATLAS_KEY_TAB}, &handled) &&
                    handled == 0U && state.focused_window == 1U,
                "key release is ignored") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x1c), 1U, '\0', ATLAS_KEY_ENTER}, &handled) &&
                    handled == 1U && state.selected_window == 1U,
                "Enter press selects focused window") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x23), 1U, 'H', ATLAS_KEY_NONE}, &handled) &&
                    handled == 0U && state.selected_window == 1U,
                "unmapped press is ignored") ||
        !expect(!horizon_input_pump(&state, 0U, &result), "zero pump budget is rejected") ||
        !expect(!horizon_input_pump(&state, HORIZON_INPUT_PUMP_MAX_EVENTS + 1U, &result),
                "over-budget pump is rejected") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x0f)) &&
                    atlas_keyboard_receive_scancode(UINT8_C(0x8f)) &&
                    atlas_keyboard_receive_scancode(UINT8_C(0x1c)) &&
                    horizon_desktop_state_initialize(&state, 3U) && horizon_input_pump(&state, 2U, &result) &&
                    result.dequeued_event_count == 2U && result.handled_event_count == 1U && result.redraw_requested == 1U &&
                    state.focused_window == 1U && atlas_keyboard_pending_event_count() == 1U,
                "bounded pump consumes only its budget and requests redraw for a focus change") ||
        !expect(horizon_input_pump(&state, 2U, &result) && result.dequeued_event_count == 1U &&
                    result.handled_event_count == 1U && result.redraw_requested == 1U &&
                    state.selected_window == 1U && atlas_keyboard_pending_event_count() == 0U,
                "later pump consumes remaining selection event") ||
        !expect(horizon_input_runtime_probe(), "runtime input adapter self-check passes")) {
        return 1;
    }

    puts("Horizon input adapter unit tests passed.");
    return 0;
}
