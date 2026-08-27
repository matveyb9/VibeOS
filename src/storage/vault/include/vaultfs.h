/* VibeOS VaultFS — first redundant superblock and recovery contract. */

#ifndef VIBEOS_VAULTFS_H
#define VIBEOS_VAULTFS_H

#include <atlas_block.h>

#define VAULTFS_SUPERBLOCK_MAGIC UINT64_C(0x5641554c54465331)
#define VAULTFS_FORMAT_VERSION UINT32_C(1)

typedef struct __attribute__((packed)) {
    uint64_t magic;
    uint32_t format_version;
    uint32_t block_bytes;
    uint64_t generation;
    uint64_t active_system_slot;
    uint64_t journal_sequence;
    uint64_t checksum;
} VAULTFS_SUPERBLOCK;

void vaultfs_superblock_initialize(
    VAULTFS_SUPERBLOCK *superblock,
    uint64_t generation,
    uint64_t active_system_slot,
    uint64_t journal_sequence);
int vaultfs_superblock_valid(const VAULTFS_SUPERBLOCK *superblock);
int vaultfs_superblock_store(
    ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    const VAULTFS_SUPERBLOCK *superblock);
int vaultfs_superblock_load_latest(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t primary_block,
    uint64_t backup_block,
    VAULTFS_SUPERBLOCK *superblock);
int vaultfs_runtime_probe(void);

#endif
