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
| Initial key policy | Pressed `N` selects next focus, pressed `P` selects previous focus, and pressed Space selects the focused logical window; releases and unmapped keys are ignored |
| Interaction | The adapter is not yet connected to a live keyboard drain loop or pointer events, and selection launches nothing |

`horizon_build_desktop_scene_for_state` accepts a validated three-window focus state and positions the retained indicator at its focused card; the original bootstrap builder still creates a deterministic initial state. The Horizon input adapter is the deliberate cross-subsystem boundary: it accepts an already-normalized `ATLAS_KEY_EVENT`, maps only recognised press events into an existing focus action, and reports whether the event was handled. Host tests verify state bounds, wraparound navigation, selection, adapter mapping, scene geometry, and rendering order. The UEFI and Legacy BIOS QEMU paths verify the isolated focus and input-adapter self-checks before their timer proof.

> The focus model and input adapter are not a live event loop, window manager, process launcher, or permission mechanism. Future Horizon work will drain Atlas keyboard/pointer queues, use Keys for window ownership, route launches through Parcel, and integrate the actual applications named in the project architecture.
