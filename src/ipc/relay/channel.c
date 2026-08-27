/* VibeOS Relay — bounded single-kernel bootstrap message channel. */

#include "relay.h"

int relay_channel_open(RELAY_CHANNEL *channel, VIBE_KEY source_key, VIBE_RIGHTS rights_ceiling) {
    uint32_t index;

    if (channel == (void *)0 || !relay_link_create(&channel->link, source_key, rights_ceiling)) {
        return 0;
    }
    for (index = 0; index < RELAY_CHANNEL_CAPACITY; ++index) {
        channel->messages[index].word = 0;
        channel->messages[index].key = VIBE_KEY_INVALID;
    }
    channel->read_index = 0;
    channel->write_index = 0;
    channel->count = 0;
    return 1;
}

int relay_channel_send(
    RELAY_CHANNEL *channel,
    VIBE_KEY sender_key,
    VIBE_RIGHTS transferred_rights,
    uint64_t word) {
    VIBE_KEY recipient_key;

    if (channel == (void *)0 || channel->count >= RELAY_CHANNEL_CAPACITY ||
        !relay_link_transfer(&channel->link, sender_key, transferred_rights, &recipient_key)) {
        return 0;
    }

    channel->messages[channel->write_index].word = word;
    channel->messages[channel->write_index].key = recipient_key;
    channel->write_index = (channel->write_index + 1U) % RELAY_CHANNEL_CAPACITY;
    ++channel->count;
    return 1;
}

int relay_channel_receive(RELAY_CHANNEL *channel, RELAY_MESSAGE *message) {
    if (channel == (void *)0 || message == (void *)0 || channel->count == 0U) {
        return 0;
    }

    *message = channel->messages[channel->read_index];
    channel->messages[channel->read_index].word = 0;
    channel->messages[channel->read_index].key = VIBE_KEY_INVALID;
    channel->read_index = (channel->read_index + 1U) % RELAY_CHANNEL_CAPACITY;
    --channel->count;
    return 1;
}

void relay_channel_close(RELAY_CHANNEL *channel) {
    uint32_t index;

    if (channel != (void *)0) {
        relay_link_close(&channel->link);
        for (index = 0; index < RELAY_CHANNEL_CAPACITY; ++index) {
            channel->messages[index].word = 0;
            channel->messages[index].key = VIBE_KEY_INVALID;
        }
        channel->read_index = 0;
        channel->write_index = 0;
        channel->count = 0;
    }
}
