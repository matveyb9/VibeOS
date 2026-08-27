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
    DAWN_CONTEXT context = {
        DAWN_CONTEXT_MAGIC,
        DAWN_CONTEXT_VERSION,
        (uint32_t)sizeof(DAWN_CONTEXT),
        (uint64_t)(uintptr_t)map,
        sizeof(map),
        0U,
        sizeof(DAWN_MEMORY_DESCRIPTOR),
        DAWN_MEMORY_DESCRIPTOR_VERSION,
        0U,
        0U,
        0U,
        0U,
        0U,
        0U,
        0U,
        0U,
        0U,
    };
    const PULSE_MEMORY_STATE *state;
    uint64_t frame_one;
    uint64_t frame_two;
    uint64_t frame_three;
    uint64_t frame_four;
    uint64_t frame_five;

    if (!expect(!pulse_memory_initialize(&(DAWN_CONTEXT){0}), "invalid descriptor stride is rejected") ||
        !expect(pulse_memory_initialize(&context), "conventional memory regions initialize") ||
        !expect(pulse_memory_take_frame(&frame_one), "first frame is available") ||
        !expect(pulse_memory_take_frame(&frame_two), "second frame is available") ||
        !expect(pulse_memory_take_frame(&frame_three), "third frame is available") ||
        !expect(pulse_memory_take_frame(&frame_four), "fourth frame crosses into next region") ||
        !expect(pulse_memory_take_frame(&frame_five), "fifth frame is available") ||
        !expect(!pulse_memory_take_frame(&frame_five), "allocator respects all tracked region limits")) {
        return 1;
    }

    state = pulse_memory_state();
    if (!expect(frame_one == UINT64_C(0x2000), "first frame is page-aligned") ||
        !expect(frame_two == UINT64_C(0x3000), "frames remain contiguous in a region") ||
        !expect(frame_three == UINT64_C(0x4000), "last aligned frame in first region is returned") ||
        !expect(frame_four == UINT64_C(0x8000) && frame_five == UINT64_C(0x9000),
                    "allocator advances to the next tracked region") ||
        !expect(state->usable_page_count == 5U, "usable page count includes all tracked regions") ||
        !expect(state->region_count == 2U, "both conventional regions are retained")) {
        return 1;
    }

    puts("Pulse memory bootstrap unit tests passed.");
    return 0;
}
