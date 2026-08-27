/* VibeOS Prism/Canvas — host checks for pixels, clipping, and retained scenes. */

#include <stdint.h>
#include <stdio.h>

#include <canvas.h>
#include <prism.h>

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    uint32_t pixels[48] = {0};
    uint8_t pixels_bgr888[18] = {0};
    DAWN_CONTEXT context = {0};
    PRISM_FRAMEBUFFER framebuffer;
    PRISM_FRAMEBUFFER framebuffer_bgr888 = {
        pixels_bgr888,
        sizeof(pixels_bgr888),
        2U,
        3U,
        2U,
        DAWN_PIXEL_FORMAT_BGR888,
    };
    CANVAS_SCENE scene;

    context.framebuffer_physical_address = (uint64_t)(uintptr_t)pixels;
    context.framebuffer_byte_size = sizeof(pixels);
    context.framebuffer_width = 6U;
    context.framebuffer_height = 8U;
    context.framebuffer_pixels_per_scan_line = 6U;
    context.framebuffer_pixel_format = DAWN_PIXEL_FORMAT_BGRX8888;

    if (!expect(prism_framebuffer_from_dawn(&context, &framebuffer), "Dawn framebuffer validates") ||
        !expect(prism_fill_rect(&framebuffer, 1U, 1U, 2U, 2U, UINT32_C(0x123456)),
                    "Prism paints rectangle") ||
        !expect(pixels[7] == UINT32_C(0x123456) && pixels[14] == UINT32_C(0x123456),
                    "BGR framebuffer stores expected packed color") ||
        !expect(prism_fill_rect(&framebuffer, 5U, 7U, 2U, 2U, UINT32_C(0xabcdef)),
                    "Prism clips out-of-bounds rectangle") ||
        !expect(pixels[47] == UINT32_C(0xabcdef), "clipped pixel reaches final framebuffer cell") ||
        !expect(prism_fill_rect(&framebuffer_bgr888, 0U, 0U, 1U, 1U, UINT32_C(0x123456)) &&
                    pixels_bgr888[0] == UINT8_C(0x56) && pixels_bgr888[1] == UINT8_C(0x34) &&
                    pixels_bgr888[2] == UINT8_C(0x12), "Prism stores BGR888 framebuffer pixels")) {
        return 1;
    }

    canvas_scene_initialize(&scene);
    if (!expect(canvas_scene_add_rect(&scene, (CANVAS_RECT){0U, 0U, 1U, 1U, UINT32_C(0xff0000)}),
                    "Canvas retains first rectangle") ||
        !expect(canvas_scene_add_rect(&scene, (CANVAS_RECT){1U, 0U, 1U, 1U, UINT32_C(0x00ff00)}),
                    "Canvas retains second rectangle") ||
        !expect(canvas_scene_render(&scene, &framebuffer), "Canvas renders retained scene") ||
        !expect(pixels[0] == UINT32_C(0x00ff0000) && pixels[1] == UINT32_C(0x00ff00),
                    "Canvas preserves draw order and pixel conversion") ||
        !expect(canvas_scene_add_label(&scene, 1U, 0U, 1U, UINT32_C(0xffffff), "V"),
                    "Canvas retains label") ||
        !expect(canvas_scene_render(&scene, &framebuffer) && pixels[1] == UINT32_C(0xffffff),
                    "Canvas renders bitmap label")) {
        return 1;
    }

    puts("Prism and Canvas bootstrap unit tests passed.");
    return 0;
}
