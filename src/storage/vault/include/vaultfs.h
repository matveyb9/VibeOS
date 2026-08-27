/* VibeOS VaultFS — first redundant superblock and recovery contract. */

#ifndef VIBEOS_VAULTFS_H
#define VIBEOS_VAULTFS_H

#include <atlas_block.h>

#define VAULTFS_SUPERBLOCK_MAGIC UINT64_C(0x5641554c54465331)
#define VAULTFS_FORMAT_VERSION UINT32_C(1)
#define VAULTFS_JOURNAL_MAGIC UINT64_C(0x5641554c544a4e31)

typedef enum {
    VAULTFS_JOURNAL_PREPARED = 1,
    VAULTFS_JOURNAL_COMMITTED = 2
} VAULTFS_JOURNAL_STATE;

typedef struct __attribute__((packed)) {
    uint64_t magic;
    uint32_t format_version;
    uint32_t block_bytes;
    uint64_t generation;
    uint64_t active_system_slot;
    uint64_t journal_sequence;
    uint32_t checksum;
} VAULTFS_SUPERBLOCK;

typedef struct __attribute__((packed)) {
    uint64_t magic;
    uint64_t transaction_id;
    uint64_t target_block;
    uint32_t payload_checksum;
    uint32_t state;
    uint32_t checksum;
} VAULTFS_JOURNAL_ENTRY;

uint32_t vaultfs_crc32(const void *data, uint64_t byte_count);

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
void vaultfs_journal_prepare(
    VAULTFS_JOURNAL_ENTRY *entry,
    uint64_t transaction_id,
    uint64_t target_block,
    uint32_t payload_checksum);
int vaultfs_journal_valid(const VAULTFS_JOURNAL_ENTRY *entry);
int vaultfs_journal_commit(VAULTFS_JOURNAL_ENTRY *entry);
int vaultfs_runtime_probe(void);

#endif
