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
    uint8_t storage[ATLAS_BLOCK_BYTES * 5U];
    ATLAS_RAM_BLOCK_DEVICE device;
    VAULTFS_SUPERBLOCK primary;
    VAULTFS_SUPERBLOCK backup;
    VAULTFS_SUPERBLOCK selected;
    VAULTFS_JOURNAL_ENTRY journal_entry;
    VAULTFS_JOURNAL_ENTRY loaded_journal_entry;
    VAULTFS_RECOVERY_DECISION recovery_decision;
    VAULTFS_ROOT_UPDATE_PLAN update_plan;
    VAULTFS_ROOT_UPDATE_PLAN invalid_update_plan;
    VAULTFS_DIRECTORY directory;
    VAULTFS_ROOT_DIRECTORY_BLOCK current_root_block;
    VAULTFS_ROOT_DIRECTORY_BLOCK root_block;
    VAULTFS_ROOT_DIRECTORY_BLOCK loaded_root_block;
    VAULTFS_ROOT_DIRECTORY_BLOCK backup_root_block;
    VAULTFS_ROOT_DIRECTORY_BLOCK selected_root_block;
    VAULTFS_DIRECTORY_ENTRY entry = {
        UINT64_C(17), UINT64_C(2), UINT64_C(128), VAULTFS_ENTRY_FILE, "readme.txt"};
    uint8_t superblock_wire[VAULTFS_SUPERBLOCK_WIRE_BYTES];
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
    vaultfs_root_directory_block_initialize(&backup_root_block, &directory, UINT64_C(8));
    vaultfs_root_directory_block_initialize(&current_root_block, &directory, UINT64_C(7));

    vaultfs_superblock_initialize(&primary, UINT64_C(7), 0U, UINT64_C(2), UINT64_C(3), UINT64_C(70));
    vaultfs_superblock_initialize(&backup, UINT64_C(8), 1U, UINT64_C(2), UINT64_C(3), UINT64_C(80));
    if (!expect(vaultfs_root_update_plan_form(&primary, &root_block, UINT64_C(11), &update_plan) &&
                    update_plan.target_root_block == UINT64_C(3) && update_plan.next_generation == UINT64_C(8),
                    "immutable root update plan targets alternate root snapshot") ||
        !expect(!vaultfs_root_update_plan_form(&primary, &root_block, 0U, &update_plan),
                    "root update plan rejects zero transaction identity")) {
        return 1;
    }
    if (!expect(vaultfs_root_update_journal_prepare(&update_plan, &journal_entry) &&
                    journal_entry.state == VAULTFS_JOURNAL_PREPARED &&
                    journal_entry.target_block == update_plan.target_root_block,
                    "root update plan produces sealed prepared journal metadata")) {
        return 1;
    }
    if (!expect(vaultfs_root_update_journal_store_prepared(&device, UINT64_C(4), &update_plan) &&
                    vaultfs_journal_load(&device, UINT64_C(4), &loaded_journal_entry) &&
                    loaded_journal_entry.state == VAULTFS_JOURNAL_PREPARED,
                    "prepared root update journal persists through canonical wire bytes")) {
        return 1;
    }
    if (!expect(vaultfs_superblock_valid(&primary) && primary.root_directory_block == UINT64_C(2),
                    "primary stores sealed root-directory block metadata") ||
        !expect(vaultfs_superblock_store(&device, 0U, &primary), "primary stores") ||
        !expect(atlas_block_read(&device, 0U, superblock_wire, sizeof(superblock_wire)) &&
                    superblock_wire[0] == 0x31U && superblock_wire[1] == 0x53U &&
                    superblock_wire[8] == VAULTFS_FORMAT_VERSION && superblock_wire[12] == 0U &&
                    superblock_wire[40] == 2U && superblock_wire[48] == 3U,
                    "primary uses canonical little-endian superblock wire bytes") ||
        !expect(vaultfs_root_directory_block_valid(&current_root_block),
                    "current root directory block checksum validates") ||
        !expect(vaultfs_root_directory_block_store(&device, &primary, &current_root_block),
                    "current root directory block stores") ||
        !expect(vaultfs_root_directory_backup_block_store(&device, &primary, &current_root_block),
                    "current backup root directory block stores") ||
        !expect(vaultfs_root_directory_block_load_dual(&device, &primary, &loaded_root_block) &&
                    loaded_root_block.generation == UINT64_C(7),
                    "old superblock selects current root before ordered update") ||
        !expect(vaultfs_root_directory_block_valid(&root_block), "root directory block checksum validates") ||
        !expect(vaultfs_root_update_snapshot_store(&device, UINT64_C(4), &primary, &update_plan, &root_block),
                    "prepared journal authorizes alternate next root snapshot store") ||
        !expect(vaultfs_root_directory_block_load_dual(&device, &primary, &loaded_root_block) &&
                    loaded_root_block.generation == UINT64_C(7),
                    "old superblock remains current after alternate snapshot store")) {
        return 1;
    }

    invalid_update_plan = update_plan;
    invalid_update_plan.target_root_block = primary.root_directory_block;
    if (!expect(!vaultfs_root_update_snapshot_store(
                    &device, UINT64_C(4), &primary, &invalid_update_plan, &root_block),
                    "snapshot store rejects plan target outside alternate root") ||
        !expect(!vaultfs_root_update_snapshot_store(&device, primary.root_directory_block, &primary, &update_plan,
                                                     &root_block),
                    "snapshot store rejects journal location overlapping a root snapshot") ||
        !expect(!vaultfs_root_update_snapshot_store(&device, UINT64_C(4), &primary, &update_plan,
                                                     &current_root_block),
                    "snapshot store rejects next root with wrong generation")) {
        return 1;
    }

    loaded_journal_entry.checksum ^= UINT32_C(1);
    if (!expect(atlas_block_write(&device, UINT64_C(4), &loaded_journal_entry, sizeof(loaded_journal_entry)) &&
                    !vaultfs_root_update_snapshot_store(&device, UINT64_C(4), &primary, &update_plan, &root_block) &&
                    !vaultfs_root_update_journal_store_committed(&device, UINT64_C(4), &primary, &update_plan) &&
                    vaultfs_root_update_journal_store_prepared(&device, UINT64_C(4), &update_plan),
                    "snapshot store rejects malformed persisted prepared journal")) {
        return 1;
    }

    vaultfs_journal_prepare(&journal_entry, update_plan.transaction_id, primary.root_directory_block,
                            update_plan.payload_checksum);
    if (!expect(vaultfs_journal_store(&device, UINT64_C(4), &journal_entry) &&
                    !vaultfs_root_update_journal_store_committed(&device, UINT64_C(4), &primary, &update_plan) &&
                    vaultfs_root_update_journal_store_prepared(&device, UINT64_C(4), &update_plan) &&
                    vaultfs_root_update_journal_store_committed(&device, UINT64_C(4), &primary, &update_plan) &&
                    vaultfs_journal_load(&device, UINT64_C(4), &loaded_journal_entry) &&
                    loaded_journal_entry.state == VAULTFS_JOURNAL_COMMITTED &&
                    !vaultfs_recovery_decide(&primary, &loaded_journal_entry, &root_block, &recovery_decision),
                    "commit requires matching persisted snapshot but does not promote old superblock")) {
        return 1;
    }

    if (!expect(vaultfs_superblock_store(&device, 1U, &backup), "backup stores") ||
        !expect(vaultfs_root_directory_block_load_dual(&device, &backup, &loaded_root_block) &&
                    loaded_root_block.generation == UINT64_C(8) && loaded_root_block.entry_count == 1U &&
                    loaded_root_block.entries[0].object_id == UINT64_C(17),
                    "promoted superblock loads alternate next root snapshot") ||
        !expect(vaultfs_root_directory_block_load_dual(&device, &backup, &loaded_root_block) &&
                    loaded_root_block.generation == UINT64_C(8),
                    "dual root directory load selects a matching media snapshot") ||
        !expect(vaultfs_root_directory_block_select(
                    &root_block, &backup_root_block, UINT64_C(8), &selected_root_block) &&
                    selected_root_block.generation == UINT64_C(8),
                    "dual root snapshot selector retains a matching valid snapshot") ||
        !expect(vaultfs_root_directory_block_load(&device, &primary, &loaded_root_block) &&
                    loaded_root_block.generation == UINT64_C(7),
                    "primary-only loader retains old root before promotion") ||
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

    vaultfs_superblock_initialize(&selected, UINT64_C(8), 1U, UINT64_C(2), UINT64_C(3), UINT64_C(80));
    vaultfs_journal_prepare(&journal_entry, UINT64_C(9), UINT64_C(2), root_block.checksum);
    if (!expect(vaultfs_journal_valid(&journal_entry), "prepared journal entry validates") ||
        !expect(vaultfs_recovery_decide(&selected, &journal_entry, &root_block, &recovery_decision) &&
                    recovery_decision == VAULTFS_RECOVERY_DISCARD_PREPARED,
                    "prepared matching journal decision discards incomplete operation") ||
        !expect(vaultfs_journal_commit(&journal_entry), "prepared entry commits") ||
        !expect(journal_entry.state == VAULTFS_JOURNAL_COMMITTED && vaultfs_journal_valid(&journal_entry),
                    "committed entry is resealed") ||
        !expect(vaultfs_recovery_decide(&selected, &journal_entry, &root_block, &recovery_decision) &&
                    recovery_decision == VAULTFS_RECOVERY_ACCEPT_COMMITTED,
                    "committed matching journal decision accepts completed operation") ||
        !expect(vaultfs_journal_store(&device, UINT64_C(4), &journal_entry) &&
                    vaultfs_journal_load(&device, UINT64_C(4), &loaded_journal_entry) &&
                    loaded_journal_entry.transaction_id == UINT64_C(9) &&
                    loaded_journal_entry.state == VAULTFS_JOURNAL_COMMITTED,
                    "committed journal entry round-trips through canonical wire bytes") ||
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
