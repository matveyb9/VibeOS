/* VibeOS Keys — opaque authority tokens for Pulse Objects. */

#ifndef VIBEOS_KEYS_H
#define VIBEOS_KEYS_H

#include <stdint.h>

typedef uint64_t VIBE_OBJECT_ID;
typedef uint64_t VIBE_KEY;
typedef uint32_t VIBE_RIGHTS;

#define VIBE_KEY_INVALID UINT64_C(0)
#define VIBE_RIGHT_READ UINT32_C(0x00000001)
#define VIBE_RIGHT_WRITE UINT32_C(0x00000002)
#define VIBE_RIGHT_INSPECT UINT32_C(0x00000004)

void origin_keys_reset(void);
int origin_key_mint(VIBE_OBJECT_ID object_id, VIBE_RIGHTS rights, VIBE_KEY *key);
int origin_key_narrow(VIBE_KEY parent, VIBE_RIGHTS rights, VIBE_KEY *child);
int origin_key_inspect(VIBE_KEY key, VIBE_OBJECT_ID *object_id, VIBE_RIGHTS *rights);
int origin_key_revoke(VIBE_KEY key);

#endif
