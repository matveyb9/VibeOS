<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/VAULTFS_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas block and VaultFS bootstrap

**Status:** Implemented as an in-memory Atlas backend and a verified VaultFS redundant-superblock foundation.

Atlas now defines the independent block-device boundary consumed by storage code. Its initial RAM backend has fixed 4 KiB blocks and exists solely to verify the storage interface before hardware discovery and drivers are ready. VaultFS uses that interface to store two checksummed superblocks. During recovery it validates both copies and selects the highest valid generation; if one copy is corrupted, the other remains bootable metadata.

| Component | Initial behavior |
|---|---|
| Atlas backend | Bounded RAM block device with 4 KiB blocks |
| VaultFS metadata | Format `2` primary and backup superblocks |
| Integrity | CRC32 over sealed metadata fields |
| Selection | Highest valid generation, primary on a tie |
| System slot | Encoded as `active_system_slot` for future A/B control |
| Journal | Prepared → committed metadata-journal entry state machine |
| A/B state | Active and pending system-slot state in every superblock |
| Directory inventory | Caller-owned bounded catalog of up to 16 validated file or directory entries |
| Root directory block | Sealed, required root-directory block reference in each superblock |

The QEMU probe writes a newer primary and an older backup, corrupts the primary checksum, confirms that recovery selects the backup slot, seals then commits a metadata-journal entry, and stages an A/B system-slot change. Recovery always boots the active slot until the pending slot is explicitly confirmed; confirmation promotes it and advances the generation. Host tests independently cover device validation, normal generation ordering, invalid-backup fallback, inverse runtime recovery, commit immutability, journal tampering, and A/B staging. Extents, B+ trees, persistent journal placement, physical-device drivers, installer flows, and Recovery UI remain subsequent VaultFS work.

The independent directory inventory adds bounded metadata for a nonzero object ID, first block, byte count, kind, and a lowercase ASCII name. `vaultfs_directory_insert` copies a valid entry only into a caller-owned inventory with remaining capacity and no same-name entry. `vaultfs_directory_find` then exposes a read-only pointer to retained metadata. Host coverage proves valid insertion and lookup, duplicate-name rejection, zero object-ID rejection, and invalid-name rejection.

This is **not** an on-disk directory implementation. The API does not allocate, read, write, or own entry blocks; persist its inventory; derive a path hierarchy; traverse child directories; authenticate names; link Parcel packages; or grant access to application data. Persistent directory layout, allocation, integrity, and authority need their own later contract.

The superblock is now format `2` and carries a required `root_directory_block` field covered by its existing checksum. Redundant-superblock recovery preserves this reference exactly with the selected valid generation; a sentinel meaning “no root directory block” is invalid. This makes the root directory’s future storage location durable metadata, but it does **not** serialize the in-memory directory inventory to that block or read it during boot. The root block’s physical layout, allocation ownership, atomic update protocol, and content checksum remain separate work.
