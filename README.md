<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="README_RU.md">🇷🇺 РУССКИЙ</a>
</p>

# VibeOS

**VibeOS is an independent graphical desktop operating system, built from scratch in C17.** It is aimed at enthusiasts who want a practical native system and at readers who want to learn from explicit interfaces, small components, and paired documentation.

VibeOS is not a Linux distribution. Its current architecture deliberately begins without a POSIX profile, Linux ABI, Multiboot, GRUB, or an inherited runtime. Prelude is its own loader; Dawn Context is its firmware-neutral handoff record; Pulse, Atlas, VaultFS, Prism, Canvas, Horizon, Parcel, Relay, and Origin are native VibeOS components.

## Current status

VibeOS is in **early platform bring-up**, not an installable operating-system release. A reproducible x86_64 QEMU path now reaches the common Pulse runtime through both UEFI/OVMF and Legacy BIOS/SeaBIOS Prelude loaders. The verified probes cover normalized Dawn Context v4 memory descriptors and explicit boot-owned reservations, an early four-GiB identity map, interrupt/timer handling, the bootstrap physical allocator and scheduler state, native component probes, and software-rendered Prism/Canvas/Horizon output.

The Legacy BIOS path uses a two-stage self-owned Prelude image, E820 normalization, VBE `0x118` linear output, and a `BGR888` framebuffer contract. It is verified in QEMU/SeaBIOS only. Neither path is a claim of physical-hardware compatibility, storage installation, user-space isolation, network support, or v1.0 completeness.

## Quick start

On a provisioned development host with the required toolchain and QEMU, run the complete verification set:

```text
make test
```

For a focused firmware path, use `make check-uefi` for the UEFI/OVMF profile or `make check-bios` for the Legacy BIOS/SeaBIOS profile. The build guides document host prerequisites, generated artifacts, and the current verification boundary.

| Goal | Guide |
|---|---|
| Prepare a Linux, macOS, or Windows development host | [Host environments](docs/en/guides/HOST_ENVIRONMENTS.md) |
| Build and inspect the UEFI profile | [x86_64 UEFI](docs/en/guides/BUILD_X86_64_UEFI.md) |
| Build and inspect the Legacy BIOS profile | [x86_64 Legacy BIOS](docs/en/guides/BUILD_X86_64_BIOS.md) |

## What is in the bootstrap

The current source tree contains early, bounded implementations of opaque Keys, Relay links and channels, the Origin ABI v1, Atlas RAM and keyboard probes, VaultFS superblock/journal/A-B state, session and Parcel policy, and the Prism/Canvas/Horizon graphical bootstrap. They establish interfaces and testable behaviour; they are not yet the complete process model, driver ecosystem, persistent filesystem, package-signing system, or desktop application suite promised by VibeOS.

## Roadmap at a glance

The next major work grows the bootstraps into real platform facilities: explicit physical-memory reservations and virtual-memory policy, hardware discovery and drivers, durable VaultFS trees, isolated native processes, compositor surfaces and input, then installation and the wider application platform. A separate public VibeSDK repository remains a later milestone.

Read the [roadmap](docs/en/ROADMAP.md) for sequencing and completion criteria.

## Explore the project

- [Architecture overview](docs/en/ARCHITECTURE.md) and [repository organisation](docs/en/REPOSITORY.md).
- [Dawn Context v4](docs/en/specs/DAWN_CONTEXT.md), [Pulse paging bootstrap](docs/en/specs/PULSE_PAGING_BOOTSTRAP.md), and [Prism/Canvas bootstrap](docs/en/specs/PRISM_CANVAS_BOOTSTRAP.md).
- [Documentation policy](docs/en/DOCUMENTATION.md) and [contribution workflow](docs/en/CONTRIBUTING.md).
- Planned [application platform and Vibe SDK](docs/en/SDK.md).

## Contributing

VibeOS is prepared as an open project. Changes use focused topic branches, reviewable pull requests, concise English commit subjects with bilingual bodies, and English/Russian documentation kept in sync.

Please read [Contributing to VibeOS](docs/en/CONTRIBUTING.md) before opening a change.

## License

The license will be selected before the first external source-code contribution is accepted.
