/* VibeOS Horizon — early retained desktop-shell scene contract. */

#ifndef VIBEOS_HORIZON_H
#define VIBEOS_HORIZON_H

#include <canvas.h>

int horizon_build_desktop_scene(uint32_t width, uint32_t height, CANVAS_SCENE *scene);
int horizon_render_desktop(PRISM_FRAMEBUFFER *framebuffer);

#endif
