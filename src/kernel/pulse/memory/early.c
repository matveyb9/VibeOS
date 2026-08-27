/*
 * VibeOS Pulse — minimal physical-frame bootstrap from the UEFI memory map.
 *
 * This first allocator is deliberately narrow: it selects one conventional
 * memory range, then hands out contiguous 4 KiB frames. It is a bootstrap for
 * page-table setup, not the final Pulse physical-memory manager.
 */

#include "memory.h"

#define PULSE_PAGE_SIZE UINT64_C(4096)
#define PULSE_EFI_CONVENTIONAL_MEMORY UINT32_C(7)

typedef struct {
    uint32_t type;
    uint32_t padding;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
} PULSE_UEFI_MEMORY_DESCRIPTOR;

static PULSE_MEMORY_STATE early_memory_state;

static uint64_t pulse_align_up(uint64_t value, uint64_t alignment) {
    return (value + (alignment - 1U)) & ~(alignment - 1U);
}

int pulse_memory_initialize(const DAWN_CONTEXT *context) {
    uint64_t descriptor_offset;
    uint64_t descriptor_count;

    if (context == (void *)0 || context->memory_descriptor_size < sizeof(PULSE_UEFI_MEMORY_DESCRIPTOR) ||
        (context->memory_map_size % context->memory_descriptor_size) != 0U) {
        return 0;
    }

    descriptor_count = context->memory_map_size / context->memory_descriptor_size;
    for (descriptor_offset = 0; descriptor_offset < descriptor_count; ++descriptor_offset) {
        const uint8_t *map_base = (const uint8_t *)(uintptr_t)context->memory_map_physical_address;
        const PULSE_UEFI_MEMORY_DESCRIPTOR *descriptor =
            (const PULSE_UEFI_MEMORY_DESCRIPTOR *)(map_base +
                                                    (descriptor_offset * context->memory_descriptor_size));
        uint64_t region_base;
        uint64_t region_limit;

        if (descriptor->type != PULSE_EFI_CONVENTIONAL_MEMORY || descriptor->number_of_pages == 0U) {
            continue;
        }

        region_base = pulse_align_up(descriptor->physical_start, PULSE_PAGE_SIZE);
        region_limit = descriptor->physical_start + (descriptor->number_of_pages * PULSE_PAGE_SIZE);
        if (region_base >= region_limit) {
            continue;
        }

        early_memory_state.selected_region_base = region_base;
        early_memory_state.selected_region_limit = region_limit;
        early_memory_state.next_free_frame = region_base;
        early_memory_state.usable_page_count = (region_limit - region_base) / PULSE_PAGE_SIZE;
        return 1;
    }

    return 0;
}

int pulse_memory_take_frame(uint64_t *physical_address) {
    uint64_t frame;

    if (physical_address == (void *)0 || early_memory_state.next_free_frame == 0U ||
        early_memory_state.next_free_frame >
            early_memory_state.selected_region_limit - PULSE_PAGE_SIZE) {
        return 0;
    }

    frame = early_memory_state.next_free_frame;
    early_memory_state.next_free_frame += PULSE_PAGE_SIZE;
    *physical_address = frame;
    return 1;
}

const PULSE_MEMORY_STATE *pulse_memory_state(void) {
    return &early_memory_state;
}
