<p align="center">
  <a href="../../en/specs/PRISM_CANVAS_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Prism и Canvas

**Статус:** Реализован как ранний x86_64 software-rendering path в QEMU UEFI и Legacy BIOS profile. Это ещё не user-space compositor, запланированный для VibeOS v1.0.

UEFI Prelude запрашивает firmware Graphics Output Protocol до выхода из Boot Services; Legacy BIOS Prelude выбирает VBE linear mode, пока доступны BIOS service. Оба producer добавляют base активного framebuffer, размер, dimensions, scan-line stride и format `RGBX8888`, `BGRX8888` или `BGR888` в append-only Dawn Context v3. После захвата `CR3` Pulse проверяет descriptor; его временный identity map на четыре ГиБ охватывает QEMU framebuffer range. GOP предоставляет framebuffer, управляемый display output, а также pixel format и scan-line information.[1]

| Слой | Начальная ответственность |
|---|---|
| Prelude | Получить GOP information до `ExitBootServices()` или выбрать Legacy BIOS VBE linear mode до firmware handoff |
| Dawn Context v3 | Передать physical framebuffer descriptor без передачи live UEFI или BIOS service |
| Prism | Проверить framebuffer и software-fill clipped RGB rectangle |
| Canvas | Хранить и рендерить bounded ordered rectangle и uppercase bitmap label |
| Pulse probe | Нарисовать dark background и три overlapping visual block до timer verification |

Host-тест проверяет descriptor handling, clipping, byte packing `BGR888`, draw order, uppercase bitmap label и retained-scene rendering. QEMU UEFI и Legacy BIOS probe подтверждают выполнение software paint path до доставки external timer. Следующая работа отделит Prism в user space, создаст surface и window composition, добавит scalable text и input, подключит display driver через Atlas и уберёт временный broad identity map.

## Источник

[1] [UEFI Specification 2.10, Graphics Output Protocol](https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html)
