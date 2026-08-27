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

int horizon_build_desktop_scene_for_state(
    uint32_t width, uint32_t height, const HORIZON_DESKTOP_STATE *state, CANVAS_SCENE *scene) {
    uint32_t header_height;
    uint32_t dock_height;
    uint32_t card_width;
    uint32_t card_height;
    uint32_t left_margin;
    uint32_t gap;
    uint32_t focus_x;
    uint32_t focus_y;

    if (scene == (void *)0 || width < HORIZON_MINIMUM_WIDTH || height < HORIZON_MINIMUM_HEIGHT ||
        !horizon_desktop_state_is_valid(state) || state->window_count != 3U) {
        return 0;
    }
    header_height = height / 12U;
    dock_height = height / 10U;
    card_width = width / 4U;
    card_height = height / 3U;
    left_margin = width / 12U;
    gap = width / 24U;
    focus_x = left_margin + (state->focused_window * (card_width + gap));
    focus_y = header_height + gap + (state->focused_window == 1U ? gap : 0U);

    canvas_scene_initialize(scene);
    return horizon_add_rect(scene, 0U, 0U, width, height, UINT32_C(0x101827)) &&
           horizon_add_rect(scene, 0U, 0U, width, header_height, UINT32_C(0x18314d)) &&
           horizon_add_rect(scene, focus_x - (gap / 4U), focus_y - (gap / 4U),
                            card_width + (gap / 2U), card_height + (gap / 2U), UINT32_C(0xe6f1ff)) &&
           horizon_add_rect(scene, left_margin, header_height + gap, card_width, card_height, UINT32_C(0x28536b)) &&
           horizon_add_rect(scene, left_margin + card_width + gap, header_height + (gap * 2U), card_width, card_height,
                            UINT32_C(0x1f3658)) &&
           horizon_add_rect(scene, left_margin + (2U * (card_width + gap)), header_height + gap, card_width, card_height,
                            UINT32_C(0x176a86)) &&
           horizon_add_rect(scene, width / 8U, height - dock_height - gap, (width * 3U) / 4U, dock_height,
                            UINT32_C(0x16a8a0)) &&
           canvas_scene_add_label(scene, gap, gap / 2U, 2U, UINT32_C(0xe6f1ff), "VIBEOS") &&
           canvas_scene_add_label(scene, left_margin + gap, header_height + (gap * 2U), 2U, UINT32_C(0xe6f1ff), "HORIZON") &&
           canvas_scene_add_label(scene, left_margin + card_width + (gap * 2U), header_height + (gap * 3U), 2U,
                                  UINT32_C(0xe6f1ff), "GUIDE") &&
           canvas_scene_add_label(scene, left_margin + (2U * card_width) + (gap * 3U), header_height + (gap * 2U),
                                  2U, UINT32_C(0xe6f1ff), "PROMPT");
}

int horizon_build_desktop_scene(uint32_t width, uint32_t height, CANVAS_SCENE *scene) {
    HORIZON_DESKTOP_STATE desktop;

    return horizon_desktop_state_initialize(&desktop, 3U) &&
           horizon_build_desktop_scene_for_state(width, height, &desktop, scene);
}

int horizon_render_desktop_for_state(PRISM_FRAMEBUFFER *framebuffer, const HORIZON_DESKTOP_STATE *state) {
    CANVAS_SCENE scene;

    return framebuffer != (void *)0 &&
           horizon_build_desktop_scene_for_state(framebuffer->width, framebuffer->height, state, &scene) &&
           canvas_scene_render(&scene, framebuffer);
}

int horizon_render_desktop(PRISM_FRAMEBUFFER *framebuffer) {
    HORIZON_DESKTOP_STATE desktop;

    return horizon_desktop_state_initialize(&desktop, 3U) && horizon_render_desktop_for_state(framebuffer, &desktop);
}
