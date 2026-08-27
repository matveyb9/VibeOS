/* VibeOS Session — bootstrap launch-mode policy contract. */

#ifndef VIBEOS_SESSION_MODE_H
#define VIBEOS_SESSION_MODE_H

#include <stdint.h>

typedef enum {
    VIBE_SESSION_LIVE = 1,
    VIBE_SESSION_PERSISTENT_LIVE = 2,
    VIBE_SESSION_INSTALLED = 3,
    VIBE_SESSION_RECOVERY = 4
} VIBE_SESSION_MODE;

typedef struct {
    VIBE_SESSION_MODE requested_mode;
    uint32_t vault_available;
    uint32_t vault_valid;
    uint32_t recovery_requested;
} VIBE_SESSION_REQUEST;

int vibe_session_select(const VIBE_SESSION_REQUEST *request, VIBE_SESSION_MODE *selected_mode);
int vibe_session_runtime_probe(void);

#endif
