<p align="center">
  <a href="../../en/specs/VAULTFS_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Atlas block и VaultFS

**Статус:** Реализован как in-memory Atlas backend и проверенный фундамент redundant superblock VaultFS.

Теперь Atlas определяет независимую block-device boundary, которую потребляет storage code. Его начальный RAM backend использует фиксированные block по 4 КиБ и существует только для проверки storage interface до готовности hardware discovery и driver. VaultFS использует этот interface для хранения двух superblock с checksum. При recovery он проверяет обе копии и выбирает наивысшую валидную generation; при повреждении одной копии другая остаётся bootable metadata.

| Компонент | Начальное поведение |
|---|---|
| Atlas backend | Bounded RAM block device с block по 4 КиБ |
| VaultFS metadata | Primary и backup superblock |
| Integrity | 64-битная FNV-1a checksum sealed field |
| Selection | Наивысшая valid generation, primary при равенстве |
| System slot | Закодирован как `active_system_slot` для будущего A/B control |
| Journal | Поле `journal_sequence` зарезервировано для journal layer |

QEMU probe записывает новый primary и более старый backup, повреждает checksum primary и подтверждает, что recovery выбирает backup slot. Host-тесты независимо покрывают validation device, обычный порядок generation, fallback при invalid backup и обратный runtime-recovery case. Extent, B+ tree, metadata journal, persistence на physical device, installer flow и Recovery UI остаются следующей работой VaultFS.
