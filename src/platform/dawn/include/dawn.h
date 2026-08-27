/*
 * VibeOS Dawn Context — platform-neutral boot contract between Prelude and Pulse.
 *
 * ABI rule: this layout is append-only. Fields store physical addresses and sizes
 * rather than pointers whose representation could vary on later architectures.
 */

#ifndef VIBEOS_DAWN_CONTEXT_H
#define VIBEOS_DAWN_CONTEXT_H

#include <stdint.h>

#define DAWN_CONTEXT_MAGIC UINT64_C(0x4441574E43545831)
#define DAWN_CONTEXT_VERSION UINT32_C(1)

typedef struct {
    uint64_t magic;
    uint32_t version;
    uint32_t size;
    uint64_t memory_map_physical_address;
    uint64_t memory_map_size;
    uint64_t memory_map_key;
    uint64_t memory_descriptor_size;
    uint32_t memory_descriptor_version;
    uint32_t reserved;
} DAWN_CONTEXT;

#endif
