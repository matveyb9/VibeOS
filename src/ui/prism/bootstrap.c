/* VibeOS Prism — early visual handoff and Canvas scene composition. */

#include <canvas.h>
#include <prism.h>

int prism_canvas_runtime_probe(const DAWN_CONTEXT *context) {
    PRISM_FRAMEBUFFER framebuffer;
    CANVAS_SCENE scene;
    uint32_t card_width;
    uint32_t card_height;

    if (!prism_framebuffer_from_dawn(context, &framebuffer) ||
        !prism_fill_rect(&framebuffer, 0U, 0U, framebuffer.width, framebuffer.height, UINT32_C(0x101827))) {
        return 0;
    }
    card_width = framebuffer.width / 2U;
    card_height = framebuffer.height / 3U;
    canvas_scene_initialize(&scene);
    if (!canvas_scene_add_rect(&scene, (CANVAS_RECT){framebuffer.width / 12U, framebuffer.height / 10U,
                                                      card_width, card_height, UINT32_C(0x28536b)}) ||
        !canvas_scene_add_rect(&scene, (CANVAS_RECT){framebuffer.width / 8U, framebuffer.height / 6U,
                                                      card_width, card_height, UINT32_C(0x1f3658)}) ||
        !canvas_scene_add_rect(&scene, (CANVAS_RECT){framebuffer.width / 6U, framebuffer.height / 4U,
                                                      card_width, card_height, UINT32_C(0x16a8a0)})) {
        return 0;
    }
    return canvas_scene_render(&scene, &framebuffer);
}
