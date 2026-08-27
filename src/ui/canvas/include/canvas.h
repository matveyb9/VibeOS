/* VibeOS Canvas — retained-mode rectangle scene bootstrap. */

#ifndef VIBEOS_CANVAS_H
#define VIBEOS_CANVAS_H

#include <prism.h>

#define CANVAS_SCENE_CAPACITY UINT32_C(16)

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t rgb_color;
} CANVAS_RECT;

typedef struct {
    CANVAS_RECT rectangles[CANVAS_SCENE_CAPACITY];
    uint32_t rectangle_count;
} CANVAS_SCENE;

void canvas_scene_initialize(CANVAS_SCENE *scene);
int canvas_scene_add_rect(CANVAS_SCENE *scene, CANVAS_RECT rectangle);
int canvas_scene_render(const CANVAS_SCENE *scene, PRISM_FRAMEBUFFER *framebuffer);

#endif
