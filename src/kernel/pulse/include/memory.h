/* VibeOS Pulse — early physical memory bootstrap contract. */

#ifndef VIBEOS_PULSE_MEMORY_H
#define VIBEOS_PULSE_MEMORY_H

#include <stdint.h>
#include <dawn.h>

typedef struct {
    uint64_t selected_region_base;
    uint64_t selected_region_limit;
    uint64_t next_free_frame;
    uint64_t usable_page_count;
} PULSE_MEMORY_STATE;

int pulse_memory_initialize(const DAWN_CONTEXT *context);
int pulse_memory_take_frame(uint64_t *physical_address);
const PULSE_MEMORY_STATE *pulse_memory_state(void);

#endif
