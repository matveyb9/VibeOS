<p align="center">
  <a href="../../en/specs/VAULTFS_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Atlas block и VaultFS

**Статус:** Реализован как in-memory Atlas backend и проверенный фундамент redundant superblock VaultFS.

Теперь Atlas определяет независимую block-device boundary, которую потребляет storage code. Его начальный RAM backend использует фиксированные block по 4 КиБ и существует только для проверки storage interface до готовности hardware discovery и driver. VaultFS использует этот interface для хранения двух superblock с checksum. При recovery он проверяет обе копии и выбирает наивысшую валидную generation; при повреждении одной копии другая остаётся bootable metadata.

| Компонент | Начальное поведение |
|---|---|
| Atlas backend | Bounded RAM block device с block по 4 КиБ |
| VaultFS metadata | Format `3` primary и backup superblock с distinct dual-root reference |
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

VaultFS superblock теперь format `3`. Они содержат два distinct обязательных root-directory block reference: primary reference на wire offset 40 и backup reference на offset 48. Explicit superblock wire record поэтому занимает 68 bytes, имеет checksum на offset 64, а canonical checksum span покрывает bytes 0–63. Validation отклоняет absent или duplicate root reference. Это публикует metadata, необходимую для хранения двух independently validated root snapshot; normal boot ещё не выбирает два block из media и не atomically replace ни один snapshot.

Journal metadata теперь имеет собственный explicit persisted path. Journal entry encode в ровно 36 little-endian bytes: magic, transaction ID, target block, payload checksum, state и final checksum. Canonical CRC32 покрывает первые 32 bytes. Bounded store/load operation проверяет entry до write и после decode, поэтому возвращается только prepared или committed sealed journal record. Existing journal остаётся одним metadata record: он ещё не reserve journal region, не replay transaction, не coordinate root update atomically и не предоставляет crash-consistency за пределами этого одного validated record.

Перед безопасной coordination root replacement VaultFS теперь имеет bounded dual-snapshot selector. Он принимает primary, затем backup root metadata только когда snapshot независимо valid и имеет requested generation; selected result копируется в caller-owned memory. Это сохраняет valid matching snapshot, если primary peer unavailable, но ещё не хранит second root reference, не allocation snapshot и не делает update atomic.

Format 3 dual-root reference теперь имеют bounded media path. VaultFS может write sealed generation-matching snapshot в primary или backup reference, read оба fixed root record через Atlas и применить dual-snapshot selector, чтобы вернуть matching caller-owned result. Он намеренно read оба record до selection, поэтому I/O failure любого reference пока завершает operation неуспешно; resilience к unavailable individual read, snapshot replacement ordering, journal coordination и atomic update остаются последующей работой.

VaultFS теперь также предоставляет conservative recovery decision для одного validated journal record и одного matching root snapshot. Он возвращает `DISCARD_PREPARED` только для sealed prepared record, target которого является одним root reference selected superblock, а payload checksum равен snapshot checksum; при тех же проверках sealed committed record даёт `ACCEPT_COMMITTED`. Он не изменяет media, не replay transaction и сам по себе не устанавливает atomic persistence.

Теперь может быть сформирован immutable root update plan для valid next-generation root snapshot. Он содержит nonzero transaction, alternate backup root target, expected next generation и sealed payload checksum. Forming plan не выполняет block write, journal transition, superblock replacement или process-visible filesystem change.

Plan может сформировать sealed `PREPARED` journal record с exact transaction identity, alternate target и payload checksum. Эта bounded transformation по-прежнему не выполняет device I/O, snapshot write, journal persistence, commit transition или superblock update.

VaultFS теперь может persist этот sealed prepared record в caller-selected bounded Atlas journal block и load его обратно через canonical journal wire validation. Это остаётся только preparation: оно не write следующий root snapshot, не commit record, не replace superblock и не заявляет atomic update.

Следующая bounded ordered operation может store next root-directory snapshot в alternate root slot. Она сначала load sealed `PREPARED` journal из явно отдельного Atlas block и требует точного совпадения его transaction ID, alternate target и payload checksum с immutable plan и valid next-generation root block. Операция также требует, чтобы target plan был backup root reference выбранного valid format-3 superblock; current primary snapshot она никогда не overwrite. Host coverage показывает, что superblock generation 7 остаётся current после записи alternate snapshot generation 8, а затем сохранённый superblock generation 8 выбирает этот alternate snapshot. Malformed journal bytes, journal location, перекрывающий любой root slot, non-alternate target и wrong root generation отклоняются до snapshot write.

Это checked RAM/block ordering primitive, а не crash-safe root update protocol. Он не доказывает, что prepared record достиг persistent media, не переводит его в `COMMITTED`, не promote redundant superblock, не replay после потери питания, не обрабатывает torn write, не allocation slot и не устанавливает atomicity на physical storage. Эти границы остаются явно отдельными следующими milestones.
