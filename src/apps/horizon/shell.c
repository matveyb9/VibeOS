/* VibeOS Horizon — first software desktop composition atop Canvas. */

#include "horizon.h"

#define HORIZON_MINIMUM_WIDTH UINT32_C(320)
#define HORIZON_MINIMUM_HEIGHT UINT32_C(240)

static int horizon_add_rect(
    CANVAS_SCENE *scene,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t color) {
    return canvas_scene_add_rect(scene, (CANVAS_RECT){x, y, width, height, color});
}

int horizon_build_desktop_scene(uint32_t width, uint32_t height, CANVAS_SCENE *scene) {
    uint32_t header_height;
    uint32_t dock_height;
    uint32_t card_width;
    uint32_t card_height;
    uint32_t left_margin;
    uint32_t gap;

    if (scene == (void *)0 || width < HORIZON_MINIMUM_WIDTH || height < HORIZON_MINIMUM_HEIGHT) {
        return 0;
    }
    header_height = height / 12U;
    dock_height = height / 10U;
    card_width = width / 4U;
    card_height = height / 3U;
    left_margin = width / 12U;
    gap = width / 24U;

    canvas_scene_initialize(scene);
    return horizon_add_rect(scene, 0U, 0U, width, height, UINT32_C(0x101827)) &&
           horizon_add_rect(scene, 0U, 0U, width, header_height, UINT32_C(0x18314d)) &&
           horizon_add_rect(scene, left_margin, header_height + gap, card_width, card_height, UINT32_C(0x28536b)) &&
           horizon_add_rect(scene, left_margin + card_width + gap, header_height + (gap * 2U), card_width, card_height,
                            UINT32_C(0x1f3658)) &&
           horizon_add_rect(scene, left_margin + (2U * (card_width + gap)), header_height + gap, card_width, card_height,
                            UINT32_C(0x176a86)) &&
           horizon_add_rect(scene, width / 8U, height - dock_height - gap, (width * 3U) / 4U, dock_height,
                            UINT32_C(0x16a8a0));
}

int horizon_render_desktop(PRISM_FRAMEBUFFER *framebuffer) {
    CANVAS_SCENE scene;

    return framebuffer != (void *)0 &&
           horizon_build_desktop_scene(framebuffer->width, framebuffer->height, &scene) &&
           canvas_scene_render(&scene, framebuffer);
}
