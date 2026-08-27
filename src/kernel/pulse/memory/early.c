/*
 * VibeOS Pulse — early physical-frame bootstrap from the UEFI memory map.
 *
 * This stage records all tracked EfiConventionalMemory ranges and allocates
 * 4 KiB frames in ascending order. It is a bootstrap for page-table setup,
 * not the final ownership-aware Pulse physical-memory manager.
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

typedef struct {
    uint64_t base;
    uint64_t limit;
    uint64_t next_free_frame;
} PULSE_MEMORY_REGION;

static PULSE_MEMORY_STATE early_memory_state;
static PULSE_MEMORY_REGION early_memory_regions[PULSE_MEMORY_MAX_REGIONS];

static uint64_t pulse_align_up(uint64_t value, uint64_t alignment) {
    return (value + (alignment - 1U)) & ~(alignment - 1U);
}

static uint64_t pulse_align_down(uint64_t value, uint64_t alignment) {
    return value & ~(alignment - 1U);
}

static void pulse_clear_memory_state(void) {
    uint32_t index;

    early_memory_state.selected_region_base = 0;
    early_memory_state.selected_region_limit = 0;
    early_memory_state.next_free_frame = 0;
    early_memory_state.usable_page_count = 0;
    early_memory_state.region_count = 0;
    early_memory_state.active_region_index = 0;

    for (index = 0; index < PULSE_MEMORY_MAX_REGIONS; ++index) {
        early_memory_regions[index].base = 0;
        early_memory_regions[index].limit = 0;
        early_memory_regions[index].next_free_frame = 0;
    }
}

int pulse_memory_initialize(const DAWN_CONTEXT *context) {
    uint64_t descriptor_index;
    uint64_t descriptor_count;
    const uint8_t *map_base;

    pulse_clear_memory_state();
    if (context == (void *)0 || context->memory_descriptor_size < sizeof(PULSE_UEFI_MEMORY_DESCRIPTOR) ||
        (context->memory_map_size % context->memory_descriptor_size) != 0U) {
        return 0;
    }

    map_base = (const uint8_t *)(uintptr_t)context->memory_map_physical_address;
    descriptor_count = context->memory_map_size / context->memory_descriptor_size;
    for (descriptor_index = 0; descriptor_index < descriptor_count; ++descriptor_index) {
        const PULSE_UEFI_MEMORY_DESCRIPTOR *descriptor =
            (const PULSE_UEFI_MEMORY_DESCRIPTOR *)(map_base +
                                                    (descriptor_index * context->memory_descriptor_size));
        uint64_t region_base;
        uint64_t region_limit;
        uint64_t region_bytes;
        uint32_t region_index;

        if (descriptor->type != PULSE_EFI_CONVENTIONAL_MEMORY || descriptor->number_of_pages == 0U ||
            descriptor->number_of_pages >
                (UINT64_MAX - descriptor->physical_start) / PULSE_PAGE_SIZE) {
            continue;
        }

        region_base = pulse_align_up(descriptor->physical_start, PULSE_PAGE_SIZE);
        region_limit = pulse_align_down(
            descriptor->physical_start + (descriptor->number_of_pages * PULSE_PAGE_SIZE),
            PULSE_PAGE_SIZE);
        if (region_base >= region_limit || early_memory_state.region_count >= PULSE_MEMORY_MAX_REGIONS) {
            return 0;
        }

        region_index = early_memory_state.region_count;
        region_bytes = region_limit - region_base;
        early_memory_regions[region_index].base = region_base;
        early_memory_regions[region_index].limit = region_limit;
        early_memory_regions[region_index].next_free_frame = region_base;
        early_memory_state.usable_page_count += region_bytes / PULSE_PAGE_SIZE;
        ++early_memory_state.region_count;
    }

    if (early_memory_state.region_count == 0U) {
        return 0;
    }

    early_memory_state.selected_region_base = early_memory_regions[0].base;
    early_memory_state.selected_region_limit = early_memory_regions[0].limit;
    early_memory_state.next_free_frame = early_memory_regions[0].next_free_frame;
    return 1;
}

int pulse_memory_take_frame(uint64_t *physical_address) {
    uint32_t region_index;

    if (physical_address == (void *)0) {
        return 0;
    }

    for (region_index = early_memory_state.active_region_index;
         region_index < early_memory_state.region_count;
         ++region_index) {
        PULSE_MEMORY_REGION *region = &early_memory_regions[region_index];

        if (region->next_free_frame > region->limit - PULSE_PAGE_SIZE) {
            continue;
        }

        *physical_address = region->next_free_frame;
        region->next_free_frame += PULSE_PAGE_SIZE;
        early_memory_state.active_region_index = region_index;
        early_memory_state.selected_region_base = region->base;
        early_memory_state.selected_region_limit = region->limit;
        early_memory_state.next_free_frame = region->next_free_frame;
        return 1;
    }

    return 0;
}

const PULSE_MEMORY_STATE *pulse_memory_state(void) {
    return &early_memory_state;
}
