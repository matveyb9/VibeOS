/* VibeOS Prism — early software framebuffer boundary. */

#ifndef VIBEOS_PRISM_H
#define VIBEOS_PRISM_H

#include <dawn.h>

typedef struct {
    uint8_t *pixels;
    uint64_t byte_size;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scan_line;
    DAWN_PIXEL_FORMAT pixel_format;
} PRISM_FRAMEBUFFER;

int prism_framebuffer_from_dawn(const DAWN_CONTEXT *context, PRISM_FRAMEBUFFER *framebuffer);
int prism_fill_rect(
    PRISM_FRAMEBUFFER *framebuffer,
    uint32_t x,
    uint32_t y,
    uint32_t width,
    uint32_t height,
    uint32_t rgb_color);

#endif
