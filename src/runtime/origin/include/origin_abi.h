/* VibeOS Origin — native C ABI bootstrap for capability operations. */

#ifndef VIBEOS_ORIGIN_ABI_H
#define VIBEOS_ORIGIN_ABI_H

#include <keys.h>

#define ORIGIN_ABI_VERSION UINT32_C(1)

typedef enum {
    ORIGIN_OPERATION_INSPECT_KEY = 1,
    ORIGIN_OPERATION_NARROW_KEY = 2
} ORIGIN_OPERATION;

typedef enum {
    ORIGIN_STATUS_OK = 0,
    ORIGIN_STATUS_BAD_FRAME = 1,
    ORIGIN_STATUS_UNSUPPORTED = 2,
    ORIGIN_STATUS_DENIED = 3
} ORIGIN_STATUS;

typedef struct {
    uint32_t version;
    uint32_t operation;
    VIBE_KEY input_key;
    VIBE_RIGHTS requested_rights;
    uint32_t reserved;
    VIBE_KEY output_key;
    VIBE_OBJECT_ID output_object;
    VIBE_RIGHTS output_rights;
    uint32_t status;
} ORIGIN_CALL;

int origin_abi_dispatch(ORIGIN_CALL *call);

#endif
