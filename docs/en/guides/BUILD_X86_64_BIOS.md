<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/guides/BUILD_X86_64_BIOS.md">🇷🇺 РУССКИЙ</a>
</p>

# Build and run: x86_64 Legacy BIOS

**Status:** Implemented for the reproducible QEMU/SeaBIOS profile. This is a self-owned Prelude path; it does not use Multiboot, GRUB, or another bootloader.

The Legacy BIOS path produces `build/vibeos-bios.img`. Its 512-byte first stage loads the separate Prelude stage two. Stage two loads the raw Pulse image, normalizes the BIOS E820 map into Dawn v3 descriptors, requests the VBE 2.0 linear `1024×768×24` mode (`0x118`), enables x86_64 long mode, and transfers the same Dawn Context contract to Pulse that UEFI Prelude produces.[1] [2]

## Requirements

| Host family | Required tools |
|---|---|
| Linux | `clang`, `lld`, LLVM `objcopy`, `nasm`, QEMU, and standard shell tools. |
| macOS | LLVM, NASM, QEMU, and a POSIX-compatible shell. Homebrew supplies the common packages. |
| Windows | MSYS2 UCRT64 or MINGW64 with LLVM, NASM, QEMU, GNU Make, and a Bash-compatible shell. |

Use the same repository checkout and toolchain validation described in [Host environments](HOST_ENVIRONMENTS.md). The exact executable path may be supplied as `QEMU_X86_64=/path/to/qemu-system-x86_64`.

## Commands

```bash
make bios-image
make check-bios
```

`make check-bios` starts the image on QEMU's PC machine, waits for Prelude's `Dawn Context` marker, then confirms that the common Pulse timer path executes. `make test` includes this check as well as the UEFI, keyboard, panic, artifact, and host unit checks.

## Scope and safety

This profile is currently validated only on QEMU's legacy BIOS emulation. It owns E820 collection and VBE mode choice while BIOS services remain available. The first MiB is conservatively normalized as reserved because it contains the active BIOS bootstrap, temporary Pulse staging data, and transition page tables. Physical legacy-BIOS hardware, alternate video modes, USB input, ACPI device discovery, and disk-filesystem loading are not yet supported claims. Do not write development images to physical media unless the target hardware and recovery procedure have been explicitly validated.

## References

[1] [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

[2] [Linux kernel documentation: vesafb](https://docs.kernel.org/fb/vesafb.html)
