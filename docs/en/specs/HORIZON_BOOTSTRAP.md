<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/HORIZON_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Horizon desktop bootstrap

**Status:** Implemented as an initial retained desktop scene with a bounded, deterministic focus model. Hardware-input binding, text editing, interactive windows, and application processes are not yet implemented.

Horizon now composes the first visible VibeOS desktop from Canvas rectangles and bitmap labels rather than presenting an unstructured framebuffer test. The scene uses a deep blue workspace, a top status band, three independent window cards, a low accent dock, and the visible names `VIBEOS`, `HORIZON`, `GUIDE`, and `PROMPT`. It is a deliberate desktop-shell boundary: Horizon selects visual arrangement, Canvas retains ordered primitives, and Prism writes the validated framebuffer.

| Element | Initial behavior |
|---|---|
| Minimum target | 320 × 240 pixels |
| Scene model | Seven bounded retained rectangles and four labels; the additional rectangle is the focus indicator |
| Workspace | Full display background |
| Window cards | Three staggered desktop regions |
| Dock | Lower three-quarter-width accent band |
| Focus model | A separate state object tracks up to eight logical windows, explicit focus, and optional selection |
| Actions | Deterministic next, previous, and select-focused actions; focus wraps within the bounded window count |
| Input adapter | A separate stateless Horizon–Atlas adapter consumes normalized key events; it owns no hardware queue or rendering state |
| Initial key policy | Pressed Tab or Right Arrow selects next focus, Shift+Tab or Left Arrow selects previous focus, and pressed Enter selects the focused logical window; releases and unmapped keys are ignored |
| Event pump | A caller-owned operation drains at most eight queued Atlas keyboard events, reports dequeued/handled counts, and requests redraw only when focus or selection changed |
| Desktop runtime | A caller-owned runtime owns one Prism framebuffer copy and Horizon focus state; it initializes the scene and composes one bounded pump-to-redraw step |
| Selection indicator | A selected logical card receives one retained amber strip near its lower edge; no strip is emitted before a selection exists |
| Interaction | No live event loop invokes the pump or redraws the framebuffer after a state change; pointer events and application launch remain unavailable |

`horizon_build_desktop_scene_for_state` accepts a validated three-window focus state and positions the retained indicator at its focused card. When selection exists, it adds one amber strip inside the selected card's lower edge; the original bootstrap builder creates no selection. The Horizon input adapter is the deliberate cross-subsystem boundary: it accepts an already-normalized `ATLAS_KEY_EVENT`, maps recognised Tab, Shift+Tab, Left/Right Arrow, and Enter press events into an existing focus action, and reports whether the event was handled. Its bounded pump is the only consumer of Atlas events in this layer: it never reads a device register, does not allocate, and stops at a caller-selected budget no greater than eight. The caller-owned desktop runtime combines the framebuffer copy, focus state, initial render, and one pump-to-redraw step while deliberately leaving waiting and IRQ delivery to Pulse. The dedicated x86_64 keyboard QEMU profile performs a controlled two-event session: IRQ1 queues Right Arrow and the runtime redraws focus, then a second IRQ1 queues Enter and the same runtime selects the persisted focused card, redraws its amber strip, and exits. Host tests verify state bounds, forward and backward navigation, selection, adapter mapping, partial and complete draining, redraw reporting, sequential runtime lifecycle, scene geometry, selection pixels, and rendering order.

> The keyboard QEMU profile is a controlled two-event proof, not a general live desktop loop. The runtime does not wait, configure interrupts, schedule itself, own device queues, operate windows, launch processes, or enforce permissions. Future work will schedule it outside the test profile, coalesce Canvas/Prism redraws, add pointer queues, use Keys for window ownership, route launches through Parcel, and integrate the actual applications named in the project architecture.
