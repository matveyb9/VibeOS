<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../ru/ARCHITECTURE.md">🇷🇺 РУССКИЙ</a>
</p>

# Architecture overview

**Status:** Accepted project direction

VibeOS is a native desktop operating system. Its architecture is designed as a set of independent subsystems connected by explicit contracts rather than by private cross-project dependencies.

## System path

```text
Prelude → Dawn Context → Pulse → Atlas / Vault / Relay → Prism → Horizon → Parcel → application
```

`Prelude` is the independent boot environment. It supports the planned UEFI and legacy BIOS paths, verifies the selected system image, and passes a Dawn Context to the kernel.

`Pulse` is a modular monolithic kernel. It owns memory, tasks, interrupts, scheduling, system calls, and Pulse Objects. Applications and services use opaque Keys instead of direct access to kernel data.

`Atlas` owns hardware discovery, buses, drivers, and restricted device grants. `Vault` owns VFS, VaultFS, volumes, journaling, and the A/B system layout. `Relay` supplies typed links, messages, events, and controlled Key transfer.

`Prism` is a user-space compositor. `Canvas` is the C17 UI SDK. `Horizon` owns the desktop session, window policy, notifications, setup, and safe session. `Parcel` validates and installs VPK packages.

## Application model

Applications use Origin, Facet, Canvas, Vault Kit, and Meridian Kit. They receive the smallest set of Keys needed to run. Files are selected through Harbor and passed as limited Vault Entry Keys; an application does not automatically receive access to a user profile or system storage.

## Supported execution modes

VibeOS is designed for three modes: a stateless Live session, Live + Vault persistent storage, and a full A/B installation. Recovery, Safe Session, Prelude Console, and Pulse Console provide layered diagnostics and repair without treating a kernel console as an unrestricted administrative shell.

## Read next

Read the [repository guide](REPOSITORY.md), [application platform plan](SDK.md), and [roadmap](ROADMAP.md).
