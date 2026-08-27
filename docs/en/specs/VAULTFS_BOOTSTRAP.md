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
| Integrity | CRC32 over sealed metadata fields |
| Selection | Highest valid generation, primary on a tie |
| System slot | Encoded as `active_system_slot` for future A/B control |
| Journal | Prepared → committed metadata-journal entry state machine |
| A/B state | Active and pending system-slot state in every superblock |

The QEMU probe writes a newer primary and an older backup, corrupts the primary checksum, confirms that recovery selects the backup slot, seals then commits a metadata-journal entry, and stages an A/B system-slot change. Recovery always boots the active slot until the pending slot is explicitly confirmed; confirmation promotes it and advances the generation. Host tests independently cover device validation, normal generation ordering, invalid-backup fallback, inverse runtime recovery, commit immutability, journal tampering, and A/B staging. Extents, B+ trees, persistent journal placement, physical-device drivers, installer flows, and Recovery UI remain subsequent VaultFS work.
