/* VibeOS VaultFS — initial redundant-superblock recovery implementation. */

#include "vaultfs.h"

uint32_t vaultfs_crc32(const void *data, uint64_t byte_count) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint32_t value = UINT32_C(0xffffffff);
    uint64_t index;
    uint32_t bit;

    for (index = 0; index < byte_count; ++index) {
        value ^= bytes[index];
        for (bit = 0; bit < 8U; ++bit) {
            value = (value >> 1U) ^ ((value & 1U) != 0U ? UINT32_C(0xedb88320) : 0U);
        }
    }
    return ~value;
}

static uint32_t vaultfs_superblock_checksum(const VAULTFS_SUPERBLOCK *superblock) {
    return vaultfs_crc32(superblock, sizeof(VAULTFS_SUPERBLOCK) - sizeof(superblock->checksum));
}

static uint32_t vaultfs_journal_checksum(const VAULTFS_JOURNAL_ENTRY *entry) {
    return vaultfs_crc32(entry, sizeof(VAULTFS_JOURNAL_ENTRY) - sizeof(entry->checksum));
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
        superblock->checksum = vaultfs_superblock_checksum(superblock);
    }
}

int vaultfs_superblock_valid(const VAULTFS_SUPERBLOCK *superblock) {
    return superblock != (void *)0 && superblock->magic == VAULTFS_SUPERBLOCK_MAGIC &&
           superblock->format_version == VAULTFS_FORMAT_VERSION &&
           superblock->block_bytes == ATLAS_BLOCK_BYTES &&
           superblock->checksum == vaultfs_superblock_checksum(superblock);
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

void vaultfs_journal_prepare(
    VAULTFS_JOURNAL_ENTRY *entry,
    uint64_t transaction_id,
    uint64_t target_block,
    uint32_t payload_checksum) {
    if (entry != (void *)0) {
        entry->magic = VAULTFS_JOURNAL_MAGIC;
        entry->transaction_id = transaction_id;
        entry->target_block = target_block;
        entry->payload_checksum = payload_checksum;
        entry->state = VAULTFS_JOURNAL_PREPARED;
        entry->checksum = vaultfs_journal_checksum(entry);
    }
}

int vaultfs_journal_valid(const VAULTFS_JOURNAL_ENTRY *entry) {
    return entry != (void *)0 && entry->magic == VAULTFS_JOURNAL_MAGIC &&
           (entry->state == VAULTFS_JOURNAL_PREPARED || entry->state == VAULTFS_JOURNAL_COMMITTED) &&
           entry->checksum == vaultfs_journal_checksum(entry);
}

int vaultfs_journal_commit(VAULTFS_JOURNAL_ENTRY *entry) {
    if (!vaultfs_journal_valid(entry) || entry->state != VAULTFS_JOURNAL_PREPARED) {
        return 0;
    }
    entry->state = VAULTFS_JOURNAL_COMMITTED;
    entry->checksum = vaultfs_journal_checksum(entry);
    return 1;
}

int vaultfs_runtime_probe(void) {
    static uint8_t storage[ATLAS_BLOCK_BYTES * 4U];
    ATLAS_RAM_BLOCK_DEVICE device;
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    VAULTFS_SUPERBLOCK recovered;
    VAULTFS_JOURNAL_ENTRY journal_entry;

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
    vaultfs_journal_prepare(&journal_entry, UINT64_C(51), UINT64_C(3), UINT32_C(0x12345678));
    return recovered.generation == UINT64_C(4) && recovered.active_system_slot == 1U &&
           vaultfs_journal_valid(&journal_entry) && vaultfs_journal_commit(&journal_entry) &&
           vaultfs_journal_valid(&journal_entry) && journal_entry.state == VAULTFS_JOURNAL_COMMITTED;
}
