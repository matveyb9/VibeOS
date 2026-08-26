<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../ru/DEVELOPMENT.md">🇷🇺 РУССКИЙ</a>
</p>

# Development environments

**Status:** Planned; commands will be added with the Project Foundation toolchain

VibeOS development documentation will cover each host and target combination explicitly. The project does not assume Linux, x86_64 hosts, or only one boot path.

| Host environment | Initial documentation status |
|---|---|
| Windows x86_64 and ARM64 | Planned through native tooling or a documented compatible environment |
| macOS x86_64 and Apple Silicon | Planned |
| Linux x86_64 and ARM64 | Planned |

| Target or execution profile | Initial status |
|---|---|
| QEMU x86_64 UEFI | First required profile |
| QEMU x86_64 legacy BIOS | First required profile |
| Physical x86_64 PC Baseline | Planned after Atlas baseline drivers |
| QEMU AArch64 `virt` | Planned after x86_64 ABI stabilisation |
| QEMU RISC-V RV64GC `virt` | Research target after AArch64 |

Each future guide will state the host, target, boot path, command sequence, debugger connection, serial log path, expected outcome, and support status. A guide that is not yet implemented will say so directly instead of presenting untested commands.
