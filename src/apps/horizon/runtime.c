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
    runtime->native_request_status = HORIZON_NATIVE_REQUEST_NONE;
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
    result->selected_application = (const void *)0;
    result->native_request_status = runtime->native_request_status;
    if (!horizon_input_pump(&runtime->desktop_state, maximum_events, &result->input)) {
        return 0;
    }
    if (result->input.redraw_requested != 0U) {
        if (result->input.selection_requested != 0U) {
            if (!horizon_selected_application(&runtime->desktop_state, &result->selected_application)) {
                return 0;
            }
            runtime->native_request_status = HORIZON_NATIVE_REQUEST_FORMED;
            result->native_request_status = runtime->native_request_status;
        }
        if (!horizon_render_desktop_for_state_and_request(
                &runtime->framebuffer, &runtime->desktop_state, runtime->native_request_status)) {
            return 0;
        }
        result->redraw_performed = 1U;
    }
    return 1;
}

int horizon_desktop_runtime_set_native_request_status(
    HORIZON_DESKTOP_RUNTIME *runtime,
    HORIZON_NATIVE_REQUEST_STATUS status) {
    if (runtime == (void *)0 || runtime->initialized == 0U || status > HORIZON_NATIVE_REQUEST_REJECTED_INVALID) {
        return 0;
    }
    runtime->native_request_status = (uint32_t)status;
    return horizon_render_desktop_for_state_and_request(
        &runtime->framebuffer, &runtime->desktop_state, runtime->native_request_status);
}
