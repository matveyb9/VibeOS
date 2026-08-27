/* VibeOS Canvas — bounded retained rectangles and uppercase bitmap labels. */

#include "canvas.h"

static uint8_t canvas_glyph_row(char character, uint32_t row) {
    static const uint8_t glyphs[][8] = {
        {'A', 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
        {'B', 0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e},
        {'C', 0x0f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0f},
        {'D', 0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e},
        {'E', 0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f},
        {'G', 0x0f, 0x10, 0x10, 0x17, 0x11, 0x11, 0x0f},
        {'H', 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
        {'I', 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1f},
        {'M', 0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11},
        {'N', 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
        {'O', 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
        {'P', 0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10},
        {'R', 0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11},
        {'S', 0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e},
        {'T', 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
        {'U', 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
        {'V', 0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04},
        {'Z', 0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f},
    };
    uint32_t index;

    if (row >= 7U) {
        return 0U;
    }
    for (index = 0; index < sizeof(glyphs) / sizeof(glyphs[0]); ++index) {
        if (glyphs[index][0] == (uint8_t)character) {
            return glyphs[index][row + 1U];
        }
    }
    return 0U;
}

static int canvas_draw_label(const CANVAS_LABEL *label, PRISM_FRAMEBUFFER *framebuffer) {
    uint32_t character_index;
    uint32_t row;
    uint32_t column;
    uint32_t cursor_x;
    uint8_t glyph_row;

    if (label == (void *)0 || label->scale == 0U) {
        return 0;
    }
    cursor_x = label->x;
    for (character_index = 0; character_index < CANVAS_LABEL_BYTES; ++character_index) {
        if (label->text[character_index] == '\0') {
            return character_index != 0U;
        }
        for (row = 0; row < 7U; ++row) {
            glyph_row = canvas_glyph_row(label->text[character_index], row);
            for (column = 0; column < 5U; ++column) {
                if ((glyph_row & (UINT8_C(0x10) >> column)) != 0U &&
                    !prism_fill_rect(framebuffer, cursor_x + (column * label->scale),
                                     label->y + (row * label->scale), label->scale, label->scale,
                                     label->rgb_color)) {
                    return 0;
                }
            }
        }
        cursor_x += 6U * label->scale;
    }
    return 0;
}

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
        for (index = 0; index < CANVAS_LABEL_CAPACITY; ++index) {
            scene->labels[index].x = 0;
            scene->labels[index].y = 0;
            scene->labels[index].scale = 0;
            scene->labels[index].rgb_color = 0;
            scene->labels[index].text[0] = '\0';
        }
        scene->label_count = 0;
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

int canvas_scene_add_label(
    CANVAS_SCENE *scene,
    uint32_t x,
    uint32_t y,
    uint32_t scale,
    uint32_t rgb_color,
    const char *text) {
    CANVAS_LABEL *label;
    uint32_t index;

    if (scene == (void *)0 || text == (void *)0 || text[0] == '\0' || scale == 0U ||
        scene->label_count >= CANVAS_LABEL_CAPACITY) {
        return 0;
    }
    label = &scene->labels[scene->label_count];
    label->x = x;
    label->y = y;
    label->scale = scale;
    label->rgb_color = rgb_color;
    for (index = 0; index < CANVAS_LABEL_BYTES; ++index) {
        label->text[index] = text[index];
        if (text[index] == '\0') {
            ++scene->label_count;
            return 1;
        }
    }
    label->text[0] = '\0';
    return 0;
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
    for (index = 0; index < scene->label_count; ++index) {
        if (!canvas_draw_label(&scene->labels[index], framebuffer)) {
            return 0;
        }
    }
    return 1;
}
