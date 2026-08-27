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

    if (!expect(!horizon_build_desktop_scene(319U, 480U, &scene), "too-narrow display is rejected") ||
        !expect(horizon_build_desktop_scene(640U, 480U, &scene), "desktop scene builds") ||
        !expect(scene.rectangle_count == 6U, "desktop has backdrop, header, three cards, and dock") ||
        !expect(scene.rectangles[0].rgb_color == UINT32_C(0x101827), "first rectangle is desktop background") ||
        !expect(scene.rectangles[5].rgb_color == UINT32_C(0x16a8a0), "last rectangle is accent dock") ||
        !expect(horizon_render_desktop(&framebuffer), "desktop scene renders") ||
        !expect(pixels[0] == UINT32_C(0x0018314d), "header overlays desktop at top-left") ||
        !expect(pixels[(450U * 640U) + 100U] == UINT32_C(0x0016a8a0), "dock renders near lower desktop")) {
        return 1;
    }

    puts("Horizon desktop bootstrap unit tests passed.");
    return 0;
}
