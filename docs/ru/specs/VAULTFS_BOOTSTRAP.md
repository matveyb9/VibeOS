<p align="center">
  <a href="../../en/specs/VAULTFS_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Atlas block и VaultFS

**Статус:** Реализован как in-memory Atlas backend и проверенный фундамент redundant superblock VaultFS.

Теперь Atlas определяет независимую block-device boundary, которую потребляет storage code. Его начальный RAM backend использует фиксированные block по 4 КиБ и существует только для проверки storage interface до готовности hardware discovery и driver. VaultFS использует этот interface для хранения двух superblock с checksum. При recovery он проверяет обе копии и выбирает наивысшую валидную generation; при повреждении одной копии другая остаётся bootable metadata.

| Компонент | Начальное поведение |
|---|---|
| Atlas backend | Bounded RAM block device с block по 4 КиБ |
| VaultFS metadata | Format `2` primary и backup superblock |
| Integrity | CRC32 по sealed metadata field |
| Selection | Наивысшая valid generation, primary при равенстве |
| System slot | Закодирован как `active_system_slot` для будущего A/B control |
| Journal | Metadata-journal entry state machine Prepared → committed |
| A/B state | Active и pending system-slot state в каждом superblock |
| Directory inventory | Caller-owned bounded catalog до 16 validated file или directory entry |
| Root directory block | Sealed обязательный root-directory block reference в каждом superblock |

QEMU probe записывает новый primary и более старый backup, повреждает checksum primary, подтверждает, что recovery выбирает backup slot, запечатывает и commit metadata-journal entry, а также stage A/B system-slot change. Recovery всегда загружает active slot, пока pending slot явно не подтверждён; подтверждение делает его active и увеличивает generation. Host-тесты независимо покрывают validation device, обычный порядок generation, fallback при invalid backup, обратный runtime recovery, неизменность commit, tampering journal и A/B staging. Extent, B+ tree, persistent journal placement, driver physical device, installer flow и Recovery UI остаются следующей работой VaultFS.

Независимый directory inventory добавляет bounded metadata для nonzero object ID, first block, byte count, kind и lowercase ASCII name. `vaultfs_directory_insert` копирует valid entry только в caller-owned inventory с remaining capacity и без entry с тем же name. Затем `vaultfs_directory_find` предоставляет read-only pointer на retained metadata. Host coverage доказывает valid insertion и lookup, duplicate-name rejection, zero object-ID rejection и invalid-name rejection.

Это **не** on-disk directory implementation. API не allocation, read, write или own entry block; не persist inventory; не выводит path hierarchy; не traverse child directory; не authenticates name; не связывает Parcel package и не grant доступ к application data. Persistent directory layout, allocation, integrity и authority требуют собственного последующего contract.

Superblock теперь format `2` и содержит обязательное поле `root_directory_block`, покрытое existing checksum. Redundant-superblock recovery сохраняет этот reference точно вместе с выбранной valid generation; sentinel со значением «нет root directory block» invalid. Тем самым future storage location root directory становится durable metadata, но in-memory directory inventory **не** сериализуется в этот block и не read во время boot. Physical layout root block, allocation ownership, atomic update protocol и content checksum остаются отдельной последующей работой.
