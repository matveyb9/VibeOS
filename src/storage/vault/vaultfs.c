/* VibeOS VaultFS — initial redundant-superblock recovery implementation. */

#include "vaultfs.h"

static uint64_t vaultfs_checksum(const VAULTFS_SUPERBLOCK *superblock) {
    const uint8_t *bytes = (const uint8_t *)superblock;
    uint64_t value = UINT64_C(1469598103934665603);
    uint64_t index;

    for (index = 0; index < sizeof(VAULTFS_SUPERBLOCK) - sizeof(superblock->checksum); ++index) {
        value ^= bytes[index];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

static void vaultfs_copy(VAULTFS_SUPERBLOCK *destination, const VAULTFS_SUPERBLOCK *source) {
    const uint8_t *source_bytes = (const uint8_t *)source;
    uint8_t *destination_bytes = (uint8_t *)destination;
    uint64_t index;

    for (index = 0; index < sizeof(VAULTFS_SUPERBLOCK); ++index) {
        destination_bytes[index] = source_bytes[index];
    }
}

void vaultfs_superblock_initialize(
    VAULTFS_SUPERBLOCK *superblock,
    uint64_t generation,
    uint64_t active_system_slot,
    uint64_t journal_sequence) {
    if (superblock != (void *)0) {
        superblock->magic = VAULTFS_SUPERBLOCK_MAGIC;
        superblock->format_version = VAULTFS_FORMAT_VERSION;
        superblock->block_bytes = (uint32_t)ATLAS_BLOCK_BYTES;
        superblock->generation = generation;
        superblock->active_system_slot = active_system_slot;
        superblock->journal_sequence = journal_sequence;
        superblock->checksum = vaultfs_checksum(superblock);
    }
}

int vaultfs_superblock_valid(const VAULTFS_SUPERBLOCK *superblock) {
    return superblock != (void *)0 && superblock->magic == VAULTFS_SUPERBLOCK_MAGIC &&
           superblock->format_version == VAULTFS_FORMAT_VERSION &&
           superblock->block_bytes == ATLAS_BLOCK_BYTES &&
           superblock->checksum == vaultfs_checksum(superblock);
}

int vaultfs_superblock_store(
    ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    const VAULTFS_SUPERBLOCK *superblock) {
    if (!vaultfs_superblock_valid(superblock)) {
        return 0;
    }
    return atlas_block_write(device, block_index, superblock, sizeof(VAULTFS_SUPERBLOCK));
}

int vaultfs_superblock_load_latest(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t primary_block,
    uint64_t backup_block,
    VAULTFS_SUPERBLOCK *superblock) {
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    int primary_valid;
    int backup_valid;

    if (superblock == (void *)0 ||
        !atlas_block_read(device, primary_block, &primary, sizeof(primary)) ||
        !atlas_block_read(device, backup_block, &backup, sizeof(backup))) {
        return 0;
    }

    primary_valid = vaultfs_superblock_valid(&primary);
    backup_valid = vaultfs_superblock_valid(&backup);
    if (!primary_valid && !backup_valid) {
        return 0;
    }
    if (primary_valid && (!backup_valid || primary.generation >= backup.generation)) {
        vaultfs_copy(superblock, &primary);
    } else {
        vaultfs_copy(superblock, &backup);
    }
    return 1;
}

int vaultfs_runtime_probe(void) {
    static uint8_t storage[ATLAS_BLOCK_BYTES * 4U];
    ATLAS_RAM_BLOCK_DEVICE device;
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    VAULTFS_SUPERBLOCK recovered;

    if (!atlas_ram_block_device_init(&device, storage, sizeof(storage))) {
        return 0;
    }
    vaultfs_superblock_initialize(&primary, UINT64_C(5), 0U, UINT64_C(50));
    vaultfs_superblock_initialize(&backup, UINT64_C(4), 1U, UINT64_C(40));
    if (!vaultfs_superblock_store(&device, 0U, &primary) ||
        !vaultfs_superblock_store(&device, 1U, &backup)) {
        return 0;
    }

    primary.checksum ^= UINT64_C(1);
    if (!atlas_block_write(&device, 0U, &primary, sizeof(primary)) ||
        !vaultfs_superblock_load_latest(&device, 0U, 1U, &recovered)) {
        return 0;
    }
    return recovered.generation == UINT64_C(4) && recovered.active_system_slot == 1U;
}
