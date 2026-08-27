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
| Начальная key policy | Pressed Tab или Right Arrow выбирает next focus, Shift+Tab или Left Arrow выбирает previous focus, pressed Enter выбирает focused logical window; release и unmapped key игнорируются |
| Event pump | Caller-owned operation извлекает не более восьми queued Atlas keyboard event, сообщает dequeued/handled count и запрашивает redraw только при изменении focus или selection |
| Desktop runtime | Caller-owned runtime владеет одной копией Prism framebuffer и Horizon focus state; он инициализирует scene и композирует один bounded pump-to-redraw step |
| Selection indicator | Selected logical card получает один retained amber strip у нижнего края; до существования selection strip не создаётся |
| Interaction | Live event loop пока не вызывает pump и не перерисовывает framebuffer после изменения state; pointer event и application launch отсутствуют |

`horizon_build_desktop_scene_for_state` принимает valid focus state из трёх window и размещает retained indicator у focused card. Когда selection существует, он добавляет один amber strip у нижнего края selected card; исходный bootstrap builder не создаёт selection. Horizon input adapter — преднамеренная cross-subsystem boundary: он принимает уже normalized `ATLAS_KEY_EVENT`, отображает recognised Tab, Shift+Tab, Left/Right Arrow и Enter press event на существующий focus action и сообщает, был ли event обработан. Его bounded pump — единственный consumer Atlas event на этом layer: он никогда не читает device register, не выделяет память и останавливается на caller-selected budget не больше восьми. Caller-owned desktop runtime объединяет framebuffer copy, focus state, initial render и один pump-to-redraw step, намеренно оставляя wait и IRQ delivery Pulse. Выделенный x86_64 keyboard QEMU profile выполняет controlled two-event session: IRQ1 помещает Right Arrow в queue, runtime перерисовывает focus, затем второй IRQ1 помещает Enter и тот же runtime выбирает persisted focused card, перерисовывает amber strip и завершает proof. Host-тесты проверяют state bound, forward и backward navigation, selection, adapter mapping, partial и complete draining, redraw reporting, sequential runtime lifecycle, scene geometry, selection pixel и rendering order.

> Keyboard QEMU profile — это controlled proof двух event, а не общий live desktop loop. Runtime не ожидает, не настраивает interrupt, не schedule сам себя, не владеет device queue, не управляет window, не запускает process и не применяет permission. Будущая работа будет schedule его вне test profile, coalesce Canvas/Prism redraw, добавлять pointer queue, использовать Keys для window ownership, направлять launch через Parcel и интегрировать настоящие приложения из project architecture.
