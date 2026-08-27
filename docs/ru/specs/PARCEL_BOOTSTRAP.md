<p align="center">
  <a href="../../en/specs/PARCEL_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Parcel VPK

**Статус:** Реализован как bootstrap manifest, registry и policy. Archive decoding и cryptographic signature verification пока не реализованы.

Теперь Parcel определяет строгий fixed-size VPK manifest для native application identifier, installation scope, payload length, payload checksum и requested right. Bounded registry принимает manifest только когда он structurally valid, caller владеет `WRITE` Key для Parcel registry object, manifest identifier ещё отсутствует и caller передал успешный результат signature verification.

| Свойство | Начальное поведение |
|---|---|
| VPK format | Version `1` manifest contract |
| Scope | `Core`, `Local`, `User` |
| Identity | Строчные `a-z`, цифры, `.` и `-`, максимум 31 bytes |
| Requested right | Непустое подмножество `READ`, `WRITE`, `INSPECT` |
| Registry | 16 независимых manifest entry |
| Authorization | `WRITE` Key на Parcel registry Pulse Object |
| Signature result | Обязательный input installation policy; cryptographic verifier pending |

QEMU probe создаёт только registry Key, требуемый Core manifest, устанавливает verified manifest и проверяет итоговый registry count. Host-тесты также отклоняют read-only installer, duplicate identifier и unverified request. Policy пока не утверждает, что сама проверяет digital signature; будущий `.vps` verifier должен передать результат `signature_verified`, прежде чем Parcel сможет использовать реальный package media.
