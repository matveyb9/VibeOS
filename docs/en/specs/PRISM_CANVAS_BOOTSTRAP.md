<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PRISM_CANVAS_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Prism and Canvas bootstrap

**Status:** Implemented as an early x86_64 UEFI software-rendering path. It is not yet the user-space compositor promised for VibeOS v1.0.

Prelude queries the firmware Graphics Output Protocol before it leaves Boot Services and adds the active framebuffer base, size, dimensions, scan-line stride, and RGBX/BGRX format to the append-only Dawn Context v2. Pulse validates the descriptor after taking control of `CR3`; its temporary four-GiB identity map covers the QEMU framebuffer range. GOP exposes the framebuffer driven by a display output and provides its pixel format and scan-line information.[1]

| Layer | Initial responsibility |
|---|---|
| Prelude | Obtain current GOP framebuffer information before `ExitBootServices()` |
| Dawn Context v2 | Transfer physical framebuffer descriptor without exposing live UEFI services |
| Prism | Validate the framebuffer and software-fill clipped RGB rectangles |
| Canvas | Retain and render a bounded ordered rectangle scene |
| Pulse probe | Paint a dark background and three overlapping visual blocks before timer verification |

The host test validates descriptor handling, clipping, BGR pixel packing, draw order, and retained-scene rendering. The QEMU probe confirms the software paint path ran before external timer delivery. Future work separates Prism into user space, creates surfaces and window composition, adds text and input, consumes display drivers through Atlas, and removes the temporary broad identity map.

## Reference

[1] [UEFI Specification 2.10, Graphics Output Protocol](https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html)
