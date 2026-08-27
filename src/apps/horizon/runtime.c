/*
 * VibeOS Horizon runtime — explicit bounded event-to-redraw composition.
 * Waiting, IRQ delivery, and device access remain outside this component.
 */

#include "horizon_runtime.h"

int horizon_desktop_runtime_initialize(PRISM_FRAMEBUFFER *framebuffer, HORIZON_DESKTOP_RUNTIME *runtime) {
    if (framebuffer == (void *)0 || runtime == (void *)0) {
        return 0;
    }
    runtime->initialized = 0U;
    if (!horizon_desktop_state_initialize(&runtime->desktop_state, HORIZON_NATIVE_APPLICATION_COUNT) ||
        !horizon_render_desktop_for_state(framebuffer, &runtime->desktop_state)) {
        return 0;
    }
    runtime->framebuffer = *framebuffer;
    runtime->initialized = 1U;
    return 1;
}

int horizon_desktop_runtime_step(
    HORIZON_DESKTOP_RUNTIME *runtime,
    uint32_t maximum_events,
    HORIZON_DESKTOP_RUNTIME_STEP_RESULT *result) {
    if (runtime == (void *)0 || result == (void *)0 || runtime->initialized == 0U ||
        !horizon_desktop_state_is_valid(&runtime->desktop_state)) {
        return 0;
    }
    result->redraw_performed = 0U;
    if (!horizon_input_pump(&runtime->desktop_state, maximum_events, &result->input)) {
        return 0;
    }
    if (result->input.redraw_requested != 0U) {
        if (!horizon_render_desktop_for_state(&runtime->framebuffer, &runtime->desktop_state)) {
            return 0;
        }
        result->redraw_performed = 1U;
    }
    return 1;
}
