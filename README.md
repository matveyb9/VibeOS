<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="README_RU.md">🇷🇺 РУССКИЙ</a>
</p>

# VibeOS

**VibeOS is an independent desktop operating system, built from scratch.** It is designed for enthusiasts who want a practical graphical system and for people who want to understand how that system works from its source code.

VibeOS is not a Linux distribution and does not inherit a POSIX compatibility layer as its starting point. It has its own loader, kernel model, application ABI, libraries, package format, graphical stack, recovery modes, and developer platform.

## Current status

VibeOS is in the **project foundation** stage. The architecture, repository principles, and initial documentation are established. The first independent x86_64 UEFI Prelude image builds and is verified in QEMU. It now captures a UEFI memory map, exits firmware Boot Services, and transfers `Dawn Context v1` to an early native Pulse image.

## What we are building

The project targets a complete desktop experience: the `Prelude` boot environment, the `Pulse` kernel, `Atlas` drivers, `VaultFS` storage, `Prism` composition, `Canvas` application UI, `Horizon` desktop, `Parcel` packages, and a native application SDK.

The first stable target is x86_64. VibeOS is designed to support UEFI and legacy BIOS boot paths, with QEMU as the first development and verification environment.

## Quick start

On a supported Linux host with the required toolchain, build and verify the first Prelude image with:

```text
make check-uefi
```

Read [Build and verify Prelude on x86_64 UEFI](docs/en/guides/BUILD_X86_64_UEFI.md) for prerequisites, generated files, and the verification result. It is an experimental bring-up image, not an installable operating-system release.

## Roadmap at a glance

The project begins with a reproducible toolchain and a minimal boot path. It then develops the kernel and IPC foundation, storage and installation, the native runtime and packages, the graphical desktop, the core applications, and finally public alpha and beta release lines.

Read the [simplified roadmap](docs/en/ROADMAP.md) for the full sequence and its completion criteria.

## Explore the project

- Read the [architecture overview](docs/en/ARCHITECTURE.md).
- Learn how the [repository is organised](docs/en/REPOSITORY.md).
- Review the [documentation policy](docs/en/DOCUMENTATION.md).
- Read the [contribution and workflow guide](docs/en/CONTRIBUTING.md).
- Build the first [x86_64 UEFI Prelude image](docs/en/guides/BUILD_X86_64_UEFI.md).
- Configure a supported [Prelude host environment](docs/en/guides/HOST_ENVIRONMENTS.md).
- Inspect the [Dawn Context v1 boot contract](docs/en/specs/DAWN_CONTEXT.md).
- See the planned [application platform and Vibe SDK](docs/en/SDK.md).
- Track the [roadmap](docs/en/ROADMAP.md).

## Contributing

VibeOS is being prepared as an open project. Contributions will use short topic branches, reviewed pull requests, structured commit messages, and documentation that stays in sync with code.

Please read [Contributing to VibeOS](docs/en/CONTRIBUTING.md) before opening a change.

## License

The project license will be selected before the first source-code contribution is accepted.
