/*
 * VibeOS Horizon input adapter — small policy bridge from already-normalized
 * Atlas key events to Horizon focus actions. It owns no queues or hardware.
 */

#include "horizon_input.h"

int horizon_input_apply_event(
    HORIZON_DESKTOP_STATE *state, const ATLAS_KEY_EVENT *event, uint32_t *handled) {
    HORIZON_DESKTOP_ACTION action;

    if (event == (const void *)0 || handled == (void *)0 || !horizon_desktop_state_is_valid(state)) {
        return 0;
    }
    *handled = 0U;
    if (event->pressed == 0U) {
        return 1;
    }
    if (event->ascii == 'N') {
        action = HORIZON_DESKTOP_ACTION_FOCUS_NEXT;
    } else if (event->ascii == 'P') {
        action = HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS;
    } else if (event->ascii == ' ') {
        action = HORIZON_DESKTOP_ACTION_SELECT_FOCUSED;
    } else {
        return 1;
    }
    if (!horizon_desktop_apply_action(state, action)) {
        return 0;
    }
    *handled = 1U;
    return 1;
}

int horizon_input_runtime_probe(void) {
    HORIZON_DESKTOP_STATE state;
    uint32_t handled;
    ATLAS_KEY_EVENT event = {UINT8_C(0x31), 1U, 'N'};

    return horizon_desktop_state_initialize(&state, 3U) &&
           horizon_input_apply_event(&state, &event, &handled) && handled == 1U && state.focused_window == 1U &&
           horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x19), 0U, 'P'}, &handled) &&
           handled == 0U && state.focused_window == 1U &&
           horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x39), 1U, ' '}, &handled) &&
           handled == 1U && state.selected_window == 1U;
}
