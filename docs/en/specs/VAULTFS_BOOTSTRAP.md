<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/VAULTFS_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas block and VaultFS bootstrap

**Status:** Implemented as an in-memory Atlas backend and a verified VaultFS redundant-superblock foundation.

Atlas now defines the independent block-device boundary consumed by storage code. Its initial RAM backend has fixed 4 KiB blocks and exists solely to verify the storage interface before hardware discovery and drivers are ready. VaultFS uses that interface to store two checksummed superblocks. During recovery it validates both copies and selects the highest valid generation; if one copy is corrupted, the other remains bootable metadata.

| Component | Initial behavior |
|---|---|
| Atlas backend | Bounded RAM block device with 4 KiB blocks |
| VaultFS metadata | Primary and backup superblocks |
| Integrity | 64-bit FNV-1a checksum over sealed fields |
| Selection | Highest valid generation, primary on a tie |
| System slot | Encoded as `active_system_slot` for future A/B control |
| Journal | `journal_sequence` field reserved for the journal layer |

The QEMU probe writes a newer primary and an older backup, corrupts the primary checksum, and confirms that recovery selects the backup slot. Host tests independently cover device validation, normal generation ordering, invalid-backup fallback, and the inverse runtime-recovery case. Extents, B+ trees, the metadata journal, persistence to physical devices, installer flows, and Recovery UI remain subsequent VaultFS work.
