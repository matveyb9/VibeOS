/* VibeOS Relay — constrained transfer links for opaque Keys. */

#ifndef VIBEOS_RELAY_H
#define VIBEOS_RELAY_H

#include <keys.h>

typedef struct {
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights_ceiling;
    uint32_t active;
} RELAY_LINK;

int relay_link_create(RELAY_LINK *link, VIBE_KEY source_key, VIBE_RIGHTS rights_ceiling);
int relay_link_transfer(const RELAY_LINK *link, VIBE_KEY sender_key, VIBE_RIGHTS rights, VIBE_KEY *recipient_key);
void relay_link_close(RELAY_LINK *link);

#endif
