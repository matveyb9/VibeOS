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
    if (event->key == ATLAS_KEY_TAB) {
        action = (event->modifiers & ATLAS_KEY_MODIFIER_SHIFT) != 0U ?
                     HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS : HORIZON_DESKTOP_ACTION_FOCUS_NEXT;
    } else if (event->key == ATLAS_KEY_ARROW_LEFT) {
        action = HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS;
    } else if (event->key == ATLAS_KEY_ARROW_RIGHT) {
        action = HORIZON_DESKTOP_ACTION_FOCUS_NEXT;
    } else if (event->key == ATLAS_KEY_ENTER) {
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

int horizon_input_pump(
    HORIZON_DESKTOP_STATE *state, uint32_t maximum_events, HORIZON_INPUT_PUMP_RESULT *result) {
    ATLAS_KEY_EVENT event;
    uint32_t event_index;

    if (result == (void *)0 || !horizon_desktop_state_is_valid(state) || maximum_events == 0U ||
        maximum_events > HORIZON_INPUT_PUMP_MAX_EVENTS) {
        return 0;
    }
    result->dequeued_event_count = 0U;
    result->handled_event_count = 0U;
    result->redraw_requested = 0U;
    for (event_index = 0U; event_index < maximum_events && atlas_keyboard_next_event(&event); ++event_index) {
        uint32_t handled;
        uint32_t previous_focus = state->focused_window;
        uint32_t previous_selection = state->selected_window;

        if (!horizon_input_apply_event(state, &event, &handled)) {
            return 0;
        }
        ++result->dequeued_event_count;
        result->handled_event_count += handled;
        if (state->focused_window != previous_focus || state->selected_window != previous_selection) {
            result->redraw_requested = 1U;
        }
    }
    return 1;
}

int horizon_input_runtime_probe(void) {
    HORIZON_DESKTOP_STATE state;
    HORIZON_INPUT_PUMP_RESULT result;
    uint32_t handled;
    ATLAS_KEY_EVENT event = {UINT8_C(0x0f), 1U, '\0', ATLAS_KEY_TAB, ATLAS_KEY_NONE};

    atlas_keyboard_initialize();
    if (!atlas_keyboard_receive_scancode(UINT8_C(0x0f)) ||
        !atlas_keyboard_receive_scancode(UINT8_C(0x8f)) ||
        !atlas_keyboard_receive_scancode(UINT8_C(0x1c)) ||
        !atlas_keyboard_receive_scancode(UINT8_C(0x0f))) {
        return 0;
    }
    return horizon_desktop_state_initialize(&state, 3U) &&
           horizon_input_apply_event(&state, &event, &handled) && handled == 1U && state.focused_window == 1U &&
           horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x0f), 0U, '\0', ATLAS_KEY_TAB, ATLAS_KEY_NONE}, &handled) &&
           handled == 0U && state.focused_window == 1U &&
           horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x1c), 1U, '\0', ATLAS_KEY_ENTER, ATLAS_KEY_NONE}, &handled) &&
           handled == 1U && state.selected_window == 1U && horizon_desktop_state_initialize(&state, 3U) &&
           horizon_input_pump(&state, 4U, &result) && result.dequeued_event_count == 4U &&
           result.handled_event_count == 3U && result.redraw_requested == 1U && state.focused_window == 2U &&
           state.selected_window == 1U && atlas_keyboard_pending_event_count() == 0U;
}
