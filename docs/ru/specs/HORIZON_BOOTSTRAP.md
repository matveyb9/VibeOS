<p align="center">
  <a href="../../en/specs/HORIZON_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap desktop Horizon

**Статус:** Реализован как начальная retained desktop scene с bounded deterministic focus model. Hardware-input binding, text editing, interactive window и application process пока не реализованы.

Теперь Horizon композирует первый видимый desktop VibeOS из Canvas rectangle и bitmap label, а не показывает неструктурированный framebuffer test. Scene использует deep-blue workspace, верхнюю status band, три независимые window card, нижний accent dock и видимые имена `VIBEOS`, `HORIZON`, `GUIDE`, `PROMPT`. Это преднамеренная desktop-shell boundary: Horizon выбирает visual arrangement, Canvas хранит ordered primitive, а Prism записывает проверенный framebuffer.

| Элемент | Начальное поведение |
|---|---|
| Minimum target | 320 × 240 pixel |
| Scene model | Семь bounded retained rectangle и четыре label; дополнительный rectangle — focus indicator |
| Workspace | Фон всего display |
| Window card | Три staggered desktop region |
| Dock | Нижняя accent band шириной три четверти display |
| Focus model | Отдельный state object хранит до восьми logical window, explicit focus и optional selection |
| Action | Deterministic action next, previous и select-focused; focus циклически переходит в пределах bounded window count |
| Interaction | Model пока не связана с Atlas keyboard или pointer event; selection ничего не запускает |

`horizon_build_desktop_scene_for_state` принимает valid focus state из трёх window и размещает retained indicator у focused card; исходный bootstrap builder по-прежнему создаёт deterministic initial state. Host-тесты проверяют state bound, wraparound navigation, selection, scene geometry и rendering order. UEFI и Legacy BIOS QEMU path проверяют isolated focus self-check до timer proof.

> Focus model не является event loop, window manager, process launcher или permission mechanism. Будущая работа Horizon свяжет Atlas keyboard/pointer input, использует Keys для window ownership, направит launch через Parcel и интегрирует настоящие приложения из project architecture.
