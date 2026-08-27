/* VibeOS Relay — constrained transfer links for opaque Keys. */

#ifndef VIBEOS_RELAY_H
#define VIBEOS_RELAY_H

#include <keys.h>

typedef struct {
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights_ceiling;
    uint32_t active;
} RELAY_LINK;

#define RELAY_CHANNEL_CAPACITY UINT32_C(8)

typedef struct {
    uint64_t word;
    VIBE_KEY key;
} RELAY_MESSAGE;

typedef struct {
    RELAY_LINK link;
    RELAY_MESSAGE messages[RELAY_CHANNEL_CAPACITY];
    uint32_t read_index;
    uint32_t write_index;
    uint32_t count;
} RELAY_CHANNEL;

int relay_link_create(RELAY_LINK *link, VIBE_KEY source_key, VIBE_RIGHTS rights_ceiling);
int relay_link_transfer(const RELAY_LINK *link, VIBE_KEY sender_key, VIBE_RIGHTS rights, VIBE_KEY *recipient_key);
void relay_link_close(RELAY_LINK *link);
int relay_channel_open(RELAY_CHANNEL *channel, VIBE_KEY source_key, VIBE_RIGHTS rights_ceiling);
int relay_channel_send(
    RELAY_CHANNEL *channel,
    VIBE_KEY sender_key,
    VIBE_RIGHTS transferred_rights,
    uint64_t word);
int relay_channel_receive(RELAY_CHANNEL *channel, RELAY_MESSAGE *message);
void relay_channel_close(RELAY_CHANNEL *channel);

#endif
