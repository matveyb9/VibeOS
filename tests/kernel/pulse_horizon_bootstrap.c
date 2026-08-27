/* VibeOS Horizon — host checks for deterministic desktop scene composition. */

#include <stdint.h>
#include <stdio.h>

#include <horizon.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    uint32_t pixels[640U * 480U] = {0};
    PRISM_FRAMEBUFFER framebuffer = {
        (uint8_t *)pixels,
        sizeof(pixels),
        640U,
        480U,
        640U,
        DAWN_PIXEL_FORMAT_BGRX8888,
    };
    CANVAS_SCENE scene;
    HORIZON_DESKTOP_STATE focus_state;
    const HORIZON_APPLICATION_DESCRIPTOR *application;

    if (!expect(!horizon_build_desktop_scene(319U, 480U, &scene), "too-narrow display is rejected") ||
        !expect(horizon_build_desktop_scene(640U, 480U, &scene), "desktop scene builds") ||
        !expect(scene.rectangle_count == 7U && scene.label_count == 4U,
                    "desktop has retained regions and named labels") ||
        !expect(scene.rectangles[0].rgb_color == UINT32_C(0x101827), "first rectangle is desktop background") ||
        !expect(scene.rectangles[2].rgb_color == UINT32_C(0xe6f1ff), "initial focus indicator precedes cards") ||
        !expect(scene.rectangles[6].rgb_color == UINT32_C(0x16a8a0), "last rectangle is accent dock") ||
        !expect(horizon_desktop_state_initialize(&focus_state, 3U) &&
                    horizon_desktop_apply_action(&focus_state, HORIZON_DESKTOP_ACTION_FOCUS_NEXT) &&
                    horizon_build_desktop_scene_for_state(640U, 480U, &focus_state, &scene) &&
                    scene.rectangles[2].x == 233U && scene.rectangles[2].y == 86U,
                "focus state positions the retained indicator at the focused card") ||
        !expect(horizon_render_desktop_for_state(&framebuffer, &focus_state) &&
                    pixels[(86U * 640U) + 233U] == UINT32_C(0x00e6f1ff),
                "focused state renders its indicator at the retained geometry") ||
        !expect(horizon_desktop_apply_action(&focus_state, HORIZON_DESKTOP_ACTION_SELECT_FOCUSED) &&
                    horizon_build_desktop_scene_for_state(640U, 480U, &focus_state, &scene) &&
                    scene.rectangle_count == 8U && scene.rectangles[7].rgb_color == UINT32_C(0xffcf5c) &&
                    scene.rectangles[7].x == 245U && scene.rectangles[7].y == 246U,
                "selected focus adds an amber strip at the selected card") ||
        !expect(horizon_application_at(0U) != (const void *)0 && horizon_application_at(0U)->id == HORIZON_APPLICATION_PROMPT &&
                    horizon_application_at(1U) != (const void *)0 && horizon_application_at(1U)->id == HORIZON_APPLICATION_CUE &&
                    horizon_application_at(2U) != (const void *)0 && horizon_application_at(2U)->id == HORIZON_APPLICATION_VECTOR &&
                    horizon_application_at(HORIZON_NATIVE_APPLICATION_COUNT) == (const void *)0,
                "catalog maps bounded desktop cards to native application IDs") ||
        !expect(horizon_selected_application(&focus_state, &application) && application->id == HORIZON_APPLICATION_CUE &&
                    application->label[0] == 'C',
                "selected card resolves to the persisted native application descriptor") ||
        !expect(horizon_render_desktop_for_state(&framebuffer, &focus_state) &&
                    pixels[(246U * 640U) + 245U] == UINT32_C(0x00ffcf5c),
                "selected state renders the amber strip") ||
        !expect(horizon_render_desktop(&framebuffer), "desktop scene renders") ||
        !expect(pixels[0] == UINT32_C(0x0018314d), "header overlays desktop at top-left") ||
        !expect(pixels[(450U * 640U) + 100U] == UINT32_C(0x0016a8a0), "dock renders near lower desktop")) {
        return 1;
    }

    puts("Horizon desktop bootstrap unit tests passed.");
    return 0;
}
