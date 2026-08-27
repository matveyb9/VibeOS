/* VibeOS Session — fail-safe policy for early launch modes. */

#include "session_mode.h"

int vibe_session_select(const VIBE_SESSION_REQUEST *request, VIBE_SESSION_MODE *selected_mode) {
    if (request == (void *)0 || selected_mode == (void *)0) {
        return 0;
    }

    if (request->recovery_requested != 0U) {
        *selected_mode = VIBE_SESSION_RECOVERY;
        return 1;
    }

    switch (request->requested_mode) {
        case VIBE_SESSION_LIVE:
            *selected_mode = VIBE_SESSION_LIVE;
            return 1;
        case VIBE_SESSION_PERSISTENT_LIVE:
            *selected_mode = request->vault_available != 0U && request->vault_valid != 0U
                                 ? VIBE_SESSION_PERSISTENT_LIVE
                                 : VIBE_SESSION_LIVE;
            return 1;
        case VIBE_SESSION_INSTALLED:
            *selected_mode = request->vault_available != 0U && request->vault_valid != 0U
                                 ? VIBE_SESSION_INSTALLED
                                 : VIBE_SESSION_RECOVERY;
            return 1;
        case VIBE_SESSION_RECOVERY:
            *selected_mode = VIBE_SESSION_RECOVERY;
            return 1;
        default:
            *selected_mode = VIBE_SESSION_RECOVERY;
            return 0;
    }
}

int vibe_session_runtime_probe(void) {
    VIBE_SESSION_MODE selected_mode;

    return vibe_session_select(
               &(VIBE_SESSION_REQUEST){VIBE_SESSION_PERSISTENT_LIVE, 0U, 0U, 0U},
               &selected_mode) &&
           selected_mode == VIBE_SESSION_LIVE &&
           vibe_session_select(
               &(VIBE_SESSION_REQUEST){VIBE_SESSION_INSTALLED, 1U, 1U, 0U},
               &selected_mode) &&
           selected_mode == VIBE_SESSION_INSTALLED &&
           vibe_session_select(
               &(VIBE_SESSION_REQUEST){VIBE_SESSION_INSTALLED, 1U, 0U, 0U},
               &selected_mode) &&
           selected_mode == VIBE_SESSION_RECOVERY &&
           vibe_session_select(
               &(VIBE_SESSION_REQUEST){VIBE_SESSION_LIVE, 1U, 1U, 1U},
               &selected_mode) &&
           selected_mode == VIBE_SESSION_RECOVERY;
}
