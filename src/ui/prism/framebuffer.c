/* VibeOS Prism — first direct software painter for a Dawn framebuffer. */

#include "prism.h"

static int prism_framebuffer_layout_valid(const PRISM_FRAMEBUFFER *framebuffer) {
    uint64_t required_bytes;

    if (framebuffer == (void *)0 || framebuffer->pixels == (void *)0 || framebuffer->width == 0U ||
        framebuffer->height == 0U || framebuffer->pixels_per_scan_line < framebuffer->width ||
        (framebuffer->pixel_format != DAWN_PIXEL_FORMAT_RGBX8888 &&
         framebuffer->pixel_format != DAWN_PIXEL_FORMAT_BGRX8888)) {
        return 0;
    }
    required_bytes = (uint64_t)framebuffer->pixels_per_scan_line * (uint64_t)framebuffer->height * 4U;
    return required_bytes <= framebuffer->byte_size;
}

int prism_framebuffer_from_dawn(const DAWN_CONTEXT *context, PRISM_FRAMEBUFFER *framebuffer) {
    if (context == (void *)0 || framebuffer == (void *)0) {
        return 0;
    }
    framebuffer->pixels = (uint8_t *)(uintptr_t)context->framebuffer_physical_address;
    framebuffer->byte_size = context->framebuffer_byte_size;
    framebuffer->width = context->framebuffer_width;
    framebuffer->height = context->framebuffer_height;
    framebuffer->pixels_per_scan_line = context->framebuffer_pixels_per_scan_line;
    framebuffer->pixel_format = (DAWN_PIXEL_FORMAT)context->framebuffer_pixel_format;
    return prism_framebuffer_layout_valid(framebuffer);
}

static uint32_t prism_pack_pixel(const PRISM_FRAMEBUFFER *framebuffer, uint32_t rgb_color) {
    uint32_t red = (rgb_color >> 16U) & UINT32_C(0xff);
    uint32_t green = (rgb_color >> 8U) & UINT32_C(0xff);
    uint32_t blue = rgb_color & UINT32_C(0xff);

    if (framebuffer->pixel_format == DAWN_PIXEL_FORMAT_RGBX8888) {
        return red | (green << 8U) | (blue << 16U);
    }
    return blue | (green << 8U) | (red << 16U);
}

int prism_fill_rect(
    PRISM_FRAMEBUFFER *framebuffer,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t rgb_color) {
    uint32_t end_x;
    uint32_t end_y;
    uint32_t row;
    uint32_t column;
    uint32_t packed_color;
    uint32_t *row_pixels;

    if (!prism_framebuffer_layout_valid(framebuffer) || width == 0U || height == 0U || x >= framebuffer->width ||
        y >= framebuffer->height) {
        return 0;
    }
    end_x = width > framebuffer->width - x ? framebuffer->width : x + width;
    end_y = height > framebuffer->height - y ? framebuffer->height : y + height;
    packed_color = prism_pack_pixel(framebuffer, rgb_color);
    for (row = y; row < end_y; ++row) {
        row_pixels = (uint32_t *)framebuffer->pixels + ((uint64_t)row * framebuffer->pixels_per_scan_line) + x;
        for (column = x; column < end_x; ++column) {
            row_pixels[column - x] = packed_color;
        }
    }
    return 1;
}
