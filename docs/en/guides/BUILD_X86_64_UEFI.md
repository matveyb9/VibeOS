<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/guides/BUILD_X86_64_UEFI.md">🇷🇺 РУССКИЙ</a>
</p>

# Build and verify Prelude on x86_64 UEFI

**Status:** Experimental Project Foundation guide

**Host:** Linux x86_64 or ARM64 with the listed packages available.

**Target:** QEMU x86_64 UEFI.

**Boot path:** UEFI.
**Support:** First required verification profile.

## Prerequisites

Install Clang, LLD, QEMU System x86, OVMF, mtools, and core build tools. On Debian or Ubuntu hosts, the package names are `clang`, `lld`, `qemu-system-x86`, `ovmf`, `mtools`, `dosfstools`, and `make`.

## Build

```text
make uefi-image
```

This produces `build/prelude/BOOTX64.EFI` and a FAT ESP image at `build/vibeos-uefi-esp.img`.

## Verify

```text
make check-uefi
```

The verification profile starts OVMF in QEMU, boots the independent Prelude UEFI application, and checks the debug marker emitted after firmware console access succeeds.

## Expected result

```text
Prelude UEFI verification passed.
```

Windows and macOS host guides will be added with equivalent tested toolchain profiles. They are not yet claimed as implemented.
