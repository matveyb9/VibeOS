/* VibeOS Origin — early composition of Keys and Relay. */

#include "origin.h"

#include <keys.h>
#include <relay.h>

#define ORIGIN_RUNTIME_PROBE_OBJECT UINT64_C(1)

int origin_runtime_probe(void) {
    VIBE_KEY authority_key;
    VIBE_KEY delegated_key;
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights;
    RELAY_LINK link;

    origin_keys_reset();
    if (!origin_key_mint(
            ORIGIN_RUNTIME_PROBE_OBJECT,
            VIBE_RIGHT_READ | VIBE_RIGHT_WRITE | VIBE_RIGHT_INSPECT,
            &authority_key) ||
        !relay_link_create(&link, authority_key, VIBE_RIGHT_READ) ||
        !relay_link_transfer(&link, authority_key, VIBE_RIGHT_READ, &delegated_key) ||
        !origin_key_inspect(delegated_key, &object_id, &rights) ||
        object_id != ORIGIN_RUNTIME_PROBE_OBJECT || rights != VIBE_RIGHT_READ ||
        relay_link_transfer(&link, authority_key, VIBE_RIGHT_WRITE, &delegated_key)) {
        return 0;
    }

    relay_link_close(&link);
    return 1;
}
