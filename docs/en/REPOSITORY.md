<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../ru/REPOSITORY.md">🇷🇺 РУССКИЙ</a>
</p>

# Repository guide

**Status:** Accepted project direction

The repository tree describes how VibeOS is developed. It does not mirror the `/System`, `/Users`, or `/Volumes` layout of an installed VibeOS machine.

```text
src/
├── boot/      early boot and loader code
├── arch/      instruction-set-specific code
├── platform/  firmware, platform tables, and machine profiles
├── kernel/    core kernel mechanisms
├── ipc/       objects, Keys, Links, and Relay protocol
├── drivers/   buses, device classes, and drivers
├── storage/   VFS, VaultFS, volumes, and verification
├── network/   network stack and protocols
├── runtime/   C17 runtime and application libraries
├── security/  trust, signatures, cryptography, and policies
├── services/  privileged user-space services
├── ui/        compositor, UI SDK, desktop, and accessibility
└── apps/      user-facing applications
```

Each directory has one responsibility. A subsystem uses another subsystem only through a documented public interface. Private headers and private data structures do not cross boundaries. New cross-domain dependencies require a small contract, a test, and documentation.

The root also contains `docs/`, `tools/`, `tests/`, `assets/`, `third_party/`, and `.github/`. Generated output belongs outside Git and is ignored by `.gitignore`.

Read [Contributing](CONTRIBUTING.md) before changing code or documentation.
