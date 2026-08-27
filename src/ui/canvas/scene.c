/* VibeOS Canvas — bounded retained rectangle scene renderer. */

#include "canvas.h"

void canvas_scene_initialize(CANVAS_SCENE *scene) {
    uint32_t index;

    if (scene != (void *)0) {
        for (index = 0; index < CANVAS_SCENE_CAPACITY; ++index) {
            scene->rectangles[index].x = 0;
            scene->rectangles[index].y = 0;
            scene->rectangles[index].width = 0;
            scene->rectangles[index].height = 0;
            scene->rectangles[index].rgb_color = 0;
        }
        scene->rectangle_count = 0;
    }
}

int canvas_scene_add_rect(CANVAS_SCENE *scene, CANVAS_RECT rectangle) {
    if (scene == (void *)0 || rectangle.width == 0U || rectangle.height == 0U ||
        scene->rectangle_count >= CANVAS_SCENE_CAPACITY) {
        return 0;
    }
    scene->rectangles[scene->rectangle_count] = rectangle;
    ++scene->rectangle_count;
    return 1;
}

int canvas_scene_render(const CANVAS_SCENE *scene, PRISM_FRAMEBUFFER *framebuffer) {
    uint32_t index;

    if (scene == (void *)0 || framebuffer == (void *)0) {
        return 0;
    }
    for (index = 0; index < scene->rectangle_count; ++index) {
        CANVAS_RECT rectangle = scene->rectangles[index];

        if (!prism_fill_rect(
                framebuffer,
                rectangle.x,
                rectangle.y,
                rectangle.width,
                rectangle.height,
                rectangle.rgb_color)) {
            return 0;
        }
    }
    return 1;
}
