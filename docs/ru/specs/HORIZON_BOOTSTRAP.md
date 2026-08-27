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
| Input adapter | Отдельный stateless Horizon–Atlas adapter принимает normalized key event; он не владеет hardware queue или rendering state |
| Начальная key policy | Pressed `N` выбирает next focus, pressed `P` — previous focus, pressed Space выбирает focused logical window; release и unmapped key игнорируются |
| Interaction | Adapter пока не связан с live keyboard drain loop или pointer event; selection ничего не запускает |

`horizon_build_desktop_scene_for_state` принимает valid focus state из трёх window и размещает retained indicator у focused card; исходный bootstrap builder по-прежнему создаёт deterministic initial state. Horizon input adapter — преднамеренная cross-subsystem boundary: он принимает уже normalized `ATLAS_KEY_EVENT`, отображает только recognised press event на существующий focus action и сообщает, был ли event обработан. Host-тесты проверяют state bound, wraparound navigation, selection, adapter mapping, scene geometry и rendering order. UEFI и Legacy BIOS QEMU path проверяют isolated focus и input-adapter self-check до timer proof.

> Focus model и input adapter не являются live event loop, window manager, process launcher или permission mechanism. Будущая работа Horizon будет извлекать events из Atlas keyboard/pointer queue, использовать Keys для window ownership, направлять launch через Parcel и интегрировать настоящие приложения из project architecture.
