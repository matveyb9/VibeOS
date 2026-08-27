/* VibeOS Relay — host tests for opaque Key narrowing and transfer bounds. */

#include <stdint.h>
#include <stdio.h>

#include <keys.h>
#include <origin.h>
#include <relay.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    VIBE_KEY authority;
    VIBE_KEY read_key;
    VIBE_KEY write_key;
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights;
    RELAY_LINK link;

    origin_keys_reset();
    if (!expect(origin_key_mint(UINT64_C(42), VIBE_RIGHT_READ | VIBE_RIGHT_WRITE, &authority),
                    "authority key is minted") ||
        !expect(relay_link_create(&link, authority, VIBE_RIGHT_READ), "read-only relay is created") ||
        !expect(relay_link_transfer(&link, authority, VIBE_RIGHT_READ, &read_key),
                    "read authority is delegated") ||
        !expect(origin_key_inspect(read_key, &object_id, &rights) &&
                    object_id == UINT64_C(42) && rights == VIBE_RIGHT_READ,
                    "delegated key preserves object and narrows rights") ||
        !expect(!relay_link_transfer(&link, authority, VIBE_RIGHT_WRITE, &write_key),
                    "relay ceiling rejects rights amplification") ||
        !expect(origin_key_narrow(authority, VIBE_RIGHT_WRITE, &write_key),
                    "direct authority may create a distinct write key") ||
        !expect(!relay_link_transfer(&link, write_key, VIBE_RIGHT_READ, &read_key),
                    "sender without requested right cannot relay it") ||
        !expect(origin_key_revoke(authority), "authority key is revoked") ||
        !expect(!relay_link_transfer(&link, authority, VIBE_RIGHT_READ, &read_key),
                    "revoked key cannot be transferred") ||
        !expect(origin_runtime_probe(), "Origin runtime probe succeeds")) {
        return 1;
    }

    puts("Pulse Relay and Origin bootstrap unit tests passed.");
    return 0;
}
