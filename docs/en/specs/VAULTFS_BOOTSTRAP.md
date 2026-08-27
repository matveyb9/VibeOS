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
| Root block payload | Checksummed, versioned bounded directory-entry block addressed through the sealed superblock reference |

The QEMU probe writes a newer primary and an older backup, corrupts the primary checksum, confirms that recovery selects the backup slot, seals then commits a metadata-journal entry, and stages an A/B system-slot change. Recovery always boots the active slot until the pending slot is explicitly confirmed; confirmation promotes it and advances the generation. Host tests independently cover device validation, normal generation ordering, invalid-backup fallback, inverse runtime recovery, commit immutability, journal tampering, and A/B staging. Extents, B+ trees, persistent journal placement, physical-device drivers, installer flows, and Recovery UI remain subsequent VaultFS work.

The independent directory inventory adds bounded metadata for a nonzero object ID, first block, byte count, kind, and a lowercase ASCII name. `vaultfs_directory_insert` copies a valid entry only into a caller-owned inventory with remaining capacity and no same-name entry. `vaultfs_directory_find` then exposes a read-only pointer to retained metadata. Host coverage proves valid insertion and lookup, duplicate-name rejection, zero object-ID rejection, and invalid-name rejection.

This is **not** an on-disk directory implementation. The API does not allocate, read, write, or own entry blocks; persist its inventory; derive a path hierarchy; traverse child directories; authenticate names; link Parcel packages; or grant access to application data. Persistent directory layout, allocation, integrity, and authority need their own later contract.

The superblock is now format `2` and carries a required `root_directory_block` field covered by its existing checksum. Redundant-superblock recovery preserves this reference exactly with the selected valid generation; a sentinel meaning “no root directory block” is invalid. This makes the root directory’s future storage location durable metadata, but it does **not** serialize the in-memory directory inventory to that block or read it during boot. The root block’s physical layout, allocation ownership, atomic update protocol, and content checksum remain separate work.

VaultFS now defines that block as a versioned, checksummed root-directory payload with a bounded count and copied directory entries. Construction clears unused record bytes before sealing the payload. Validation checks its magic, version, checksum, count, every retained entry, and duplicate names. Store/load operations use only the sealed root-block reference from a valid superblock; host coverage proves the round trip from caller-owned inventory through Atlas RAM storage and back. The operation remains intentionally narrow: it does not atomically coordinate a root-block replacement with a superblock update, allocate a new block, recover a journaled directory mutation, traverse a hierarchy, or expose the result to Parcel/Horizon.

Root-directory loading now also requires the payload generation to equal the selected valid superblock generation. A checksum-valid payload at the right block but from a different generation is rejected rather than presented as current metadata. This binds a durable directory snapshot to the generation selected by redundant recovery, while still not supplying an atomic writer, rollback history, journal replay, or hierarchy traversal.

## Portable persisted-byte-format follow-up

Current VaultFS bootstrap records use implementation layout annotations to keep their in-memory representation compact. That is adequate only for the present single-toolchain proof and is **not** the final strict ISO C17 persisted-format contract. Before any compatible-media claim, the next migration must define fixed byte offsets and byte counts for every superblock, journal, directory entry, and root-block field; encode/decode unsigned values explicitly; exclude only the fixed checksum field from canonical checksum input; and reject short, reserved, or malformed byte records. The migration is deliberately separated from the verified storage behavior above so it can receive independent host vectors and recovery validation.

The root-directory path has now completed the first such migration. Its persisted payload is exactly 988 bytes: a 24-byte fixed header, sixteen 60-byte fixed directory records, and a four-byte checksum. Root-directory entry and block objects are ordinary C17 host metadata, while the block I/O path explicitly encodes/decodes little-endian unsigned fields and the fixed name bytes. The canonical CRC32 covers precisely the first 984 wire bytes. The superblock and journal formats remain separately scoped follow-up migrations; no cross-toolchain or compatible-media claim is made yet.

The superblock path has completed the next migration. Its persisted payload is exactly 60 bytes, with explicit little-endian fields at fixed offsets and a four-byte checksum at offset 56; its canonical CRC32 covers precisely the preceding 56 bytes. The redundant store/load path now encodes and decodes this buffer before validation and generation selection. A host byte vector confirms the initial magic, format, block-size, and root-reference locations. Journal persistence remains a separate migration, and the format continues to make no compatible-media or cross-toolchain claim.

VaultFS superblocks are now format `3`. They carry two distinct required root-directory block references: the primary reference at wire offset 40 and the backup reference at offset 48. The explicit superblock wire record is therefore 68 bytes, with its checksum at offset 64 and its canonical checksum span covering bytes 0 through 63. Validation rejects an absent or duplicated root reference. This publishes the metadata necessary to retain two independently validated root snapshots; it does not yet select two blocks from media during normal boot or atomically replace either snapshot.

Journal metadata now has its own explicit persisted path. A journal entry encodes to exactly 36 little-endian bytes: magic, transaction ID, target block, payload checksum, state, and a final checksum. The canonical CRC32 covers the first 32 bytes. Bounded store/load operations validate an entry before writing and after decoding, so only a prepared or committed sealed journal record is returned. The existing journal remains a single metadata record: it does not yet reserve a journal region, replay transactions, coordinate a root update atomically, or provide crash-consistency beyond that one validated record.

Before root replacement can be coordinated safely, VaultFS now has a bounded dual-snapshot selector. It accepts primary then backup root metadata only when a snapshot independently validates and has the requested generation; it copies the selected result into caller-owned memory. This preserves a valid matching snapshot if its primary peer is unavailable, but does not yet store a second root reference, allocate snapshots, or make an update atomic.

Format 3 dual-root references now have a bounded media path. VaultFS can write a sealed generation-matching snapshot to either the primary or backup reference, read both fixed root records through Atlas, and apply the dual-snapshot selector to return a matching caller-owned result. It deliberately reads both records before selecting, so an I/O failure for either reference currently fails the operation; resilience to unavailable individual reads, snapshot replacement ordering, journal coordination, and atomic update remain subsequent work.

VaultFS now also exposes a conservative recovery decision for one validated journal record and one matching root snapshot. It returns `DISCARD_PREPARED` only for a sealed prepared record whose target is one of the selected superblock’s root references and whose payload checksum equals the snapshot checksum; it returns `ACCEPT_COMMITTED` under the same checks for a sealed committed record. It changes no media, replays no transaction, and does not itself establish atomic persistence.

An immutable root update plan can now be formed for a valid next-generation root snapshot. It names a nonzero transaction, the alternate backup root target, the expected next generation, and the sealed payload checksum. Forming the plan performs no block write, no journal transition, no superblock replacement, and no process-visible filesystem change.
