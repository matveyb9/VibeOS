<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/ATLAS_KEYBOARD_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Atlas keyboard bootstrap

**Status:** Implemented as a bounded, architecture-specific event decoder. IRQ delivery and physical discovery remain subsequent Atlas work.

The first Atlas input component accepts legacy translated PS/2 scan-code-set-1 bytes and turns each non-extended make or break code into a small event with a normalized key, pressed state, and optional uppercase ASCII character. It deliberately has no dependency on Horizon, Canvas, Parcel, or Origin. The QEMU PC platform documents PS/2 keyboard emulation, providing a reproducible target for the first adapter.[1]

| Property | Current behavior |
|---|---|
| Queue | Ring buffer of 32 events; rejects overflow rather than overwriting input |
| Input | Set-1 make/break bytes delivered by a future i8042 adapter |
| Text | Uppercase Latin letters plus space; unsupported keys retain a zero ASCII value |
| Extended prefixes | Deferred; `0xe0` and `0xe1` are intentionally not misclassified as keys |
| Hardware I/O | Not yet performed by this module |
| Consumption | `atlas_keyboard_next_event()` returns FIFO events without leaking queue state |

Host probes verify FIFO ordering, key-up preservation, prefix deferral, exact count reporting, and overflow rejection. The next step binds a safely detected i8042 device to IRQ1, acknowledges the PIC, and sends these events to a dedicated input service. USB HID, mouse input, ACPI resource discovery, international layouts, repeat, modifiers, and user-space focus policy are intentionally outside this bootstrap.

## Reference

[1] [QEMU System emulator user documentation](https://www.qemu.org/docs/master/system/qemu-manpage.html)
