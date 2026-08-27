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
| Root block payload | Checksumed versioned bounded directory-entry block, адресуемый через sealed superblock reference |

QEMU probe записывает новый primary и более старый backup, повреждает checksum primary, подтверждает, что recovery выбирает backup slot, запечатывает и commit metadata-journal entry, а также stage A/B system-slot change. Recovery всегда загружает active slot, пока pending slot явно не подтверждён; подтверждение делает его active и увеличивает generation. Host-тесты независимо покрывают validation device, обычный порядок generation, fallback при invalid backup, обратный runtime recovery, неизменность commit, tampering journal и A/B staging. Extent, B+ tree, persistent journal placement, driver physical device, installer flow и Recovery UI остаются следующей работой VaultFS.

Независимый directory inventory добавляет bounded metadata для nonzero object ID, first block, byte count, kind и lowercase ASCII name. `vaultfs_directory_insert` копирует valid entry только в caller-owned inventory с remaining capacity и без entry с тем же name. Затем `vaultfs_directory_find` предоставляет read-only pointer на retained metadata. Host coverage доказывает valid insertion и lookup, duplicate-name rejection, zero object-ID rejection и invalid-name rejection.

Это **не** on-disk directory implementation. API не allocation, read, write или own entry block; не persist inventory; не выводит path hierarchy; не traverse child directory; не authenticates name; не связывает Parcel package и не grant доступ к application data. Persistent directory layout, allocation, integrity и authority требуют собственного последующего contract.

Superblock теперь format `2` и содержит обязательное поле `root_directory_block`, покрытое existing checksum. Redundant-superblock recovery сохраняет этот reference точно вместе с выбранной valid generation; sentinel со значением «нет root directory block» invalid. Тем самым future storage location root directory становится durable metadata, но in-memory directory inventory **не** сериализуется в этот block и не read во время boot. Physical layout root block, allocation ownership, atomic update protocol и content checksum остаются отдельной последующей работой.

VaultFS теперь определяет этот block как versioned checksummed root-directory payload с bounded count и copied directory entry. Construction очищает unused record bytes перед sealing payload. Validation проверяет magic, version, checksum, count, каждый retained entry и duplicate name. Store/load operation использует только sealed root-block reference из valid superblock; host coverage доказывает round trip caller-owned inventory через Atlas RAM storage и обратно. Операция намеренно остаётся узкой: она не atomically coordinate replacement root block с superblock update, не allocation new block, не recover journaled directory mutation, не traverse hierarchy и не предоставляет result Parcel/Horizon.

Root-directory loading теперь также требует совпадения payload generation с selected valid superblock generation. Checksum-valid payload в верном block, но от другой generation, отклоняется и не представляется current metadata. Это связывает durable directory snapshot с generation, выбранной redundant recovery, но всё ещё не предоставляет atomic writer, rollback history, journal replay или hierarchy traversal.

## Последующая работа над portable persisted-byte-format

Текущие bootstrap record VaultFS используют implementation layout annotations, чтобы сохранить compact in-memory representation. Этого достаточно только для нынешнего proof на одном toolchain и это **не** final strict ISO C17 persisted-format contract. Перед любым compatible-media claim следующая migration должна определить fixed byte offset и byte count каждого поля superblock, journal, directory entry и root block; явно encode/decode unsigned value; исключать из canonical checksum input только fixed checksum field; отклонять short, reserved и malformed byte record. Migration намеренно отделена от проверенного storage behavior выше, чтобы получить independent host vector и recovery validation.

Путь root-directory теперь завершил первую такую migration. Его persisted payload — ровно 988 bytes: fixed header на 24 bytes, шестнадцать fixed directory record по 60 bytes и checksum на четыре bytes. Root-directory entry и block object — ordinary C17 host metadata, тогда как block I/O path явно encode/decode little-endian unsigned field и fixed name byte. Canonical CRC32 покрывает ровно первые 984 wire bytes. Форматы superblock и journal остаются отдельно ограниченными последующими migration; cross-toolchain или compatible-media claim пока не делается.

Путь superblock завершил следующую migration. Его persisted payload — ровно 60 bytes с explicit little-endian field по fixed offset и four-byte checksum на offset 56; canonical CRC32 покрывает ровно предшествующие 56 bytes. Redundant store/load path теперь encode и decode этот buffer перед validation и generation selection. Host byte vector подтверждает initial location magic, format, block-size и root-reference. Journal persistence остаётся отдельной migration, и формат по-прежнему не делает compatible-media или cross-toolchain claim.

Journal metadata теперь имеет собственный explicit persisted path. Journal entry encode в ровно 36 little-endian bytes: magic, transaction ID, target block, payload checksum, state и final checksum. Canonical CRC32 покрывает первые 32 bytes. Bounded store/load operation проверяет entry до write и после decode, поэтому возвращается только prepared или committed sealed journal record. Existing journal остаётся одним metadata record: он ещё не reserve journal region, не replay transaction, не coordinate root update atomically и не предоставляет crash-consistency за пределами этого одного validated record.

Перед безопасной coordination root replacement VaultFS теперь имеет bounded dual-snapshot selector. Он принимает primary, затем backup root metadata только когда snapshot независимо valid и имеет requested generation; selected result копируется в caller-owned memory. Это сохраняет valid matching snapshot, если primary peer unavailable, но ещё не хранит second root reference, не allocation snapshot и не делает update atomic.
