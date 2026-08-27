/* VibeOS Pulse — host unit checks for the early physical-frame bootstrap. */

#include <stdint.h>
#include <stdio.h>

#include "memory.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    DAWN_MEMORY_DESCRIPTOR map[3] = {
        {UINT64_C(0x0), UINT64_C(0x1000), DAWN_MEMORY_RESERVED, 0U},
        {UINT64_C(0x1003), UINT64_C(0x4000), DAWN_MEMORY_USABLE, 0U},
        {UINT64_C(0x8000), UINT64_C(0x2000), DAWN_MEMORY_USABLE, 0U},
    };
    DAWN_MEMORY_RANGE reservations[2] = {
        {UINT64_C(0x3001), UINT64_C(0x0100)},
        {UINT64_C(0x8000), UINT64_C(0x1000)},
    };
    DAWN_CONTEXT context = {
        .magic = DAWN_CONTEXT_MAGIC,
        .version = DAWN_CONTEXT_VERSION,
        .size = (uint32_t)sizeof(DAWN_CONTEXT),
        .memory_map_physical_address = (uint64_t)(uintptr_t)map,
        .memory_map_size = sizeof(map),
        .memory_descriptor_size = sizeof(DAWN_MEMORY_DESCRIPTOR),
        .memory_descriptor_version = DAWN_MEMORY_DESCRIPTOR_VERSION,
        .boot_reservations_physical_address = (uint64_t)(uintptr_t)reservations,
        .boot_reservations_size = sizeof(reservations),
        .boot_reservation_descriptor_size = sizeof(DAWN_MEMORY_RANGE),
        .boot_reservation_descriptor_version = DAWN_MEMORY_RANGE_VERSION,
        .boot_reservation_count = 2U,
    };
    const PULSE_MEMORY_STATE *state;
    uint64_t frame_one;
    uint64_t frame_two;
    uint64_t frame_three;
    uint64_t frame_four;

    if (!expect(!pulse_memory_initialize(&(DAWN_CONTEXT){0}), "invalid descriptor stride is rejected") ||
        !expect(pulse_memory_initialize(&context), "memory regions and boot reservations initialize") ||
        !expect(pulse_memory_take_frame(&frame_one), "first frame is available") ||
        !expect(pulse_memory_take_frame(&frame_two), "second frame is available") ||
        !expect(pulse_memory_take_frame(&frame_three), "third frame advances past reservations") ||
        !expect(!pulse_memory_take_frame(&frame_four), "allocator excludes all boot-owned frames")) {
        return 1;
    }

    state = pulse_memory_state();
    if (!expect(frame_one == UINT64_C(0x2000), "first frame is page-aligned") ||
        !expect(frame_two == UINT64_C(0x4000), "partial reservation removes its full page") ||
        !expect(frame_three == UINT64_C(0x9000), "allocator advances past reserved frame in next region") ||
        !expect(state->usable_page_count == 3U, "usable count excludes boot-owned pages") ||
        !expect(state->boot_reserved_page_count == 2U, "boot-owned page count is retained") ||
        !expect(state->boot_reservation_count == 2U, "both boot reservations are retained") ||
        !expect(state->region_count == 2U, "both conventional regions are retained")) {
        return 1;
    }

    puts("Pulse memory bootstrap unit tests passed.");
    return 0;
}
