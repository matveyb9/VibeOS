/*
 * VibeOS Horizon — static bootstrap application catalog.
 * This is a data-only mapping from logical desktop cards to native app IDs;
 * it neither loads an image nor grants a launch capability.
 */

#include "horizon.h"

static const HORIZON_APPLICATION_DESCRIPTOR horizon_native_applications[HORIZON_NATIVE_APPLICATION_COUNT] = {
    {HORIZON_APPLICATION_PROMPT, "PROMPT"},
    {HORIZON_APPLICATION_CUE, "CUE"},
    {HORIZON_APPLICATION_VECTOR, "VECTOR"},
};

const HORIZON_APPLICATION_DESCRIPTOR *horizon_application_at(uint32_t logical_window) {
    if (logical_window >= HORIZON_NATIVE_APPLICATION_COUNT) {
        return (const void *)0;
    }
    return &horizon_native_applications[logical_window];
}

int horizon_selected_application(
    const HORIZON_DESKTOP_STATE *state, const HORIZON_APPLICATION_DESCRIPTOR **application) {
    if (application == (void *)0) {
        return 0;
    }
    *application = (const void *)0;
    if (!horizon_desktop_state_is_valid(state) || state->selected_window == HORIZON_DESKTOP_NO_WINDOW) {
        return 0;
    }
    *application = horizon_application_at(state->selected_window);
    return *application != (const void *)0;
}
