/* VibeOS Atlas — in-memory block backend used to verify storage contracts. */

#include "atlas_block.h"

static void atlas_copy(uint8_t *destination, const uint8_t *source, uint64_t byte_count) {
    uint64_t index;

    for (index = 0; index < byte_count; ++index) {
        destination[index] = source[index];
    }
}

static void atlas_zero(uint8_t *destination, uint64_t byte_count) {
    uint64_t index;

    for (index = 0; index < byte_count; ++index) {
        destination[index] = 0;
    }
}

int atlas_ram_block_device_init(ATLAS_RAM_BLOCK_DEVICE *device, uint8_t *bytes, uint64_t byte_count) {
    if (device == (void *)0 || bytes == (void *)0 || byte_count == 0U ||
        (byte_count % ATLAS_BLOCK_BYTES) != 0U) {
        return 0;
    }

    device->bytes = bytes;
    device->block_count = byte_count / ATLAS_BLOCK_BYTES;
    return 1;
}

int atlas_block_read(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    void *destination,
    uint64_t byte_count) {
    const uint8_t *block;

    if (device == (void *)0 || destination == (void *)0 || byte_count > ATLAS_BLOCK_BYTES ||
        block_index >= device->block_count) {
        return 0;
    }

    block = device->bytes + (block_index * ATLAS_BLOCK_BYTES);
    atlas_copy((uint8_t *)destination, block, byte_count);
    return 1;
}

int atlas_block_write(
    ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    const void *source,
    uint64_t byte_count) {
    uint8_t *block;

    if (device == (void *)0 || source == (void *)0 || byte_count > ATLAS_BLOCK_BYTES ||
        block_index >= device->block_count) {
        return 0;
    }

    block = device->bytes + (block_index * ATLAS_BLOCK_BYTES);
    atlas_copy(block, (const uint8_t *)source, byte_count);
    atlas_zero(block + byte_count, ATLAS_BLOCK_BYTES - byte_count);
    return 1;
}
