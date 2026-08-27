<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/HORIZON_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Horizon desktop bootstrap

**Status:** Implemented as an initial retained desktop scene. Input, text, interactive windows, and application processes are not yet implemented.

Horizon now composes the first visible VibeOS desktop from Canvas rectangles rather than presenting an unstructured framebuffer test. The scene uses a deep blue workspace, a top status band, three independent window cards, and a low accent dock. It is a deliberate desktop-shell boundary: Horizon selects visual arrangement, Canvas retains ordered primitives, and Prism writes the validated framebuffer.

| Element | Initial behavior |
|---|---|
| Minimum target | 320 × 240 pixels |
| Scene model | Six bounded retained rectangles |
| Workspace | Full display background |
| Window cards | Three staggered desktop regions |
| Dock | Lower three-quarter-width accent band |
| Interaction | Not yet available |

Host tests verify target-size rejection, deterministic scene count, visual order, top-band overlay, and dock pixels. The UEFI QEMU path now renders this desktop scene before it reports its timer proof. Future Horizon work will add Canvas text, cursor and keyboard input, window ownership through Keys, app launch through Parcel, and the actual applications named in the project architecture.
