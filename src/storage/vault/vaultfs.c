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

static void vaultfs_superblock_encode(const VAULTFS_SUPERBLOCK *superblock, uint8_t *bytes);

static uint32_t vaultfs_superblock_checksum(const VAULTFS_SUPERBLOCK *superblock) {
    uint8_t bytes[VAULTFS_SUPERBLOCK_WIRE_BYTES];

    vaultfs_superblock_encode(superblock, bytes);
    return vaultfs_crc32(bytes, VAULTFS_SUPERBLOCK_WIRE_BYTES - sizeof(superblock->checksum));
}

static void vaultfs_superblock_seal(VAULTFS_SUPERBLOCK *superblock) {
    superblock->checksum = vaultfs_superblock_checksum(superblock);
}

static int vaultfs_system_slot_valid(uint64_t slot) {
    return slot == 0U || slot == 1U;
}

static int vaultfs_entry_name_valid(const char *name) {
    uint32_t index;
    char character;

    if (name == (const void *)0 || name[0] == '\0') {
        return 0;
    }
    for (index = 0U; index < VAULTFS_ENTRY_NAME_BYTES; ++index) {
        character = name[index];
        if (character == '\0') {
            return 1;
        }
        if (!((character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') ||
              character == '.' || character == '-' || character == '_')) {
            return 0;
        }
    }
    return 0;
}

static int vaultfs_entry_name_equal(const char *left, const char *right) {
    uint32_t index;

    for (index = 0U; index < VAULTFS_ENTRY_NAME_BYTES; ++index) {
        if (left[index] != right[index]) {
            return 0;
        }
        if (left[index] == '\0') {
            return 1;
        }
    }
    return 0;
}

static void vaultfs_journal_encode(const VAULTFS_JOURNAL_ENTRY *entry, uint8_t *bytes);

static uint32_t vaultfs_journal_checksum(const VAULTFS_JOURNAL_ENTRY *entry) {
    uint8_t bytes[VAULTFS_JOURNAL_WIRE_BYTES];

    vaultfs_journal_encode(entry, bytes);
    return vaultfs_crc32(bytes, VAULTFS_JOURNAL_WIRE_BYTES - sizeof(entry->checksum));
}

static void vaultfs_write_u32_le(uint8_t *bytes, uint32_t value) {
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8U);
    bytes[2] = (uint8_t)(value >> 16U);
    bytes[3] = (uint8_t)(value >> 24U);
}

static void vaultfs_write_u64_le(uint8_t *bytes, uint64_t value) {
    uint32_t index;

    for (index = 0U; index < 8U; ++index) {
        bytes[index] = (uint8_t)(value >> (index * 8U));
    }
}

static uint32_t vaultfs_read_u32_le(const uint8_t *bytes) {
    return (uint32_t)((uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8U) |
                      ((uint32_t)bytes[2] << 16U) | ((uint32_t)bytes[3] << 24U));
}

static uint64_t vaultfs_read_u64_le(const uint8_t *bytes) {
    uint64_t value = 0U;
    uint32_t index;

    for (index = 0U; index < 8U; ++index) {
        value |= ((uint64_t)bytes[index]) << (index * 8U);
    }
    return value;
}

static void vaultfs_superblock_encode(const VAULTFS_SUPERBLOCK *superblock, uint8_t *bytes) {
    uint32_t index;

    for (index = 0U; index < VAULTFS_SUPERBLOCK_WIRE_BYTES; ++index) {
        bytes[index] = 0U;
    }
    vaultfs_write_u64_le(&bytes[0], superblock->magic);
    vaultfs_write_u32_le(&bytes[8], superblock->format_version);
    vaultfs_write_u32_le(&bytes[12], superblock->block_bytes);
    vaultfs_write_u64_le(&bytes[16], superblock->generation);
    vaultfs_write_u64_le(&bytes[24], superblock->active_system_slot);
    vaultfs_write_u64_le(&bytes[32], superblock->pending_system_slot);
    vaultfs_write_u64_le(&bytes[40], superblock->root_directory_block);
    vaultfs_write_u64_le(&bytes[48], superblock->backup_root_directory_block);
    vaultfs_write_u64_le(&bytes[56], superblock->journal_sequence);
    vaultfs_write_u32_le(&bytes[64], superblock->checksum);
}

static void vaultfs_superblock_decode(VAULTFS_SUPERBLOCK *superblock, const uint8_t *bytes) {
    uint8_t *superblock_bytes = (uint8_t *)superblock;
    uint32_t index;

    for (index = 0U; index < sizeof(*superblock); ++index) {
        superblock_bytes[index] = 0U;
    }
    superblock->magic = vaultfs_read_u64_le(&bytes[0]);
    superblock->format_version = vaultfs_read_u32_le(&bytes[8]);
    superblock->block_bytes = vaultfs_read_u32_le(&bytes[12]);
    superblock->generation = vaultfs_read_u64_le(&bytes[16]);
    superblock->active_system_slot = vaultfs_read_u64_le(&bytes[24]);
    superblock->pending_system_slot = vaultfs_read_u64_le(&bytes[32]);
    superblock->root_directory_block = vaultfs_read_u64_le(&bytes[40]);
    superblock->backup_root_directory_block = vaultfs_read_u64_le(&bytes[48]);
    superblock->journal_sequence = vaultfs_read_u64_le(&bytes[56]);
    superblock->checksum = vaultfs_read_u32_le(&bytes[64]);
}

static void vaultfs_journal_encode(const VAULTFS_JOURNAL_ENTRY *entry, uint8_t *bytes) {
    vaultfs_write_u64_le(&bytes[0], entry->magic);
    vaultfs_write_u64_le(&bytes[8], entry->transaction_id);
    vaultfs_write_u64_le(&bytes[16], entry->target_block);
    vaultfs_write_u32_le(&bytes[24], entry->payload_checksum);
    vaultfs_write_u32_le(&bytes[28], entry->state);
    vaultfs_write_u32_le(&bytes[32], entry->checksum);
}

static void vaultfs_journal_decode(VAULTFS_JOURNAL_ENTRY *entry, const uint8_t *bytes) {
    uint8_t *entry_bytes = (uint8_t *)entry;
    uint32_t index;

    for (index = 0U; index < sizeof(*entry); ++index) {
        entry_bytes[index] = 0U;
    }
    entry->magic = vaultfs_read_u64_le(&bytes[0]);
    entry->transaction_id = vaultfs_read_u64_le(&bytes[8]);
    entry->target_block = vaultfs_read_u64_le(&bytes[16]);
    entry->payload_checksum = vaultfs_read_u32_le(&bytes[24]);
    entry->state = vaultfs_read_u32_le(&bytes[28]);
    entry->checksum = vaultfs_read_u32_le(&bytes[32]);
}

static void vaultfs_root_directory_encode(const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block, uint8_t *bytes) {
    uint32_t index;
    uint32_t byte_index;

    for (index = 0U; index < VAULTFS_ROOT_DIRECTORY_WIRE_BYTES; ++index) {
        bytes[index] = 0U;
    }
    vaultfs_write_u64_le(&bytes[0], root_block->magic);
    vaultfs_write_u32_le(&bytes[8], root_block->format_version);
    vaultfs_write_u32_le(&bytes[12], root_block->entry_count);
    vaultfs_write_u64_le(&bytes[16], root_block->generation);
    for (index = 0U; index < VAULTFS_DIRECTORY_CAPACITY; ++index) {
        uint8_t *entry_bytes = &bytes[24U + index * 60U];
        const VAULTFS_DIRECTORY_ENTRY *entry = &root_block->entries[index];

        vaultfs_write_u64_le(&entry_bytes[0], entry->object_id);
        vaultfs_write_u64_le(&entry_bytes[8], entry->first_block);
        vaultfs_write_u64_le(&entry_bytes[16], entry->byte_count);
        vaultfs_write_u32_le(&entry_bytes[24], entry->kind);
        for (byte_index = 0U; byte_index < VAULTFS_ENTRY_NAME_BYTES; ++byte_index) {
            entry_bytes[28U + byte_index] = (uint8_t)entry->name[byte_index];
        }
    }
    vaultfs_write_u32_le(&bytes[984], root_block->checksum);
}

static void vaultfs_root_directory_decode(VAULTFS_ROOT_DIRECTORY_BLOCK *root_block, const uint8_t *bytes) {
    uint8_t *root_bytes = (uint8_t *)root_block;
    uint32_t index;
    uint32_t byte_index;

    for (index = 0U; index < sizeof(*root_block); ++index) {
        root_bytes[index] = 0U;
    }
    root_block->magic = vaultfs_read_u64_le(&bytes[0]);
    root_block->format_version = vaultfs_read_u32_le(&bytes[8]);
    root_block->entry_count = vaultfs_read_u32_le(&bytes[12]);
    root_block->generation = vaultfs_read_u64_le(&bytes[16]);
    for (index = 0U; index < VAULTFS_DIRECTORY_CAPACITY; ++index) {
        const uint8_t *entry_bytes = &bytes[24U + index * 60U];
        VAULTFS_DIRECTORY_ENTRY *entry = &root_block->entries[index];

        entry->object_id = vaultfs_read_u64_le(&entry_bytes[0]);
        entry->first_block = vaultfs_read_u64_le(&entry_bytes[8]);
        entry->byte_count = vaultfs_read_u64_le(&entry_bytes[16]);
        entry->kind = vaultfs_read_u32_le(&entry_bytes[24]);
        for (byte_index = 0U; byte_index < VAULTFS_ENTRY_NAME_BYTES; ++byte_index) {
            entry->name[byte_index] = (char)entry_bytes[28U + byte_index];
        }
    }
    root_block->checksum = vaultfs_read_u32_le(&bytes[984]);
}

static uint32_t vaultfs_root_directory_checksum(const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block) {
    uint8_t bytes[VAULTFS_ROOT_DIRECTORY_WIRE_BYTES];

    vaultfs_root_directory_encode(root_block, bytes);
    return vaultfs_crc32(bytes, VAULTFS_ROOT_DIRECTORY_WIRE_BYTES - sizeof(root_block->checksum));
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
    uint64_t root_directory_block,
    uint64_t backup_root_directory_block,
    uint64_t journal_sequence) {
    if (superblock != (void *)0) {
        superblock->magic = VAULTFS_SUPERBLOCK_MAGIC;
        superblock->format_version = VAULTFS_FORMAT_VERSION;
        superblock->block_bytes = (uint32_t)ATLAS_BLOCK_BYTES;
        superblock->generation = generation;
        superblock->active_system_slot = active_system_slot;
        superblock->pending_system_slot = VAULTFS_SYSTEM_SLOT_NONE;
        superblock->root_directory_block = root_directory_block;
        superblock->backup_root_directory_block = backup_root_directory_block;
        superblock->journal_sequence = journal_sequence;
        vaultfs_superblock_seal(superblock);
    }
}

int vaultfs_superblock_valid(const VAULTFS_SUPERBLOCK *superblock) {
    return superblock != (void *)0 && superblock->magic == VAULTFS_SUPERBLOCK_MAGIC &&
           superblock->format_version == VAULTFS_FORMAT_VERSION &&
           superblock->block_bytes == ATLAS_BLOCK_BYTES && vaultfs_system_slot_valid(superblock->active_system_slot) &&
           superblock->root_directory_block != VAULTFS_ROOT_DIRECTORY_BLOCK_NONE &&
           superblock->backup_root_directory_block != VAULTFS_ROOT_DIRECTORY_BLOCK_NONE &&
           superblock->root_directory_block != superblock->backup_root_directory_block &&
           (superblock->pending_system_slot == VAULTFS_SYSTEM_SLOT_NONE ||
            vaultfs_system_slot_valid(superblock->pending_system_slot)) &&
           superblock->checksum == vaultfs_superblock_checksum(superblock);
}

int vaultfs_superblock_store(
    ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t block_index,
    const VAULTFS_SUPERBLOCK *superblock) {
    uint8_t bytes[VAULTFS_SUPERBLOCK_WIRE_BYTES];

    if (!vaultfs_superblock_valid(superblock)) {
        return 0;
    }
    vaultfs_superblock_encode(superblock, bytes);
    return atlas_block_write(device, block_index, bytes, sizeof(bytes));
}

int vaultfs_superblock_load_latest(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    uint64_t primary_block,
    uint64_t backup_block,
    VAULTFS_SUPERBLOCK *superblock) {
    uint8_t primary_bytes[VAULTFS_SUPERBLOCK_WIRE_BYTES];
    uint8_t backup_bytes[VAULTFS_SUPERBLOCK_WIRE_BYTES];
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    int primary_valid;
    int backup_valid;

    if (superblock == (void *)0 ||
        !atlas_block_read(device, primary_block, primary_bytes, sizeof(primary_bytes)) ||
        !atlas_block_read(device, backup_block, backup_bytes, sizeof(backup_bytes))) {
        return 0;
    }
    vaultfs_superblock_decode(&primary, primary_bytes);
    vaultfs_superblock_decode(&backup, backup_bytes);

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

int vaultfs_journal_store(
    ATLAS_RAM_BLOCK_DEVICE *device, uint64_t block_index, const VAULTFS_JOURNAL_ENTRY *entry) {
    uint8_t bytes[VAULTFS_JOURNAL_WIRE_BYTES];

    if (!vaultfs_journal_valid(entry)) {
        return 0;
    }
    vaultfs_journal_encode(entry, bytes);
    return atlas_block_write(device, block_index, bytes, sizeof(bytes));
}

int vaultfs_journal_load(
    const ATLAS_RAM_BLOCK_DEVICE *device, uint64_t block_index, VAULTFS_JOURNAL_ENTRY *entry) {
    uint8_t bytes[VAULTFS_JOURNAL_WIRE_BYTES];

    if (entry == (void *)0 || !atlas_block_read(device, block_index, bytes, sizeof(bytes))) {
        return 0;
    }
    vaultfs_journal_decode(entry, bytes);
    return vaultfs_journal_valid(entry);
}

int vaultfs_system_slot_stage(VAULTFS_SUPERBLOCK *superblock, uint64_t target_slot) {
    if (!vaultfs_superblock_valid(superblock) || !vaultfs_system_slot_valid(target_slot) ||
        target_slot == superblock->active_system_slot) {
        return 0;
    }
    superblock->pending_system_slot = target_slot;
    vaultfs_superblock_seal(superblock);
    return 1;
}

int vaultfs_system_slot_confirm(VAULTFS_SUPERBLOCK *superblock) {
    if (!vaultfs_superblock_valid(superblock) ||
        !vaultfs_system_slot_valid(superblock->pending_system_slot)) {
        return 0;
    }
    superblock->active_system_slot = superblock->pending_system_slot;
    superblock->pending_system_slot = VAULTFS_SYSTEM_SLOT_NONE;
    ++superblock->generation;
    vaultfs_superblock_seal(superblock);
    return 1;
}

int vaultfs_system_slot_recover(const VAULTFS_SUPERBLOCK *superblock, uint64_t *boot_slot) {
    if (!vaultfs_superblock_valid(superblock) || boot_slot == (void *)0) {
        return 0;
    }
    *boot_slot = superblock->active_system_slot;
    return 1;
}

void vaultfs_directory_initialize(VAULTFS_DIRECTORY *directory) {
    uint8_t *bytes = (uint8_t *)directory;
    uint64_t index;

    if (directory == (void *)0) {
        return;
    }
    for (index = 0U; index < sizeof(*directory); ++index) {
        bytes[index] = 0U;
    }
}

int vaultfs_directory_entry_valid(const VAULTFS_DIRECTORY_ENTRY *entry) {
    return entry != (const void *)0 && entry->object_id != 0U && entry->first_block != UINT64_MAX &&
           (entry->kind == VAULTFS_ENTRY_FILE || entry->kind == VAULTFS_ENTRY_DIRECTORY) &&
           vaultfs_entry_name_valid(entry->name);
}

int vaultfs_directory_insert(VAULTFS_DIRECTORY *directory, const VAULTFS_DIRECTORY_ENTRY *entry) {
    uint32_t index;

    if (directory == (void *)0 || !vaultfs_directory_entry_valid(entry) ||
        directory->count >= VAULTFS_DIRECTORY_CAPACITY) {
        return 0;
    }
    if (vaultfs_directory_find(directory, entry->name) != (const void *)0) {
        return 0;
    }
    for (index = 0U; index < sizeof(*entry); ++index) {
        ((uint8_t *)&directory->entries[directory->count])[index] = ((const uint8_t *)entry)[index];
    }
    ++directory->count;
    return 1;
}

const VAULTFS_DIRECTORY_ENTRY *vaultfs_directory_find(
    const VAULTFS_DIRECTORY *directory, const char *name) {
    uint32_t index;

    if (directory == (const void *)0 || !vaultfs_entry_name_valid(name)) {
        return (const void *)0;
    }
    for (index = 0U; index < directory->count; ++index) {
        if (vaultfs_entry_name_equal(directory->entries[index].name, name)) {
            return &directory->entries[index];
        }
    }
    return (const void *)0;
}

void vaultfs_root_directory_block_initialize(
    VAULTFS_ROOT_DIRECTORY_BLOCK *root_block,
    const VAULTFS_DIRECTORY *directory,
    uint64_t generation) {
    uint8_t *bytes = (uint8_t *)root_block;
    uint32_t index;

    if (root_block == (void *)0 || directory == (const void *)0 || directory->count > VAULTFS_DIRECTORY_CAPACITY) {
        return;
    }
    for (index = 0U; index < sizeof(*root_block); ++index) {
        bytes[index] = 0U;
    }
    root_block->magic = VAULTFS_ROOT_DIRECTORY_MAGIC;
    root_block->format_version = VAULTFS_ROOT_DIRECTORY_VERSION;
    root_block->entry_count = directory->count;
    root_block->generation = generation;
    for (index = 0U; index < directory->count; ++index) {
        uint32_t byte_index;
        for (byte_index = 0U; byte_index < sizeof(VAULTFS_DIRECTORY_ENTRY); ++byte_index) {
            ((uint8_t *)&root_block->entries[index])[byte_index] = ((const uint8_t *)&directory->entries[index])[byte_index];
        }
    }
    root_block->checksum = vaultfs_root_directory_checksum(root_block);
}

int vaultfs_root_directory_block_valid(const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block) {
    uint32_t index;
    uint32_t other_index;

    if (root_block == (const void *)0 || root_block->magic != VAULTFS_ROOT_DIRECTORY_MAGIC ||
        root_block->format_version != VAULTFS_ROOT_DIRECTORY_VERSION ||
        root_block->entry_count > VAULTFS_DIRECTORY_CAPACITY ||
        root_block->checksum != vaultfs_root_directory_checksum(root_block)) {
        return 0;
    }
    for (index = 0U; index < root_block->entry_count; ++index) {
        if (!vaultfs_directory_entry_valid(&root_block->entries[index])) {
            return 0;
        }
        for (other_index = 0U; other_index < index; ++other_index) {
            if (vaultfs_entry_name_equal(root_block->entries[index].name, root_block->entries[other_index].name)) {
                return 0;
            }
        }
    }
    return 1;
}

int vaultfs_root_directory_block_store(
    ATLAS_RAM_BLOCK_DEVICE *device,
    const VAULTFS_SUPERBLOCK *superblock,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *root_block) {
    uint8_t bytes[VAULTFS_ROOT_DIRECTORY_WIRE_BYTES];

    if (!vaultfs_superblock_valid(superblock) || !vaultfs_root_directory_block_valid(root_block)) {
        return 0;
    }
    vaultfs_root_directory_encode(root_block, bytes);
    return atlas_block_write(device, superblock->root_directory_block, bytes, sizeof(bytes));
}

int vaultfs_root_directory_block_load(
    const ATLAS_RAM_BLOCK_DEVICE *device,
    const VAULTFS_SUPERBLOCK *superblock,
    VAULTFS_ROOT_DIRECTORY_BLOCK *root_block) {
    uint8_t bytes[VAULTFS_ROOT_DIRECTORY_WIRE_BYTES];

    if (!vaultfs_superblock_valid(superblock) || root_block == (void *)0 ||
        !atlas_block_read(device, superblock->root_directory_block, bytes, sizeof(bytes))) {
        return 0;
    }
    vaultfs_root_directory_decode(root_block, bytes);
    return vaultfs_root_directory_block_valid(root_block) && root_block->generation == superblock->generation;
}

int vaultfs_root_directory_block_select(
    const VAULTFS_ROOT_DIRECTORY_BLOCK *primary,
    const VAULTFS_ROOT_DIRECTORY_BLOCK *backup,
    uint64_t generation,
    VAULTFS_ROOT_DIRECTORY_BLOCK *selected) {
    const VAULTFS_ROOT_DIRECTORY_BLOCK *source;
    uint32_t index;

    if (selected == (void *)0) {
        return 0;
    }
    source = primary != (const void *)0 && vaultfs_root_directory_block_valid(primary) &&
                     primary->generation == generation
                 ? primary
                 : backup != (const void *)0 && vaultfs_root_directory_block_valid(backup) &&
                       backup->generation == generation
                     ? backup
                     : (const void *)0;
    if (source == (const void *)0) {
        return 0;
    }
    for (index = 0U; index < sizeof(*selected); ++index) {
        ((uint8_t *)selected)[index] = ((const uint8_t *)source)[index];
    }
    return 1;
}

int vaultfs_runtime_probe(void) {
    static uint8_t storage[ATLAS_BLOCK_BYTES * 4U];
    ATLAS_RAM_BLOCK_DEVICE device;
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    VAULTFS_SUPERBLOCK recovered;
    VAULTFS_JOURNAL_ENTRY journal_entry;
    uint64_t boot_slot;

    if (!atlas_ram_block_device_init(&device, storage, sizeof(storage))) {
        return 0;
    }
    vaultfs_superblock_initialize(&primary, UINT64_C(5), 0U, UINT64_C(2), UINT64_C(3), UINT64_C(50));
    vaultfs_superblock_initialize(&backup, UINT64_C(4), 1U, UINT64_C(2), UINT64_C(3), UINT64_C(40));
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
           vaultfs_journal_valid(&journal_entry) && journal_entry.state == VAULTFS_JOURNAL_COMMITTED &&
           vaultfs_system_slot_stage(&recovered, 0U) && vaultfs_system_slot_recover(&recovered, &boot_slot) &&
           boot_slot == 1U && vaultfs_system_slot_confirm(&recovered) &&
           vaultfs_system_slot_recover(&recovered, &boot_slot) && boot_slot == 0U;
}
