<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/ATLAS_KEYBOARD_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas keyboard bootstrap

**Status:** Implemented as a bounded, architecture-specific event decoder with a reproducible QEMU i8042 IRQ1 adapter. Physical discovery remains subsequent Atlas work.

The first Atlas input component accepts legacy translated PS/2 scan-code-set-1 bytes and turns each non-extended make or break code into a small event with a normalized key, pressed state, and optional uppercase ASCII character. It deliberately has no dependency on Horizon, Canvas, Parcel, or Origin. The QEMU PC platform documents PS/2 keyboard emulation, providing a reproducible target for the first adapter.[1]

| Property | Current behavior |
|---|---|
| Queue | Ring buffer of 32 events; rejects overflow rather than overwriting input |
| Input | Set-1 make/break bytes delivered by a future i8042 adapter |
| Text | Uppercase Latin letters plus space; unsupported keys retain a zero ASCII value |
| Semantic keys | Explicit `ATLAS_KEY_TAB` and `ATLAS_KEY_ENTER` preserve non-text desktop intent alongside raw scan code and optional text |
| Extended prefixes | Deferred; `0xe0` and `0xe1` are intentionally not misclassified as keys |
| Hardware I/O | Separate constrained i8042 adapter reads data only after output-ready status and acknowledges PIC IRQ1 |
| Consumption | `atlas_keyboard_next_event()` returns FIFO events without leaking queue state |

Host probes verify FIFO ordering, key-up preservation, prefix deferral, exact count reporting, semantic Tab/Enter decoding, and overflow rejection. A separate QEMU profile boots to an IRQ1-ready state, injects Tab through QEMU's monitor, and verifies an end-to-end controlled transition: IRQ1 enqueues the event and returns, then the Horizon profile-owned pump consumes it and redraws focus. The IRQ adapter itself remains Horizon-independent and never renders or exits QEMU. USB HID, mouse input, ACPI resource discovery, international layouts, repeat, modifiers, Shift+Tab, and general user-space focus policy are intentionally outside this bootstrap.

## Reference

[1] [QEMU System emulator user documentation](https://www.qemu.org/docs/master/system/qemu-manpage.html)
