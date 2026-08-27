<p align="center">
  <a href="../../en/specs/PARCEL_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Parcel VPK

**Статус:** Реализован как bootstrap manifest, registry, installation policy и data-only native launch-request admission. Archive decoding и cryptographic signature verification пока не реализованы.

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
| Native request | Bounded Prompt, Cue или Vector ID resolve в canonical Parcel application identifier |
| Admission | Read-only lookup разрешает native request только когда его exact identifier уже находится в registry |
| Admission result | Immutable status `admitted`, `not installed` или `invalid request`, связанный с native application ID |
| Image descriptor | Immutable future-loader metadata: flat x86_64 format, nonzero byte count и checksum, а также in-range entry offset |
| ELF64 header metadata | Read-only inspection как минимум 64 переданных bytes: ELF magic, 64-bit little-endian current format, static `ET_EXEC`, x86-64 machine, entry address и structurally bounded program-header table declaration [1] |

QEMU probe создаёт только registry Key, требуемый Core manifest, устанавливает verified Prompt manifest, проверяет итоговый registry count и допускает один matching native request. Read-only admission operation возвращает immutable explicit result для valid installed request, valid but not-installed request либо invalid request. Bounded image descriptor служит только validation и намеренно не содержит media address, loaded bytes, callback или process identity. Host-тесты также отклоняют read-only installer, duplicate identifier, unverified request, unknown native ID и out-of-range image entry offset. Horizon runtime selection может resolve native descriptor, а Pulse может сформировать этот Parcel request record; ни одна из этих операций не загружает package media, не исполняет code, не создаёт process, не предоставляет right и не изменяет registry. Policy пока не утверждает, что сама проверяет digital signature; будущий `.vps` verifier должен передать результат `signature_verified`, прежде чем Parcel сможет использовать реальный package media.

Новая граница `parcel_elf64_header_describe` получает только предоставленный caller byte pointer, явный available-byte count и caller-owned output metadata. Она отклоняет short buffer, неверные magic/class/byte order/ident version, non-current ELF version, тип не `ET_EXEC`, машину не x86-64, header размером не 64 bytes, inconsistent program-header absence, undersized program-header entry и overflowing declared program-header extent. Она **не** получает и не сохраняет package media, не разыменовывает declared program-header table, не интерпретирует program header или segment, не отображает память, не выполняет relocation, dynamic linking, не предоставляет POSIX или libc ABI, не выдаёт right, не создаёт process и не вызывает stated entry address.

## References

[1] [System V ABI: ELF Header](https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html)
