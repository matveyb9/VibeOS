<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../ru/SDK.md">🇷🇺 РУССКИЙ</a>
</p>

# Application platform and Vibe SDK

**Status:** Accepted project direction; implementation planned

VibeOS will provide a native application platform. C17 is the first official application language. Applications use Origin, Facet, Canvas, Vault Kit, and Meridian Kit rather than POSIX or direct kernel access.

The planned Vibe SDK provides a C17 toolchain profile, headers and ABI libraries, `vibe` CLI tooling, QEMU profiles, templates, samples, VPK packaging, signing, and documentation for Windows, macOS, and Linux hosts.

An application receives minimal Keys by default: its process, AppData, a Canvas window, and logging. Files, network, clipboard, notifications, background mode, devices, and administrative capabilities are provided through an explicit manifest declaration and user or system approval.

Vibe SDK is planned as a future separate repository. It will not be created until VibeOS has a minimal stable Application ABI and a releaseable SDK bundle. VibeOS remains the source of truth for ABI, library sources, specifications, and compatibility tests.
