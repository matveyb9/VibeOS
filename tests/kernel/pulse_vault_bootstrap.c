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

    if (!expect(!atlas_ram_block_device_init(&device, storage, ATLAS_BLOCK_BYTES - 1U),
                    "unaligned block device size is rejected") ||
        !expect(atlas_ram_block_device_init(&device, storage, sizeof(storage)),
                    "aligned block device initializes")) {
        return 1;
    }

    vaultfs_superblock_initialize(&primary, UINT64_C(7), 0U, UINT64_C(70));
    vaultfs_superblock_initialize(&backup, UINT64_C(8), 1U, UINT64_C(80));
    if (!expect(vaultfs_superblock_valid(&primary), "primary checksum validates") ||
        !expect(vaultfs_superblock_store(&device, 0U, &primary), "primary stores") ||
        !expect(vaultfs_superblock_store(&device, 1U, &backup), "backup stores") ||
        !expect(vaultfs_superblock_load_latest(&device, 0U, 1U, &selected) &&
                    selected.generation == UINT64_C(8),
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

    puts("Atlas and VaultFS bootstrap unit tests passed.");
    return 0;
}
