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
| Event pump | Caller-owned operation извлекает не более восьми queued Atlas keyboard event, сообщает dequeued/handled count и запрашивает redraw только при изменении focus или selection |
| Interaction | Live event loop пока не вызывает pump и не перерисовывает framebuffer после изменения state; pointer event и application launch отсутствуют |

`horizon_build_desktop_scene_for_state` принимает valid focus state из трёх window и размещает retained indicator у focused card; исходный bootstrap builder по-прежнему создаёт deterministic initial state. Horizon input adapter — преднамеренная cross-subsystem boundary: он принимает уже normalized `ATLAS_KEY_EVENT`, отображает только recognised press event на существующий focus action и сообщает, был ли event обработан. Его bounded pump — единственный consumer Atlas event на этом layer: он никогда не читает device register, не выделяет память и останавливается на caller-selected budget не больше восьми. Host-тесты проверяют state bound, wraparound navigation, selection, adapter mapping, partial и complete draining, redraw reporting, scene geometry и rendering order. UEFI и Legacy BIOS QEMU path проверяют isolated focus, adapter и pump self-check до timer proof.

> Focus model, input adapter и pump не являются live event loop, window manager, framebuffer redraw loop, process launcher или permission mechanism. Будущая работа Horizon будет schedule pump, запрашивать Canvas/Prism redraw после state change, добавлять pointer queue, использовать Keys для window ownership, направлять launch через Parcel и интегрировать настоящие приложения из project architecture.
