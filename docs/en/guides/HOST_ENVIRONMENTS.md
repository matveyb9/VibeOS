<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/guides/HOST_ENVIRONMENTS.md">🇷🇺 РУССКИЙ</a>
</p>

# Host environments for Prelude

**Status:** Experimental Project Foundation guide

VibeOS uses one source tree and one `make` interface for the first Prelude x86_64 UEFI target. Host instructions below describe developer environments; they do not claim that every host profile is part of release validation yet.

| Host | Build support | QEMU verification | Required action |
|---|---|---|---|
| Linux | Implemented and verified on Ubuntu 24.04 | Implemented and verified with QEMU and OVMF | Install the listed packages and run `make check-uefi`. |
| macOS | Documented, not yet CI-verified | Documented; requires an explicit OVMF firmware pair | Install Homebrew packages and set `OVMF_CODE` and `OVMF_VARS`. |
| Windows 10/11 | Documented through MSYS2 MINGW64, not yet CI-verified | Documented; requires an explicit OVMF firmware pair | Use an MSYS2 MINGW64 shell and set `OVMF_CODE` and `OVMF_VARS`. |

## Common commands

Run the prerequisite check first:

```text
tools/check-host.sh
```

Build and start the automated UEFI probe:

```text
make check-uefi
```

`OVMF_CODE` and `OVMF_VARS` must point to matching, writable-use OVMF firmware files. The verification script copies the variable-store template before every run, so a test does not alter the source template.

## Linux

On Debian or Ubuntu, install:

```text
sudo apt-get install clang lld qemu-system-x86 ovmf mtools dosfstools make
```

The default OVMF paths used by the first profile are `/usr/share/OVMF/OVMF_CODE_4M.fd` and `/usr/share/OVMF/OVMF_VARS_4M.fd`.

## macOS

Install the base command-line tools and QEMU:

```text
xcode-select --install
brew install llvm qemu make mtools coreutils
```

Ensure the Homebrew LLVM bin directory precedes the Apple toolchain in `PATH`, then obtain a compatible x86_64 OVMF code image and variable-store template. OVMF is the EDK II UEFI firmware implementation intended for QEMU/KVM virtual machines.[1] Provide their paths before running the common commands:

```text
export OVMF_CODE=/absolute/path/to/OVMF_CODE.fd
export OVMF_VARS=/absolute/path/to/OVMF_VARS.fd
export PATH="$(brew --prefix llvm)/bin:$PATH"
```

Use `gtimeout` from `coreutils`; the verification script selects it automatically if `timeout` is absent. QEMU documents Homebrew as a supported installation path for macOS.[2]

## Windows 10 or 11

Install MSYS2, update it, then open an **MINGW64** shell. QEMU's own download page documents the MINGW64 package name for 64-bit Windows.[2]

```text
pacman -Syu
pacman -S mingw-w64-x86_64-clang mingw-w64-x86_64-lld mingw-w64-x86_64-qemu mingw-w64-x86_64-mtools make coreutils
```

Place matching x86_64 OVMF files somewhere stable, for example `C:/VibeOS/firmware`, and export them in the MSYS2 shell:

```text
export OVMF_CODE=/c/VibeOS/firmware/OVMF_CODE.fd
export OVMF_VARS=/c/VibeOS/firmware/OVMF_VARS.fd
```

Run `tools/check-host.sh` and then `make check-uefi`. This profile intentionally uses only the MSYS2 shell interface; PowerShell and native batch wrappers are not introduced until they are testable in the project.

## References

[1] [TianoCore: OVMF](https://www.tianocore.org/tianocore-wiki.github.io/platforms-packages/platform-ports/ovmf.html)

[2] [QEMU: Download and host installation paths](https://www.qemu.org/download/)
