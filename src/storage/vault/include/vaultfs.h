/* VibeOS VaultFS — first redundant superblock and recovery contract. */

#ifndef VIBEOS_VAULTFS_H
#define VIBEOS_VAULTFS_H

#include <atlas_block.h>

#define VAULTFS_SUPERBLOCK_MAGIC UINT64_C(0x5641554c54465331)
#define VAULTFS_FORMAT_VERSION UINT32_C(1)
#define VAULTFS_JOURNAL_MAGIC UINT64_C(0x5641554c544a4e31)
#define VAULTFS_SYSTEM_SLOT_NONE UINT64_MAX
#define VAULTFS_DIRECTORY_CAPACITY UINT32_C(16)
#define VAULTFS_ENTRY_NAME_BYTES UINT32_C(32)

typedef enum {
    VAULTFS_JOURNAL_PREPARED = 1,
    VAULTFS_JOURNAL_COMMITTED = 2
} VAULTFS_JOURNAL_STATE;

typedef enum {
    VAULTFS_ENTRY_FILE = 1,
    VAULTFS_ENTRY_DIRECTORY = 2
} VAULTFS_ENTRY_KIND;

typedef struct __attribute__((packed)) {
    uint64_t magic;
    uint32_t format_version;
    uint32_t block_bytes;
    uint64_t generation;
    uint64_t active_system_slot;
    uint64_t pending_system_slot;
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

typedef struct {
    uint64_t object_id;
    uint64_t first_block;
    uint64_t byte_count;
    uint32_t kind;
    char name[VAULTFS_ENTRY_NAME_BYTES];
} VAULTFS_DIRECTORY_ENTRY;

typedef struct {
    VAULTFS_DIRECTORY_ENTRY entries[VAULTFS_DIRECTORY_CAPACITY];
    uint32_t count;
} VAULTFS_DIRECTORY;

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
int vaultfs_system_slot_stage(VAULTFS_SUPERBLOCK *superblock, uint64_t target_slot);
int vaultfs_system_slot_confirm(VAULTFS_SUPERBLOCK *superblock);
int vaultfs_system_slot_recover(const VAULTFS_SUPERBLOCK *superblock, uint64_t *boot_slot);
void vaultfs_directory_initialize(VAULTFS_DIRECTORY *directory);
int vaultfs_directory_entry_valid(const VAULTFS_DIRECTORY_ENTRY *entry);
int vaultfs_directory_insert(VAULTFS_DIRECTORY *directory, const VAULTFS_DIRECTORY_ENTRY *entry);
const VAULTFS_DIRECTORY_ENTRY *vaultfs_directory_find(
    const VAULTFS_DIRECTORY *directory, const char *name);
int vaultfs_runtime_probe(void);

#endif
