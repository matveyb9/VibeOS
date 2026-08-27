/* Horizon focus test — bounded navigation is deterministic without a device driver. */

#include <stdio.h>

#include "horizon.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    HORIZON_DESKTOP_STATE state;

    if (!expect(!horizon_desktop_state_initialize((void *)0, 3U), "null state is rejected") ||
        !expect(!horizon_desktop_state_initialize(&state, 0U), "zero window count is rejected") ||
        !expect(!horizon_desktop_state_initialize(&state, HORIZON_DESKTOP_MAX_WINDOWS + 1U),
                "over-capacity window count is rejected") ||
        !expect(horizon_desktop_state_initialize(&state, 3U), "three-window state initializes") ||
        !expect(horizon_desktop_state_is_valid(&state) && state.focused_window == 0U &&
                    state.selected_window == HORIZON_DESKTOP_NO_WINDOW,
                "initial focus and empty selection are explicit") ||
        !expect(horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS) &&
                    state.focused_window == 2U,
                "previous focus wraps backward") ||
        !expect(horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_FOCUS_NEXT) &&
                    state.focused_window == 0U,
                "next focus wraps forward") ||
        !expect(horizon_desktop_apply_action(&state, HORIZON_DESKTOP_ACTION_SELECT_FOCUSED) &&
                    state.selected_window == 0U,
                "selection retains focused window") ||
        !expect(!horizon_desktop_apply_action(&state, (HORIZON_DESKTOP_ACTION)0),
                "unknown action is rejected") ||
        !expect(horizon_runtime_probe(), "runtime focus self-check passes")) {
        return 1;
    }

    puts("Horizon focus unit tests passed.");
    return 0;
}
