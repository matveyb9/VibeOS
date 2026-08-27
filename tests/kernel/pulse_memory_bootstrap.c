/* VibeOS Pulse — host unit checks for the first physical-frame bootstrap. */

#include <stdint.h>
#include <stdio.h>

#include "memory.h"

typedef struct {
    uint32_t type;
    uint32_t padding;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
} TEST_UEFI_MEMORY_DESCRIPTOR;

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    TEST_UEFI_MEMORY_DESCRIPTOR map[2] = {
        {0U, 0U, UINT64_C(0x0), 0U, 1U, 0U},
        {7U, 0U, UINT64_C(0x1003), 0U, 4U, 0U},
    };
    DAWN_CONTEXT context = {
        DAWN_CONTEXT_MAGIC,
        DAWN_CONTEXT_VERSION,
        (uint32_t)sizeof(DAWN_CONTEXT),
        (uint64_t)(uintptr_t)map,
        sizeof(map),
        0U,
        sizeof(TEST_UEFI_MEMORY_DESCRIPTOR),
        1U,
        0U,
        0U,
        0U,
    };
    const PULSE_MEMORY_STATE *state;
    uint64_t frame_one;
    uint64_t frame_two;
    uint64_t frame_three;

    if (!expect(!pulse_memory_initialize(&(DAWN_CONTEXT){0}), "invalid descriptor stride is rejected") ||
        !expect(pulse_memory_initialize(&context), "conventional memory region initializes") ||
        !expect(pulse_memory_take_frame(&frame_one), "first frame is available") ||
        !expect(pulse_memory_take_frame(&frame_two), "second frame is available") ||
        !expect(pulse_memory_take_frame(&frame_three), "third frame is available") ||
        !expect(!pulse_memory_take_frame(&frame_three), "allocator respects selected region limit")) {
        return 1;
    }

    state = pulse_memory_state();
    if (!expect(frame_one == UINT64_C(0x2000), "first frame is page-aligned") ||
        !expect(frame_two == UINT64_C(0x3000), "frames are contiguous") ||
        !expect(frame_three == UINT64_C(0x4000), "last valid frame is returned") ||
        !expect(state->usable_page_count == 3U, "usable page count reflects alignment")) {
        return 1;
    }

    puts("Pulse memory bootstrap unit tests passed.");
    return 0;
}
