/* VibeOS Session — host checks for fail-safe launch-mode policy. */

#include <stdio.h>

#include <session_mode.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    VIBE_SESSION_MODE selected_mode;

    if (!expect(vibe_session_select(
                    &(VIBE_SESSION_REQUEST){VIBE_SESSION_LIVE, 0U, 0U, 0U},
                    &selected_mode) && selected_mode == VIBE_SESSION_LIVE,
                    "live mode does not require Vault") ||
        !expect(vibe_session_select(
                    &(VIBE_SESSION_REQUEST){VIBE_SESSION_PERSISTENT_LIVE, 1U, 0U, 0U},
                    &selected_mode) && selected_mode == VIBE_SESSION_LIVE,
                    "invalid persistence falls back to live") ||
        !expect(vibe_session_select(
                    &(VIBE_SESSION_REQUEST){VIBE_SESSION_INSTALLED, 1U, 0U, 0U},
                    &selected_mode) && selected_mode == VIBE_SESSION_RECOVERY,
                    "invalid installed Vault enters recovery") ||
        !expect(vibe_session_select(
                    &(VIBE_SESSION_REQUEST){VIBE_SESSION_INSTALLED, 1U, 1U, 0U},
                    &selected_mode) && selected_mode == VIBE_SESSION_INSTALLED,
                    "valid installed Vault starts installed mode") ||
        !expect(vibe_session_select(
                    &(VIBE_SESSION_REQUEST){VIBE_SESSION_LIVE, 1U, 1U, 1U},
                    &selected_mode) && selected_mode == VIBE_SESSION_RECOVERY,
                    "explicit recovery overrides requested mode") ||
        !expect(!vibe_session_select(
                    &(VIBE_SESSION_REQUEST){(VIBE_SESSION_MODE)99, 1U, 1U, 0U},
                    &selected_mode) && selected_mode == VIBE_SESSION_RECOVERY,
                    "unknown mode fails safely into recovery")) {
        return 1;
    }

    puts("VibeOS session-mode bootstrap unit tests passed.");
    return 0;
}
