/*
 * VibeOS Pulse — early physical-frame bootstrap from Dawn memory regions.
 *
 * This stage records all tracked Dawn usable ranges and allocates
 * 4 KiB frames in ascending order. It is a bootstrap for page-table setup,
 * not the final ownership-aware Pulse physical-memory manager.
 */

#include "memory.h"

#define PULSE_PAGE_SIZE UINT64_C(4096)
typedef struct {
    uint64_t base;
    uint64_t limit;
    uint64_t next_free_frame;
} PULSE_MEMORY_REGION;

typedef struct {
    uint64_t base;
    uint64_t limit;
} PULSE_BOOT_RESERVATION;

typedef struct {
    uint64_t physical_address;
    PULSE_MEMORY_OWNER owner;
} PULSE_ALLOCATED_FRAME;

static PULSE_MEMORY_STATE early_memory_state;
static PULSE_MEMORY_REGION early_memory_regions[PULSE_MEMORY_MAX_REGIONS];
static PULSE_BOOT_RESERVATION early_boot_reservations[PULSE_MEMORY_MAX_BOOT_RESERVATIONS];
static PULSE_ALLOCATED_FRAME early_allocated_frames[PULSE_MEMORY_MAX_ALLOCATED_FRAMES];

static uint64_t pulse_align_up(uint64_t value, uint64_t alignment) {
    return (value + (alignment - 1U)) & ~(alignment - 1U);
}

static uint64_t pulse_align_down(uint64_t value, uint64_t alignment) {
    return value & ~(alignment - 1U);
}

static uint64_t pulse_max_u64(uint64_t left, uint64_t right) {
    return left > right ? left : right;
}

static uint64_t pulse_min_u64(uint64_t left, uint64_t right) {
    return left < right ? left : right;
}

static void pulse_clear_memory_state(void) {
    uint32_t index;

    early_memory_state.selected_region_base = 0;
    early_memory_state.selected_region_limit = 0;
    early_memory_state.next_free_frame = 0;
    early_memory_state.usable_page_count = 0;
    early_memory_state.boot_reserved_page_count = 0;
    early_memory_state.allocated_page_count = 0;
    early_memory_state.region_count = 0;
    early_memory_state.active_region_index = 0;
    early_memory_state.boot_reservation_count = 0;

    for (index = 0; index < PULSE_MEMORY_MAX_REGIONS; ++index) {
        early_memory_regions[index].base = 0;
        early_memory_regions[index].limit = 0;
        early_memory_regions[index].next_free_frame = 0;
    }
    for (index = 0; index < PULSE_MEMORY_MAX_BOOT_RESERVATIONS; ++index) {
        early_boot_reservations[index].base = 0;
        early_boot_reservations[index].limit = 0;
    }
    for (index = 0; index < PULSE_MEMORY_MAX_ALLOCATED_FRAMES; ++index) {
        early_allocated_frames[index].physical_address = 0;
        early_allocated_frames[index].owner = PULSE_MEMORY_OWNER_NONE;
    }
}

static int pulse_boot_reservations_initialize(const DAWN_CONTEXT *context) {
    const uint8_t *reservations_base;
    uint64_t reservation_index;
    uint64_t expected_size;
    uint64_t previous_limit = 0;

    if (context->boot_reservations_physical_address == 0U || context->boot_reservations_size == 0U ||
        context->boot_reservation_descriptor_size != sizeof(DAWN_MEMORY_RANGE) ||
        context->boot_reservation_descriptor_version != DAWN_MEMORY_RANGE_VERSION ||
        context->boot_reservation_count == 0U ||
        context->boot_reservation_count > PULSE_MEMORY_MAX_BOOT_RESERVATIONS) {
        return 0;
    }
    expected_size = (uint64_t)context->boot_reservation_count * context->boot_reservation_descriptor_size;
    if (expected_size != context->boot_reservations_size) {
        return 0;
    }

    reservations_base = (const uint8_t *)(uintptr_t)context->boot_reservations_physical_address;
    for (reservation_index = 0; reservation_index < context->boot_reservation_count; ++reservation_index) {
        const DAWN_MEMORY_RANGE *reservation =
            (const DAWN_MEMORY_RANGE *)(reservations_base +
                                        (reservation_index * context->boot_reservation_descriptor_size));
        uint64_t reservation_limit;
        uint64_t page_base;
        uint64_t page_limit;

        if (reservation->byte_size == 0U || reservation->physical_start > UINT64_MAX - reservation->byte_size) {
            return 0;
        }
        reservation_limit = reservation->physical_start + reservation->byte_size;
        if (reservation_limit > UINT64_MAX - (PULSE_PAGE_SIZE - 1U)) {
            return 0;
        }
        page_base = pulse_align_down(reservation->physical_start, PULSE_PAGE_SIZE);
        page_limit = pulse_align_up(reservation_limit, PULSE_PAGE_SIZE);
        if (reservation_index > 0U && page_base < previous_limit) {
            return 0;
        }
        early_boot_reservations[reservation_index].base = page_base;
        early_boot_reservations[reservation_index].limit = page_limit;
        previous_limit = page_limit;
    }
    early_memory_state.boot_reservation_count = context->boot_reservation_count;
    return 1;
}

static uint64_t pulse_reserved_pages_in_region(uint64_t region_base, uint64_t region_limit) {
    uint64_t reservation_index;
    uint64_t reserved_page_count = 0;

    for (reservation_index = 0; reservation_index < early_memory_state.boot_reservation_count;
         ++reservation_index) {
        uint64_t overlap_base = pulse_max_u64(region_base, early_boot_reservations[reservation_index].base);
        uint64_t overlap_limit = pulse_min_u64(region_limit, early_boot_reservations[reservation_index].limit);

        if (overlap_base < overlap_limit) {
            reserved_page_count += (overlap_limit - overlap_base) / PULSE_PAGE_SIZE;
        }
    }
    return reserved_page_count;
}

static int pulse_frame_is_boot_reserved(uint64_t frame) {
    uint32_t reservation_index;

    for (reservation_index = 0; reservation_index < early_memory_state.boot_reservation_count;
         ++reservation_index) {
        if (frame >= early_boot_reservations[reservation_index].base &&
            frame < early_boot_reservations[reservation_index].limit) {
            return 1;
        }
    }
    return 0;
}

static int pulse_memory_append_canonical_region(uint64_t region_base, uint64_t region_limit) {
    uint64_t region_bytes;
    PULSE_MEMORY_REGION *previous_region;

    if (region_base >= region_limit || early_memory_state.usable_page_count > UINT64_MAX -
                                                               ((region_limit - region_base) / PULSE_PAGE_SIZE)) {
        return 0;
    }
    region_bytes = region_limit - region_base;
    if (early_memory_state.region_count != 0U) {
        previous_region = &early_memory_regions[early_memory_state.region_count - 1U];
        if (region_base < previous_region->limit) {
            return 0;
        }
        if (region_base == previous_region->limit) {
            previous_region->limit = region_limit;
            early_memory_state.usable_page_count += region_bytes / PULSE_PAGE_SIZE;
            return 1;
        }
    }
    if (early_memory_state.region_count >= PULSE_MEMORY_MAX_REGIONS) {
        return 0;
    }
    early_memory_regions[early_memory_state.region_count].base = region_base;
    early_memory_regions[early_memory_state.region_count].limit = region_limit;
    early_memory_regions[early_memory_state.region_count].next_free_frame = region_base;
    early_memory_state.usable_page_count += region_bytes / PULSE_PAGE_SIZE;
    ++early_memory_state.region_count;
    return 1;
}

int pulse_memory_initialize(const DAWN_CONTEXT *context) {
    uint64_t descriptor_index;
    uint64_t descriptor_count;
    uint64_t previous_descriptor_limit = 0;
    const uint8_t *map_base;

    pulse_clear_memory_state();
    if (context == (void *)0 || context->memory_map_physical_address == 0U || context->memory_map_size == 0U ||
        context->memory_descriptor_size != sizeof(DAWN_MEMORY_DESCRIPTOR) ||
        context->memory_descriptor_version != DAWN_MEMORY_DESCRIPTOR_VERSION ||
        (context->memory_map_size % context->memory_descriptor_size) != 0U) {
        return 0;
    }
    if (!pulse_boot_reservations_initialize(context)) {
        return 0;
    }

    map_base = (const uint8_t *)(uintptr_t)context->memory_map_physical_address;
    descriptor_count = context->memory_map_size / context->memory_descriptor_size;
    for (descriptor_index = 0; descriptor_index < descriptor_count; ++descriptor_index) {
        const DAWN_MEMORY_DESCRIPTOR *descriptor =
            (const DAWN_MEMORY_DESCRIPTOR *)(map_base + (descriptor_index * context->memory_descriptor_size));
        uint64_t region_base;
        uint64_t region_limit;

        if (descriptor->byte_size == 0U || descriptor->byte_size > UINT64_MAX - descriptor->physical_start) {
            return 0;
        }
        if (descriptor_index != 0U && descriptor->physical_start < previous_descriptor_limit) {
            return 0;
        }
        previous_descriptor_limit = descriptor->physical_start + descriptor->byte_size;
        if (descriptor->kind != DAWN_MEMORY_USABLE) {
            continue;
        }
        if (previous_descriptor_limit > UINT64_MAX - (PULSE_PAGE_SIZE - 1U)) {
            return 0;
        }

        region_base = pulse_align_up(descriptor->physical_start, PULSE_PAGE_SIZE);
        region_limit = pulse_align_down(
            descriptor->physical_start + descriptor->byte_size,
            PULSE_PAGE_SIZE);
        if (region_base >= region_limit || !pulse_memory_append_canonical_region(region_base, region_limit)) return 0;
        early_memory_state.boot_reserved_page_count += pulse_reserved_pages_in_region(region_base, region_limit);
    }

    if (early_memory_state.region_count == 0U) {
        return 0;
    }
    if (early_memory_state.boot_reserved_page_count > early_memory_state.usable_page_count) {
        return 0;
    }
    early_memory_state.usable_page_count -= early_memory_state.boot_reserved_page_count;

    early_memory_state.selected_region_base = early_memory_regions[0].base;
    early_memory_state.selected_region_limit = early_memory_regions[0].limit;
    early_memory_state.next_free_frame = early_memory_regions[0].next_free_frame;
    return 1;
}

int pulse_memory_take_frame_owned(PULSE_MEMORY_OWNER owner, uint64_t *physical_address) {
    uint32_t region_index;

    if (physical_address == (void *)0 || owner == PULSE_MEMORY_OWNER_NONE ||
        early_memory_state.allocated_page_count >= PULSE_MEMORY_MAX_ALLOCATED_FRAMES) {
        return 0;
    }

    for (region_index = early_memory_state.active_region_index;
         region_index < early_memory_state.region_count;
         ++region_index) {
        PULSE_MEMORY_REGION *region = &early_memory_regions[region_index];

        while (region->next_free_frame <= region->limit - PULSE_PAGE_SIZE) {
            uint64_t candidate = region->next_free_frame;

            region->next_free_frame += PULSE_PAGE_SIZE;
            if (pulse_frame_is_boot_reserved(candidate)) {
                continue;
            }
            *physical_address = candidate;
            early_allocated_frames[early_memory_state.allocated_page_count].physical_address = candidate;
            early_allocated_frames[early_memory_state.allocated_page_count].owner = owner;
            ++early_memory_state.allocated_page_count;
            early_memory_state.active_region_index = region_index;
            early_memory_state.selected_region_base = region->base;
            early_memory_state.selected_region_limit = region->limit;
            early_memory_state.next_free_frame = region->next_free_frame;
            return 1;
        }
    }

    return 0;
}

int pulse_memory_take_frame(uint64_t *physical_address) {
    return pulse_memory_take_frame_owned(PULSE_MEMORY_OWNER_BOOTSTRAP, physical_address);
}

PULSE_MEMORY_OWNER pulse_memory_frame_owner(uint64_t physical_address) {
    uint32_t frame_index;

    for (frame_index = 0; frame_index < early_memory_state.allocated_page_count; ++frame_index) {
        if (early_allocated_frames[frame_index].physical_address == physical_address) {
            return early_allocated_frames[frame_index].owner;
        }
    }
    return PULSE_MEMORY_OWNER_NONE;
}

const PULSE_MEMORY_STATE *pulse_memory_state(void) {
    return &early_memory_state;
}
