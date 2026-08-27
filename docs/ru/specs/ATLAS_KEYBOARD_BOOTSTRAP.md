<p align="center">
  <a href="../../en/specs/ATLAS_KEYBOARD_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap keyboard Atlas

**Статус:** Реализован как bounded architecture-specific event decoder с воспроизводимым QEMU i8042 IRQ1 adapter. Physical discovery остаётся последующей работой Atlas.

Первый input component Atlas принимает legacy translated PS/2 scan-code-set-1 byte и превращает каждый non-extended make или break code в небольшое event с normalized key, pressed state и optional uppercase ASCII character. Он намеренно не зависит от Horizon, Canvas, Parcel или Origin. PC platform QEMU документирует PS/2 keyboard emulation, что даёт воспроизводимый target для первого adapter.[1]

| Свойство | Текущее поведение |
|---|---|
| Queue | Ring buffer на 32 event; при overflow input отклоняется, а не перезаписывается |
| Input | Set-1 make/break byte от будущего i8042 adapter |
| Text | Uppercase Latin letter и space; unsupported key сохраняет zero ASCII value |
| Semantic key | Явные `ATLAS_KEY_TAB` и `ATLAS_KEY_ENTER` сохраняют non-text desktop intent рядом с raw scan code и optional text |
| Modifier | Bounded Shift bit отслеживается по Set-1 left/right Shift make и break code и snapshot в каждом queued event |
| Extended prefix | Один pending `0xe0` prefix потребляется следующим code для decode Left/Right arrow; `0xe1` по-прежнему очищает pending prefix и отложен |
| Hardware I/O | Отдельный constrained i8042 adapter читает data только после output-ready status и подтверждает PIC IRQ1 |
| Consumption | `atlas_keyboard_next_event()` возвращает FIFO event, не раскрывая queue state |

Host probe проверяет FIFO ordering, key-up preservation, bounded prefix handling, точный count, semantic Tab/Enter/Left/Right decoding, Shift state snapshot и overflow rejection. Отдельный QEMU profile загружается до IRQ1-ready state, inject Right Arrow через QEMU monitor и проверяет end-to-end controlled transition: IRQ1 помещает event в queue и возвращается, затем profile-owned pump Horizon потребляет его и перерисовывает focus. Сам IRQ adapter остаётся независимым от Horizon и никогда не рендерит и не завершает QEMU. USB HID, mouse input, ACPI resource discovery, international layout, repeat, lock state, другие modifier и extended key вне Left/Right намеренно выходят за bootstrap.

## Источник

[1] [Документация QEMU System emulator](https://www.qemu.org/docs/master/system/qemu-manpage.html)
