<p align="center">
  <a href="../../en/specs/ATLAS_KEYBOARD_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap keyboard Atlas

**Статус:** Реализован как bounded architecture-specific event decoder. IRQ delivery и physical discovery остаются последующей работой Atlas.

Первый input component Atlas принимает legacy translated PS/2 scan-code-set-1 byte и превращает каждый non-extended make или break code в небольшое event с normalized key, pressed state и optional uppercase ASCII character. Он намеренно не зависит от Horizon, Canvas, Parcel или Origin. PC platform QEMU документирует PS/2 keyboard emulation, что даёт воспроизводимый target для первого adapter.[1]

| Свойство | Текущее поведение |
|---|---|
| Queue | Ring buffer на 32 event; при overflow input отклоняется, а не перезаписывается |
| Input | Set-1 make/break byte от будущего i8042 adapter |
| Text | Uppercase Latin letter и space; unsupported key сохраняет zero ASCII value |
| Extended prefix | Отложены: `0xe0` и `0xe1` намеренно не классифицируются как key |
| Hardware I/O | Этот module его пока не выполняет |
| Consumption | `atlas_keyboard_next_event()` возвращает FIFO event, не раскрывая queue state |

Host probe проверяет FIFO ordering, key-up preservation, prefix deferral, точный count и overflow rejection. Следующий шаг свяжет безопасно обнаруженный i8042 device с IRQ1, подтвердит PIC и передаст эти event в выделенный input service. USB HID, mouse input, ACPI resource discovery, international layout, repeat, modifier и user-space focus policy намеренно выходят за bootstrap.

## Источник

[1] [Документация QEMU System emulator](https://www.qemu.org/docs/master/system/qemu-manpage.html)
