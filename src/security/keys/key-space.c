/* VibeOS Keys — early authoritative opaque-token key space. */

#include "keys.h"

#define ORIGIN_KEY_CAPACITY UINT32_C(64)

typedef struct {
    VIBE_KEY token;
    VIBE_OBJECT_ID object_id;
    VIBE_RIGHTS rights;
    uint32_t active;
} ORIGIN_KEY_RECORD;

static ORIGIN_KEY_RECORD origin_key_records[ORIGIN_KEY_CAPACITY];
static VIBE_KEY origin_next_key;

void origin_keys_reset(void) {
    uint32_t index;

    for (index = 0; index < ORIGIN_KEY_CAPACITY; ++index) {
        origin_key_records[index].token = VIBE_KEY_INVALID;
        origin_key_records[index].object_id = 0;
        origin_key_records[index].rights = 0;
        origin_key_records[index].active = 0;
    }
    origin_next_key = 1;
}

static ORIGIN_KEY_RECORD *origin_key_find(VIBE_KEY key) {
    uint32_t index;

    if (key == VIBE_KEY_INVALID) {
        return (void *)0;
    }
    for (index = 0; index < ORIGIN_KEY_CAPACITY; ++index) {
        if (origin_key_records[index].active != 0U && origin_key_records[index].token == key) {
            return &origin_key_records[index];
        }
    }
    return (void *)0;
}

static int origin_key_insert(VIBE_OBJECT_ID object_id, VIBE_RIGHTS rights, VIBE_KEY *key) {
    uint32_t index;

    if (key == (void *)0 || object_id == 0U || rights == 0U || origin_next_key == VIBE_KEY_INVALID) {
        return 0;
    }
    for (index = 0; index < ORIGIN_KEY_CAPACITY; ++index) {
        if (origin_key_records[index].active == 0U) {
            origin_key_records[index].token = origin_next_key;
            origin_key_records[index].object_id = object_id;
            origin_key_records[index].rights = rights;
            origin_key_records[index].active = 1;
            *key = origin_next_key;
            ++origin_next_key;
            return 1;
        }
    }
    return 0;
}

int origin_key_mint(VIBE_OBJECT_ID object_id, VIBE_RIGHTS rights, VIBE_KEY *key) {
    return origin_key_insert(object_id, rights, key);
}

int origin_key_narrow(VIBE_KEY parent, VIBE_RIGHTS rights, VIBE_KEY *child) {
    ORIGIN_KEY_RECORD *record = origin_key_find(parent);

    if (record == (void *)0 || rights == 0U || (rights & ~record->rights) != 0U) {
        return 0;
    }
    return origin_key_insert(record->object_id, rights, child);
}

int origin_key_inspect(VIBE_KEY key, VIBE_OBJECT_ID *object_id, VIBE_RIGHTS *rights) {
    ORIGIN_KEY_RECORD *record = origin_key_find(key);

    if (record == (void *)0 || object_id == (void *)0 || rights == (void *)0) {
        return 0;
    }
    *object_id = record->object_id;
    *rights = record->rights;
    return 1;
}

int origin_key_revoke(VIBE_KEY key) {
    ORIGIN_KEY_RECORD *record = origin_key_find(key);

    if (record == (void *)0) {
        return 0;
    }
    record->active = 0;
    return 1;
}
