/*
 * VibeOS Dawn Context — platform-neutral boot contract between Prelude and Pulse.
 *
 * ABI rule: this layout is append-only. Fields store physical addresses and sizes
 * rather than pointers whose representation could vary on later architectures.
 */

#ifndef VIBEOS_DAWN_CONTEXT_H
#define VIBEOS_DAWN_CONTEXT_H

#include <stdint.h>

#define DAWN_CONTEXT_MAGIC UINT64_C(0x4441574E43545831)
#define DAWN_CONTEXT_VERSION UINT32_C(5)
#define DAWN_MEMORY_DESCRIPTOR_VERSION UINT32_C(1)
#define DAWN_MEMORY_RANGE_VERSION UINT32_C(1)

typedef enum {
    DAWN_MEMORY_RESERVED = 0,
    DAWN_MEMORY_USABLE = 1
} DAWN_MEMORY_KIND;

typedef struct {
    uint64_t physical_start;
    uint64_t byte_size;
    uint32_t kind;
    uint32_t attributes;
} DAWN_MEMORY_DESCRIPTOR;

typedef struct {
    uint64_t physical_start;
    uint64_t byte_size;
} DAWN_MEMORY_RANGE;

typedef enum {
    DAWN_PIXEL_FORMAT_RGBX8888 = 1,
    DAWN_PIXEL_FORMAT_BGRX8888 = 2,
    DAWN_PIXEL_FORMAT_BGR888 = 3
} DAWN_PIXEL_FORMAT;

typedef struct {
    uint64_t magic;
    uint32_t version;
    uint32_t size;
    uint64_t memory_map_physical_address;
    uint64_t memory_map_size;
    uint64_t memory_map_key;
    uint64_t memory_descriptor_size;
    uint32_t memory_descriptor_version;
    uint32_t reserved;
    uint64_t kernel_stack_top;
    uint64_t kernel_stack_size;
    uint64_t framebuffer_physical_address;
    uint64_t framebuffer_byte_size;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pixels_per_scan_line;
    uint32_t framebuffer_pixel_format;
    uint64_t boot_reservations_physical_address;
    uint64_t boot_reservations_size;
    uint64_t boot_reservation_descriptor_size;
    uint32_t boot_reservation_descriptor_version;
    uint32_t boot_reservation_count;
    uint64_t acpi_rsdp_physical_address;
} DAWN_CONTEXT;

#endif
