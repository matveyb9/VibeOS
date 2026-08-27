<p align="center">
  <a href="../../en/specs/HORIZON_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap desktop Horizon

**Статус:** Реализован как начальная retained desktop scene. Input, text, interactive window и application process пока не реализованы.

Теперь Horizon композирует первый видимый desktop VibeOS из Canvas rectangle, а не показывает неструктурированный framebuffer test. Scene использует deep-blue workspace, верхнюю status band, три независимые window card и нижний accent dock. Это преднамеренная desktop-shell boundary: Horizon выбирает visual arrangement, Canvas хранит ordered primitive, а Prism записывает проверенный framebuffer.

| Элемент | Начальное поведение |
|---|---|
| Minimum target | 320 × 240 pixel |
| Scene model | Шесть bounded retained rectangle |
| Workspace | Фон всего display |
| Window card | Три staggered desktop region |
| Dock | Нижняя accent band шириной три четверти display |
| Interaction | Пока недоступна |

Host-тесты проверяют rejection недостаточного target size, deterministic scene count, visual order, top-band overlay и dock pixel. UEFI QEMU path теперь рендерит эту desktop scene до сообщения timer proof. Будущая работа Horizon добавит Canvas text, cursor и keyboard input, window ownership через Key, app launch через Parcel и настоящие приложения, названные в project architecture.
