/*
 * VibeOS Horizon — bounded desktop focus model separate from Canvas rendering
 * and any Atlas input transport. It models intent only; it launches nothing.
 */

#include "horizon.h"

int horizon_desktop_state_is_valid(const HORIZON_DESKTOP_STATE *state) {
    return state != (const void *)0 && state->window_count != 0U &&
           state->window_count <= HORIZON_DESKTOP_MAX_WINDOWS &&
           state->focused_window < state->window_count &&
           (state->selected_window == HORIZON_DESKTOP_NO_WINDOW ||
            state->selected_window < state->window_count);
}

int horizon_desktop_state_initialize(HORIZON_DESKTOP_STATE *state, uint32_t window_count) {
    if (state == (void *)0 || window_count == 0U || window_count > HORIZON_DESKTOP_MAX_WINDOWS) {
        return 0;
    }
    state->window_count = window_count;
    state->focused_window = 0U;
    state->selected_window = HORIZON_DESKTOP_NO_WINDOW;
    return 1;
}

int horizon_desktop_apply_action(HORIZON_DESKTOP_STATE *state, HORIZON_DESKTOP_ACTION action) {
    if (!horizon_desktop_state_is_valid(state)) {
        return 0;
    }
    if (action == HORIZON_DESKTOP_ACTION_FOCUS_NEXT) {
        state->focused_window = (state->focused_window + 1U) % state->window_count;
        return 1;
    }
    if (action == HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS) {
        state->focused_window = state->focused_window == 0U ? state->window_count - 1U : state->focused_window - 1U;
        return 1;
    }
    if (action == HORIZON_DESKTOP_ACTION_SELECT_FOCUSED) {
        state->selected_window = state->focused_window;
        return 1;
    }
    return 0;
}

int horizon_runtime_probe(void) {
    HORIZON_DESKTOP_STATE state;

    return horizon_desktop_state_initialize(&state, 3U) &&
           horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_FOCUS_NEXT) &&
           horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_FOCUS_NEXT) &&
           horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_FOCUS_NEXT) &&
           state.focused_window == 0U &&
           horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS) &&
           horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_SELECT_FOCUSED) &&
           state.focused_window == 2U && state.selected_window == 2U;
}
