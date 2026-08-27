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

int horizon_build_desktop_scene_for_state_and_request(
    uint32_t width, uint32_t height, const HORIZON_DESKTOP_STATE *state, uint32_t native_request_status, CANVAS_SCENE *scene) {
    uint32_t header_height;
    uint32_t dock_height;
    uint32_t card_width;
    uint32_t card_height;
    uint32_t left_margin;
    uint32_t gap;
    uint32_t focus_x;
    uint32_t focus_y;
    uint32_t selected_x;
    uint32_t selected_y;
    const HORIZON_APPLICATION_DESCRIPTOR *first_application;
    const HORIZON_APPLICATION_DESCRIPTOR *second_application;
    const HORIZON_APPLICATION_DESCRIPTOR *third_application;

    if (scene == (void *)0 || width < HORIZON_MINIMUM_WIDTH || height < HORIZON_MINIMUM_HEIGHT ||
        !horizon_desktop_state_is_valid(state) || state->window_count != HORIZON_NATIVE_APPLICATION_COUNT ||
        native_request_status > 4U) {
        return 0;
    }
    first_application = horizon_application_at(0U);
    second_application = horizon_application_at(1U);
    third_application = horizon_application_at(2U);
    if (first_application == (const void *)0 || second_application == (const void *)0 ||
        third_application == (const void *)0) {
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
    selected_x = left_margin + (state->selected_window * (card_width + gap));
    selected_y = header_height + gap + (state->selected_window == 1U ? gap : 0U);

    canvas_scene_initialize(scene);
    if (!horizon_add_rect(scene, 0U, 0U, width, height, UINT32_C(0x101827)) ||
        !horizon_add_rect(scene, 0U, 0U, width, header_height, UINT32_C(0x18314d)) ||
        !horizon_add_rect(scene, focus_x - (gap / 4U), focus_y - (gap / 4U),
                          card_width + (gap / 2U), card_height + (gap / 2U), UINT32_C(0xe6f1ff)) ||
        !horizon_add_rect(scene, left_margin, header_height + gap, card_width, card_height, UINT32_C(0x28536b)) ||
        !horizon_add_rect(scene, left_margin + card_width + gap, header_height + (gap * 2U), card_width, card_height,
                          UINT32_C(0x1f3658)) ||
        !horizon_add_rect(scene, left_margin + (2U * (card_width + gap)), header_height + gap, card_width, card_height,
                          UINT32_C(0x176a86)) ||
        !horizon_add_rect(scene, width / 8U, height - dock_height - gap, (width * 3U) / 4U, dock_height,
                          UINT32_C(0x16a8a0))) {
        return 0;
    }
    if (state->selected_window != HORIZON_DESKTOP_NO_WINDOW &&
        !horizon_add_rect(scene, selected_x + (gap / 4U), selected_y + card_height - (gap / 4U),
                          card_width - (gap / 2U), gap / 6U, UINT32_C(0xffcf5c))) {
        return 0;
    }
    if (!canvas_scene_add_label(scene, gap, gap / 2U, 2U, UINT32_C(0xe6f1ff), "VIBEOS") ||
        !canvas_scene_add_label(scene, left_margin + gap, header_height + (gap * 2U), 2U, UINT32_C(0xe6f1ff), first_application->label) ||
        !canvas_scene_add_label(scene, left_margin + card_width + (gap * 2U), header_height + (gap * 3U), 2U,
                                UINT32_C(0xe6f1ff), second_application->label) ||
        !canvas_scene_add_label(scene, left_margin + (2U * card_width) + (gap * 3U), header_height + (gap * 2U),
                                2U, UINT32_C(0xe6f1ff), third_application->label)) {
        return 0;
    }
    if (native_request_status == HORIZON_NATIVE_REQUEST_NONE) {
        return 1;
    }
    if (native_request_status == HORIZON_NATIVE_REQUEST_FORMED) {
        return canvas_scene_add_label(scene, width / 4U, height - dock_height, 1U, UINT32_C(0xffcf5c), "FORMED");
    }
    if (native_request_status == HORIZON_NATIVE_REQUEST_ADMITTED) {
        return canvas_scene_add_label(scene, width / 4U, height - dock_height, 1U, UINT32_C(0x70e0b6), "ADMITTED");
    }
    if (native_request_status == HORIZON_NATIVE_REQUEST_REJECTED_NOT_INSTALLED) {
        return canvas_scene_add_label(scene, width / 4U, height - dock_height, 1U, UINT32_C(0xff9d68), "NOT INSTALLED");
    }
    return canvas_scene_add_label(scene, width / 4U, height - dock_height, 1U, UINT32_C(0xff7a90), "INVALID");
}

int horizon_build_desktop_scene_for_state(
    uint32_t width, uint32_t height, const HORIZON_DESKTOP_STATE *state, CANVAS_SCENE *scene) {
    return horizon_build_desktop_scene_for_state_and_request(width, height, state, 0U, scene);
}

int horizon_build_desktop_scene(uint32_t width, uint32_t height, CANVAS_SCENE *scene) {
    HORIZON_DESKTOP_STATE desktop;

    return horizon_desktop_state_initialize(&desktop, HORIZON_NATIVE_APPLICATION_COUNT) &&
           horizon_build_desktop_scene_for_state(width, height, &desktop, scene);
}

int horizon_render_desktop_for_state(PRISM_FRAMEBUFFER *framebuffer, const HORIZON_DESKTOP_STATE *state) {
    return horizon_render_desktop_for_state_and_request(framebuffer, state, 0U);
}

int horizon_render_desktop_for_state_and_request(
    PRISM_FRAMEBUFFER *framebuffer, const HORIZON_DESKTOP_STATE *state, uint32_t native_request_status) {
    CANVAS_SCENE scene;

    return framebuffer != (void *)0 &&
           horizon_build_desktop_scene_for_state_and_request(
               framebuffer->width, framebuffer->height, state, native_request_status, &scene) &&
           canvas_scene_render(&scene, framebuffer);
}

int horizon_render_desktop(PRISM_FRAMEBUFFER *framebuffer) {
    HORIZON_DESKTOP_STATE desktop;

    return horizon_desktop_state_initialize(&desktop, HORIZON_NATIVE_APPLICATION_COUNT) &&
           horizon_render_desktop_for_state(framebuffer, &desktop);
}
