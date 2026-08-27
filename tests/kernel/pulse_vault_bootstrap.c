/* VibeOS VaultFS — host tests for superblock selection and recovery. */

#include <stdint.h>
#include <stdio.h>

#include <atlas_block.h>
#include <vaultfs.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    uint8_t storage[ATLAS_BLOCK_BYTES * 3U];
    ATLAS_RAM_BLOCK_DEVICE device;
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    VAULTFS_SUPERBLOCK selected;
    VAULTFS_JOURNAL_ENTRY journal_entry;
    VAULTFS_DIRECTORY directory;
    VAULTFS_ROOT_DIRECTORY_BLOCK root_block;
    VAULTFS_ROOT_DIRECTORY_BLOCK loaded_root_block;
    VAULTFS_DIRECTORY_ENTRY entry = {
        UINT64_C(17), UINT64_C(2), UINT64_C(128), VAULTFS_ENTRY_FILE, "readme.txt"};
    uint64_t boot_slot;

    if (!expect(!atlas_ram_block_device_init(&device, storage, ATLAS_BLOCK_BYTES - 1U),
                    "unaligned block device size is rejected") ||
        !expect(atlas_ram_block_device_init(&device, storage, sizeof(storage)),
                    "aligned block device initializes")) {
        return 1;
    }

    vaultfs_directory_initialize(&directory);
    if (!expect(vaultfs_directory_entry_valid(&entry), "bounded directory entry validates") ||
        !expect(vaultfs_directory_insert(&directory, &entry), "valid directory entry inserts") ||
        !expect(directory.count == 1U && vaultfs_directory_find(&directory, "readme.txt") != (const void *)0 &&
                    vaultfs_directory_find(&directory, "readme.txt")->object_id == UINT64_C(17),
                    "read-only directory lookup returns inserted entry") ||
        !expect(!vaultfs_directory_insert(&directory, &entry), "duplicate directory name is rejected")) {
        return 1;
    }
    entry.object_id = 0U;
    if (!expect(!vaultfs_directory_entry_valid(&entry), "zero directory object identity is rejected") ||
        !expect(vaultfs_directory_find(&directory, "bad/name") == (const void *)0,
                    "invalid directory lookup name is rejected")) {
        return 1;
    }
    vaultfs_root_directory_block_initialize(&root_block, &directory, UINT64_C(8));

    vaultfs_superblock_initialize(&primary, UINT64_C(7), 0U, UINT64_C(2), UINT64_C(70));
    vaultfs_superblock_initialize(&backup, UINT64_C(8), 1U, UINT64_C(2), UINT64_C(80));
    if (!expect(vaultfs_superblock_valid(&primary) && primary.root_directory_block == UINT64_C(2),
                    "primary stores sealed root-directory block metadata") ||
        !expect(vaultfs_superblock_store(&device, 0U, &primary), "primary stores") ||
        !expect(vaultfs_superblock_store(&device, 1U, &backup), "backup stores") ||
        !expect(vaultfs_root_directory_block_valid(&root_block), "root directory block checksum validates") ||
        !expect(vaultfs_root_directory_block_store(&device, &backup, &root_block), "root directory block stores") ||
        !expect(vaultfs_root_directory_block_load(&device, &backup, &loaded_root_block) &&
                    loaded_root_block.generation == UINT64_C(8) && loaded_root_block.entry_count == 1U &&
                    loaded_root_block.entries[0].object_id == UINT64_C(17),
                    "root directory block loads through sealed superblock reference") ||
        !expect(!vaultfs_root_directory_block_load(&device, &primary, &loaded_root_block),
                    "root directory block generation mismatch is rejected") ||
        !expect(vaultfs_superblock_load_latest(&device, 0U, 1U, &selected) &&
                    selected.generation == UINT64_C(8) && selected.root_directory_block == UINT64_C(2),
                    "newer valid superblock is selected")) {
        return 1;
    }

    backup.checksum ^= UINT64_C(1);
    if (!expect(atlas_block_write(&device, 1U, &backup, sizeof(backup)), "corrupted backup writes") ||
        !expect(vaultfs_superblock_load_latest(&device, 0U, 1U, &selected) &&
                    selected.generation == UINT64_C(7),
                    "valid primary recovers from bad backup") ||
        !expect(vaultfs_runtime_probe(), "runtime probe recovers from bad primary")) {
        return 1;
    }

    vaultfs_journal_prepare(&journal_entry, UINT64_C(9), UINT64_C(2), UINT32_C(0x10203040));
    if (!expect(vaultfs_journal_valid(&journal_entry), "prepared journal entry validates") ||
        !expect(vaultfs_journal_commit(&journal_entry), "prepared entry commits") ||
        !expect(journal_entry.state == VAULTFS_JOURNAL_COMMITTED && vaultfs_journal_valid(&journal_entry),
                    "committed entry is resealed") ||
        !expect(!vaultfs_journal_commit(&journal_entry), "committed entry cannot commit twice")) {
        return 1;
    }

    journal_entry.target_block ^= UINT64_C(1);
    if (!expect(!vaultfs_journal_valid(&journal_entry), "tampered journal entry is rejected")) {
        return 1;
    }

    if (!expect(vaultfs_system_slot_stage(&primary, 1U), "alternate system slot stages") ||
        !expect(vaultfs_system_slot_recover(&primary, &boot_slot) && boot_slot == 0U,
                    "unconfirmed slot preserves active boot slot") ||
        !expect(vaultfs_system_slot_confirm(&primary), "staged system slot confirms") ||
        !expect(vaultfs_system_slot_recover(&primary, &boot_slot) && boot_slot == 1U,
                    "confirmed slot becomes active") ||
        !expect(!vaultfs_system_slot_stage(&primary, 1U), "active slot cannot stage to itself")) {
        return 1;
    }

    primary.root_directory_block = VAULTFS_ROOT_DIRECTORY_BLOCK_NONE;
    if (!expect(!vaultfs_superblock_valid(&primary), "missing root-directory block is rejected")) {
        return 1;
    }

    puts("Atlas and VaultFS bootstrap unit tests passed.");
    return 0;
}
