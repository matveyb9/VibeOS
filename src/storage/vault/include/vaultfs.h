/* VibeOS VaultFS — first redundant superblock and recovery contract. */

#ifndef VIBEOS_VAULTFS_H
#define VIBEOS_VAULTFS_H

#include <atlas_block.h>

#define VAULTFS_SUPERBLOCK_MAGIC UINT64_C(0x5641554c54465331)
#define VAULTFS_FORMAT_VERSION UINT32_C(3)
#define VAULTFS_SUPERBLOCK_WIRE_BYTES UINT32_C(68)
#define VAULTFS_JOURNAL_MAGIC UINT64_C(0x5641554c544a4e31)
#define VAULTFS_JOURNAL_WIRE_BYTES UINT32_C(36)
#define VAULTFS_SYSTEM_SLOT_NONE UINT64_MAX
#define VAULTFS_ROOT_DIRECTORY_BLOCK_NONE UINT64_MAX
#define VAULTFS_ROOT_DIRECTORY_MAGIC UINT64_C(0x5641554c54524431)
#define VAULTFS_ROOT_DIRECTORY_VERSION UINT32_C(1)
#define VAULTFS_ROOT_DIRECTORY_WIRE_BYTES UINT32_C(988)
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

typedef enum {
    VAULTFS_RECOVERY_DISCARD_PREPARED = 1,
    VAULTFS_RECOVERY_ACCEPT_COMMITTED = 2
} VAULTFS_RECOVERY_DECISION;

typedef struct {
    uint64_t magic;
    uint32_t format_version;
    uint32_t block_bytes;
    uint64_t generation;
    uint64_t active_system_slot;
    uint64_t pending_system_slot;
    uint64_t root_directory_block;
    uint64_t backup_root_directory_block;
    uint64_t journal_sequence;
    uint32_t checksum;
} VAULTFS_SUPERBLOCK;

typedef struct {
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

typedef struct {
    uint64_t magic;
    uint32_t format_version;
    uint32_t entry_count;
    uint64_t generation;
    VAULTFS_DIRECTORY_ENTRY entries[VAULTFS_DIRECTORY_CAPACITY];
    uint32_t checksum;
} VAULTFS_ROOT_DIRECTORY_BLOCK;

typedef struct {
    uint64_t transaction_id;
    uint64_t target_root_block;
    uint64_t next_generation;
    uint32_t payload_checksum;
} VAULTFS_ROOT_UPDATE_PLAN;

uint32_t vaultfs_crc32(const void *data, uint64_t byte_count);

void vaultfs_superblock_initialize(
    VAULTFS_SUPERBLOCK *superblock,
    uint64_t generation,
    uint64_t active_system_slot,
    uint64_t root_directory_block,
    uint64_t backup_root_directory_block,
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
int vaultfs_journal_store(
    ATLAS_RAM_BLOCK_DEVICE *device, uint64_t block_index, const VAULTFS_JOURNAL_ENTRY *entry);
int vaultfs_journal_load(
    const ATLAS_RAM_BLOCK_DEVICE *device, uint64_t block_index, VAULTFS_JOURNAL_ENTRY *entry);
int vaultfs_recovery_decide(
    const VAULTFS_SUPERBLOCK *superblock,
    const VAULTFS_JOURNAL_ENTRY *journal,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block,
    VAULTFS_RECOVERY_DECISION *decision);
int vaultfs_root_update_plan_form(
    const VAULTFS_SUPERBLOCK *superblock,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *next_root_block,
    uint64_t transaction_id,
    VAULTFS_ROOT_UPDATE_PLAN *plan);
int vaultfs_root_update_journal_prepare(
    const VAULTFS_ROOT_UPDATE_PLAN *plan, VAULTFS_JOURNAL_ENTRY *journal);
int vaultfs_system_slot_stage(VAULTFS_SUPERBLOCK *superblock, uint64_t target_slot);
int vaultfs_system_slot_confirm(VAULTFS_SUPERBLOCK *superblock);
int vaultfs_system_slot_recover(const VAULTFS_SUPERBLOCK *superblock, uint64_t *boot_slot);
void vaultfs_directory_initialize(VAULTFS_DIRECTORY *directory);
int vaultfs_directory_entry_valid(const VAULTFS_DIRECTORY_ENTRY *entry);
int vaultfs_directory_insert(VAULTFS_DIRECTORY *directory, const VAULTFS_DIRECTORY_ENTRY *entry);
const VAULTFS_DIRECTORY_ENTRY *vaultfs_directory_find(
    const VAULTFS_DIRECTORY *directory, const char *name);
void vaultfs_root_directory_block_initialize(
    VAULTFS_ROOT_DIRECTORY_BLOCK *root_block,
    const VAULTFS_DIRECTORY *directory,
    uint64_t generation);
int vaultfs_root_directory_block_valid(const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block);
int vaultfs_root_directory_block_store(
    ATLAS_RAM_BLOCK_DEVICE *device,
    const VAULTFS_SUPERBLOCK *superblock,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block);
int vaultfs_root_directory_backup_block_store(
    ATLAS_RAM_BLOCK_DEVICE *device,
    const VAULTFS_SUPERBLOCK *superblock,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block);
int vaultfs_root_directory_block_load(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    const VAULTFS_SUPERBLOCK *superblock,
    VAULTFS_ROOT_DIRECTORY_BLOCK *root_block);
int vaultfs_root_directory_block_load_dual(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    const VAULTFS_SUPERBLOCK *superblock,
    VAULTFS_ROOT_DIRECTORY_BLOCK *root_block);
int vaultfs_root_directory_block_select(
    const VAULTFS_ROOT_DIRECTORY_BLOCK *primary,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *backup,
    uint64_t generation,
    VAULTFS_ROOT_DIRECTORY_BLOCK *selected);
int vaultfs_runtime_probe(void);

#endif
