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
    uint32_t handled = UINT32_MAX;

    if (!expect(horizon_desktop_state_initialize(&state, 3U), "state initializes") ||
        !expect(!horizon_input_apply_event(&state, (void *)0, &handled), "null event is rejected") ||
        !expect(!horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x31), 1U, 'N'}, (void *)0),
                "null handled output is rejected") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x31), 1U, 'N'}, &handled) &&
                    handled == 1U && state.focused_window == 1U,
                "N press advances focus") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x31), 0U, 'N'}, &handled) &&
                    handled == 0U && state.focused_window == 1U,
                "key release is ignored") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x19), 1U, 'P'}, &handled) &&
                    handled == 1U && state.focused_window == 0U,
                "P press moves focus backward") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x39), 1U, ' '}, &handled) &&
                    handled == 1U && state.selected_window == 0U,
                "Space press selects focused window") ||
        !expect(horizon_input_apply_event(&state, &(ATLAS_KEY_EVENT){UINT8_C(0x23), 1U, 'H'}, &handled) &&
                    handled == 0U && state.selected_window == 0U,
                "unmapped press is ignored") ||
        !expect(horizon_input_runtime_probe(), "runtime input adapter self-check passes")) {
        return 1;
    }

    puts("Horizon input adapter unit tests passed.");
    return 0;
}
