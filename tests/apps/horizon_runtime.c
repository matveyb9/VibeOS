/* Horizon runtime test — explicit bounded event-to-redraw lifecycle. */

#include <stdio.h>

#include "horizon_runtime.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    static uint32_t pixels[640U * 480U];
    PRISM_FRAMEBUFFER framebuffer = {
        (uint8_t *)pixels, sizeof(pixels), 640U, 480U, 640U, DAWN_PIXEL_FORMAT_BGRX8888,
    };
    HORIZON_DESKTOP_RUNTIME runtime;
    HORIZON_DESKTOP_RUNTIME_STEP_RESULT result;

    atlas_keyboard_initialize();
    if (!expect(!horizon_desktop_runtime_initialize(&framebuffer, (void *)0), "null runtime is rejected") ||
        !expect(horizon_desktop_runtime_initialize(&framebuffer, &runtime), "runtime initializes and renders") ||
        !expect(runtime.initialized == 1U && runtime.desktop_state.focused_window == 0U,
                "runtime owns initialized focus state") ||
        !expect(horizon_desktop_runtime_step(&runtime, 1U, &result) && result.input.dequeued_event_count == 0U &&
                    result.redraw_performed == 0U && result.selected_application == (const void *)0,
                "empty step does not redraw") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x0f)) && horizon_desktop_runtime_step(&runtime, 1U, &result) &&
                    result.input.dequeued_event_count == 1U && result.input.handled_event_count == 1U &&
                    result.input.redraw_requested == 1U && result.redraw_performed == 1U &&
                    result.selected_application == (const void *)0 &&
                    runtime.desktop_state.focused_window == 1U && pixels[(86U * 640U) + 233U] == UINT32_C(0x00e6f1ff),
                "Tab event advances focus and redraws retained indicator") ||
        !expect(atlas_keyboard_receive_scancode(UINT8_C(0x1c)) && horizon_desktop_runtime_step(&runtime, 1U, &result) &&
                    result.input.dequeued_event_count == 1U && result.input.handled_event_count == 1U &&
                    result.redraw_performed == 1U && runtime.desktop_state.selected_window == 1U &&
                    result.selected_application != (const void *)0 &&
                    result.selected_application->id == HORIZON_APPLICATION_CUE &&
                    pixels[(246U * 640U) + 245U] == UINT32_C(0x00ffcf5c),
                "following Enter selects the persisted focus and redraws amber strip") ||
        !expect(!horizon_desktop_runtime_step(&runtime, 0U, &result), "invalid pump budget is rejected") ||
        !expect(!horizon_desktop_runtime_step((void *)0, 1U, &result), "null runtime step is rejected")) {
        return 1;
    }

    puts("Horizon runtime unit tests passed.");
    return 0;
}
