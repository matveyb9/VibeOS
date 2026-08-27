/* VibeOS Atlas — minimal block-device boundary for early storage modules. */

#ifndef VIBEOS_ATLAS_BLOCK_H
#define VIBEOS_ATLAS_BLOCK_H

#include <stdint.h>

#define ATLAS_BLOCK_BYTES UINT64_C(4096)

typedef struct {
    uint8_t *bytes;
    uint64_t block_count;
} ATLAS_RAM_BLOCK_DEVICE;

int atlas_ram_block_device_init(ATLAS_RAM_BLOCK_DEVICE *device, uint8_t *bytes, uint64_t byte_count);
int atlas_block_read(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    void *destination,
    uint64_t byte_count);
int atlas_block_write(
    ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    const void *source,
    uint64_t byte_count);

#endif
