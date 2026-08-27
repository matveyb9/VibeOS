/* VibeOS Pulse — early physical memory bootstrap contract. */

#ifndef VIBEOS_PULSE_MEMORY_H
#define VIBEOS_PULSE_MEMORY_H

#include <stdint.h>
#include <dawn.h>

#define PULSE_MEMORY_MAX_REGIONS UINT32_C(32)
#define PULSE_MEMORY_MAX_BOOT_RESERVATIONS UINT32_C(32)
#define PULSE_MEMORY_MAX_ALLOCATED_FRAMES UINT32_C(64)

typedef enum {
    PULSE_MEMORY_OWNER_NONE = 0,
    PULSE_MEMORY_OWNER_BOOTSTRAP = 1,
    PULSE_MEMORY_OWNER_PAGE_TABLE = 2
} PULSE_MEMORY_OWNER;

typedef struct {
    uint64_t selected_region_base;
    uint64_t selected_region_limit;
    uint64_t next_free_frame;
    uint64_t usable_page_count;
    uint64_t boot_reserved_page_count;
    uint64_t allocated_page_count;
    uint32_t region_count;
    uint32_t active_region_index;
    uint32_t boot_reservation_count;
} PULSE_MEMORY_STATE;

int pulse_memory_initialize(const DAWN_CONTEXT *context);
int pulse_memory_take_frame(uint64_t *physical_address);
int pulse_memory_take_frame_owned(PULSE_MEMORY_OWNER owner, uint64_t *physical_address);
PULSE_MEMORY_OWNER pulse_memory_frame_owner(uint64_t physical_address);
const PULSE_MEMORY_STATE *pulse_memory_state(void);

#endif
