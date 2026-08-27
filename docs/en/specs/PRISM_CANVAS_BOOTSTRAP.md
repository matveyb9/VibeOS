<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PRISM_CANVAS_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Prism and Canvas bootstrap

**Status:** Implemented as an early x86_64 software-rendering path in the QEMU UEFI and Legacy BIOS profiles. It is not yet the user-space compositor promised for VibeOS v1.0.

UEFI Prelude queries the firmware Graphics Output Protocol before it leaves Boot Services; Legacy BIOS Prelude selects a VBE linear mode while BIOS services remain available. Both producers add the active framebuffer base, size, dimensions, scan-line stride, and `RGBX8888`, `BGRX8888`, or `BGR888` format to the append-only Dawn Context v3. Pulse validates the descriptor after taking control of `CR3`; its temporary four-GiB identity map covers the QEMU framebuffer range. GOP exposes the framebuffer driven by a display output and provides its pixel format and scan-line information.[1]

| Layer | Initial responsibility |
|---|---|
| Prelude | Obtain GOP information before `ExitBootServices()` or choose the Legacy BIOS VBE linear mode before the firmware handoff |
| Dawn Context v3 | Transfer physical framebuffer descriptor without exposing live UEFI or BIOS services |
| Prism | Validate the framebuffer and software-fill clipped RGB rectangles |
| Canvas | Retain and render bounded ordered rectangles and uppercase bitmap labels |
| Pulse probe | Paint a dark background and three overlapping visual blocks before timer verification |

The host test validates descriptor handling, clipping, `BGR888` byte packing, draw order, uppercase bitmap labels, and retained-scene rendering. The QEMU UEFI and Legacy BIOS probes confirm the software paint path ran before external timer delivery. Future work separates Prism into user space, creates surfaces and window composition, adds scalable text and input, consumes display drivers through Atlas, and removes the temporary broad identity map.

## Reference

[1] [UEFI Specification 2.10, Graphics Output Protocol](https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html)
