<p align="center">
  <a href="../../en/specs/KEYS_RELAY_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Keys, Relay и Origin

**Статус:** Реализован как in-kernel bootstrap model и проверен host-тестами и normal x86_64 QEMU-probe.

Первая модель полномочий VibeOS использует **opaque Key token**, а не ambient object access. Token является лишь идентификатором; Origin владеет authoritative record его целевого Pulse Object и выданных rights. Relay Link привязывается к одному object и rights ceiling. Передача проходит только если и Key отправителя, и этот ceiling содержат каждое запрошенное право; после этого Origin создаёт более узкий Key получателя.

| Элемент | Поведение текущего bootstrap |
|---|---|
| Pulse Object | 64-битный object identifier |
| Key | Opaque 64-битный token; rights хранятся только в record Origin |
| Rights | `READ`, `WRITE`, `INSPECT` |
| Key minting | Только Origin, фиксированная ёмкость 64 record |
| Narrowing | Child Key должен быть непустым подмножеством прав parent |
| Relay Link | Привязан к object и immutable transfer ceiling, пока активен |
| Revocation | Немедленно отключает указанный Key record |

QEMU startup probe создаёт Key с `READ`/`WRITE`/`INSPECT` для object `1`, создаёт Relay Link с ограничением `READ`, передаёт read-only child Key, проверяет его и отклоняет попытку передачи `WRITE`. Модель намеренно локальна для раннего kernel. Process-scoped key space, lifecycle inheritance, atomic concurrency, object destruction и user-mode transport появятся, когда Pulse и Origin получат process isolation.
