/* VibeOS Relay — early constrained Key transfer links. */

#include "relay.h"

int relay_link_create(RELAY_LINK *link, VIBE_KEY source_key, VIBE_RIGHTS rights_ceiling) {
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS source_rights;

    if (link == (void *)0 || rights_ceiling == 0U ||
        !origin_key_inspect(source_key, &object_id, &source_rights) ||
        (rights_ceiling & ~source_rights) != 0U) {
        return 0;
    }

    link->object_id = object_id;
    link->rights_ceiling = rights_ceiling;
    link->active = 1;
    return 1;
}

int relay_link_transfer(const RELAY_LINK *link, VIBE_KEY sender_key, VIBE_RIGHTS rights, VIBE_KEY *recipient_key) {
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS sender_rights;

    if (link == (void *)0 || recipient_key == (void *)0 || link->active == 0U || rights == 0U ||
        !origin_key_inspect(sender_key, &object_id, &sender_rights) || object_id != link->object_id ||
        (rights & ~link->rights_ceiling) != 0U || (rights & ~sender_rights) != 0U) {
        return 0;
    }

    return origin_key_narrow(sender_key, rights, recipient_key);
}

void relay_link_close(RELAY_LINK *link) {
    if (link != (void *)0) {
        link->object_id = 0;
        link->rights_ceiling = 0;
        link->active = 0;
    }
}
