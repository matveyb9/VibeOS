<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PARCEL_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Parcel VPK bootstrap

**Status:** Implemented as a manifest, registry, installation policy, and data-only native launch-request admission bootstrap. Archive decoding and cryptographic signature verification are not yet implemented.

Parcel now defines a strict, fixed-size VPK manifest for a native application identifier, installation scope, payload length, payload checksum, and requested rights. A bounded registry accepts a manifest only when it is structurally valid, a caller owns a `WRITE` Key for the Parcel registry object, the manifest identifier is not already present, and the caller has supplied a successful signature-verification result.

| Property | Initial behavior |
|---|---|
| VPK format | Version `1` manifest contract |
| Scopes | `Core`, `Local`, `User` |
| Identity | Lowercase `a-z`, digits, `.` and `-`, maximum 31 bytes |
| Requested rights | Non-empty subset of `READ`, `WRITE`, `INSPECT` |
| Registry | 16 independent manifest entries |
| Authorization | `WRITE` Key on the Parcel registry Pulse Object |
| Signature result | Required input to installation policy; cryptographic verifier is pending |
| Native request | Bounded Prompt, Cue, or Vector ID resolves to a canonical Parcel application identifier |
| Admission | Read-only lookup permits a native request only when its exact identifier is already in the registry |
| Admission result | Immutable `admitted`, `not installed`, or `invalid request` status paired with the native application ID |
| Image descriptor | Immutable future-loader metadata: flat x86_64 format, nonzero byte count and checksum, and an in-range entry offset |
| ELF64 header metadata | Read-only inspection of at least 64 supplied bytes: ELF magic, 64-bit little-endian current format, static `ET_EXEC`, x86-64 machine, entry address, and a structurally bounded program-header table declaration [1] |
| ELF64 program-header metadata | Read-only inspection of caller-supplied, exactly 56-byte `PT_LOAD` records; it returns only segment count and the declared virtual-address span after size, flags, overflow, alignment, and congruence checks [2] |

The QEMU probe mints only the registry Key needed for a Core manifest, installs a verified Prompt manifest, checks the resulting registry count, and admits one matching native request. The read-only admission operation returns an immutable explicit result for a valid installed request, valid but not-installed request, or invalid request. The bounded image descriptor is validation-only and intentionally contains no media address, loaded bytes, callback, or process identity. Host tests also reject a read-only installer, duplicate identifier, unverified request, unknown native ID, and an out-of-range image entry offset. A Horizon runtime selection may resolve a native descriptor and Pulse may form this Parcel request record; neither operation loads package media, executes code, creates a process, grants rights, or changes the registry. The policy does not claim to verify a digital signature itself yet; the future `.vps` verifier must supply the `signature_verified` result before Parcel can use real package media.

The new `parcel_elf64_header_describe` boundary receives only a caller-provided byte pointer, an explicit available-byte count, and caller-owned output metadata. It rejects short buffers, invalid magic/class/byte order/ident version, a non-current ELF version, non-`ET_EXEC` type, non-x86-64 machine, a non-64-byte header, inconsistent program-header absence, undersized program-header entries, and an overflowing declared program-header extent. It does **not** fetch or retain package media, dereference the declared program-header table, interpret program headers or segments, map memory, relocate, link dynamically, expose a POSIX or libc ABI, grant rights, create a process, or call the stated entry address.

`parcel_elf64_program_headers_describe` is a separate subsequent boundary. It accepts a header metadata record plus a distinct bounded table buffer, requires the initial 56-byte ELF64 entry size and one or more declared records, and permits only `PT_LOAD` entries with standard permission bits. It verifies `p_filesz ≤ p_memsz`, checked virtual-address extent, and an optional power-of-two alignment/congruence relation; it returns no bytes, mappings, permissions, load addresses, callbacks, or executable handles. This is structural metadata validation, not segment loading: Parcel neither reads the declared file offsets nor copies/zero-fills a segment, installs page permissions, resolves symbols, relocates, invokes a dynamic interpreter, or transfers control. [2]

## References

[1] [System V ABI: ELF Header](https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html)
[2] [System V ABI: Program Header](https://refspecs.linuxfoundation.org/elf/gabi4+/ch5.pheader.html)
