<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/KEYS_RELAY_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Keys, Relay, and Origin bootstrap

**Status:** Implemented as an in-kernel bootstrap model and verified in host tests and the normal x86_64 QEMU probe.

The first authority model in VibeOS uses **opaque Key tokens** rather than ambient object access. The token is only an identifier; Origin owns the authoritative record of its target Pulse Object and granted rights. A Relay Link binds to one object and a rights ceiling. A transfer succeeds only when both the sender Key and that ceiling contain every requested right, after which Origin mints a narrower recipient Key.

| Element | Current bootstrap behavior |
|---|---|
| Pulse Object | 64-bit object identifier |
| Key | Opaque 64-bit token; rights are stored only in Origin's record |
| Rights | `READ`, `WRITE`, `INSPECT` |
| Key minting | Origin-only, fixed capacity of 64 records |
| Narrowing | Child Key must be a non-empty subset of parent rights |
| Relay Link | Object-bound with an immutable transfer ceiling while active |
| Revocation | Disables the specified Key record immediately |

Relay now also offers a bounded in-kernel channel with eight FIFO slots. Sending a message asks the channel's Link to mint the recipient Key, so the queue carries a word together with a constrained capability rather than a copied authority record. The QEMU startup probe mints a read/write/inspect Key for object `1`, creates a Relay Link capped at `READ`, transfers a read-only child Key, verifies it, rejects an attempted `WRITE` transfer, then sends and receives a read-only Key through the channel.

The model is intentionally local to the early kernel. Process-scoped key spaces, lifecycle inheritance, atomic concurrency, object destruction, blocking waits, and user-mode transport will be added as Pulse and Origin acquire process isolation.

## Origin ABI bootstrap

Origin now also exposes a versioned, plain-C call frame for two native operations: `INSPECT_KEY` and `NARROW_KEY`. The `ORIGIN_CALL` frame carries its ABI version, operation, input Key, requested rights, result fields, and explicit status. The current in-kernel probe uses it to create and verify an `INSPECT` Key. It is the source-level native contract from which the future user-mode Origin runtime and Vibe SDK bindings will grow; it is not a POSIX interface.
