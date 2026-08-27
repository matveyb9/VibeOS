<p align="center">
  <a href="../../en/specs/PRISM_CANVAS_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap Prism и Canvas

**Статус:** Реализован как ранний x86_64 UEFI software-rendering path. Это ещё не user-space compositor, запланированный для VibeOS v1.0.

Prelude запрашивает firmware Graphics Output Protocol до выхода из Boot Services и добавляет base активного framebuffer, размер, dimensions, scan-line stride и RGBX/BGRX format в append-only Dawn Context v2. После захвата `CR3` Pulse проверяет descriptor; его временный identity map на четыре ГиБ охватывает QEMU framebuffer range. GOP предоставляет framebuffer, управляемый display output, а также pixel format и scan-line information.[1]

| Слой | Начальная ответственность |
|---|---|
| Prelude | Получить current GOP framebuffer information до `ExitBootServices()` |
| Dawn Context v2 | Передать physical framebuffer descriptor без передачи live UEFI service |
| Prism | Проверить framebuffer и software-fill clipped RGB rectangle |
| Canvas | Хранить и рендерить bounded ordered rectangle и uppercase bitmap label |
| Pulse probe | Нарисовать dark background и три overlapping visual block до timer verification |

Host-тест проверяет descriptor handling, clipping, BGR pixel packing, draw order, uppercase bitmap label и retained-scene rendering. QEMU probe подтверждает выполнение software paint path до доставки external timer. Следующая работа отделит Prism в user space, создаст surface и window composition, добавит scalable text и input, подключит display driver через Atlas и уберёт временный broad identity map.

## Источник

[1] [UEFI Specification 2.10, Graphics Output Protocol](https://uefi.org/specs/UEFI/2.10/12_Protocols_Console_Support.html)
