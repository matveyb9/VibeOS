/* VibeOS Origin — early composition of Keys and Relay. */

#include "origin.h"
#include "origin_abi.h"

#include <keys.h>
#include <relay.h>

#define ORIGIN_RUNTIME_PROBE_OBJECT UINT64_C(1)

int origin_runtime_probe(void) {
    VIBE_KEY authority_key;
    VIBE_KEY delegated_key;
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights;
    RELAY_LINK link;
    RELAY_CHANNEL channel;
    RELAY_MESSAGE message;
    ORIGIN_CALL call;

    origin_keys_reset();
    if (!origin_key_mint(
            ORIGIN_RUNTIME_PROBE_OBJECT,
            VIBE_RIGHT_READ | VIBE_RIGHT_WRITE | VIBE_RIGHT_INSPECT,
            &authority_key) ||
        !relay_link_create(&link, authority_key, VIBE_RIGHT_READ) ||
        !relay_link_transfer(&link, authority_key, VIBE_RIGHT_READ, &delegated_key) ||
        !origin_key_inspect(delegated_key, &object_id, &rights) ||
        object_id != ORIGIN_RUNTIME_PROBE_OBJECT || rights != VIBE_RIGHT_READ ||
        relay_link_transfer(&link, authority_key, VIBE_RIGHT_WRITE, &delegated_key) ||
        !relay_channel_open(&channel, authority_key, VIBE_RIGHT_READ) ||
        !relay_channel_send(&channel, authority_key, VIBE_RIGHT_READ, UINT64_C(0x56494245)) ||
        !relay_channel_receive(&channel, &message) || message.word != UINT64_C(0x56494245) ||
        !origin_key_inspect(message.key, &object_id, &rights) ||
        object_id != ORIGIN_RUNTIME_PROBE_OBJECT || rights != VIBE_RIGHT_READ) {
        return 0;
    }

    call.version = ORIGIN_ABI_VERSION;
    call.operation = ORIGIN_OPERATION_NARROW_KEY;
    call.input_key = authority_key;
    call.requested_rights = VIBE_RIGHT_INSPECT;
    call.reserved = 0;
    call.output_key = VIBE_KEY_INVALID;
    call.output_object = 0;
    call.output_rights = 0;
    call.status = ORIGIN_STATUS_BAD_FRAME;
    if (!origin_abi_dispatch(&call) || call.status != ORIGIN_STATUS_OK ||
        !origin_key_inspect(call.output_key, &object_id, &rights) ||
        object_id != ORIGIN_RUNTIME_PROBE_OBJECT || rights != VIBE_RIGHT_INSPECT) {
        return 0;
    }

    relay_link_close(&link);
    relay_channel_close(&channel);
    return 1;
}
