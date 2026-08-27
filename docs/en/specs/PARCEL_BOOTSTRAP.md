<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PARCEL_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Parcel VPK bootstrap

**Status:** Implemented as a manifest, registry, and policy bootstrap. Archive decoding and cryptographic signature verification are not yet implemented.

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

The QEMU probe mints only the registry Key needed for a Core manifest, installs a verified manifest, and checks the resulting registry count. Host tests also reject a read-only installer, duplicate identifier, and unverified request. The policy does not claim to verify a digital signature itself yet; the future `.vps` verifier must supply the `signature_verified` result before Parcel can use real package media.
