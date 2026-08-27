/*
 * VibeOS Horizon desktop runtime — caller-owned composition of framebuffer,
 * focus state, and one bounded input step; it owns no IRQ or wait loop.
 */

#ifndef VIBEOS_HORIZON_RUNTIME_H
#define VIBEOS_HORIZON_RUNTIME_H

#include <horizon_input.h>

typedef struct {
    PRISM_FRAMEBUFFER framebuffer;
    HORIZON_DESKTOP_STATE desktop_state;
    uint32_t native_request_status;
    uint32_t initialized;
} HORIZON_DESKTOP_RUNTIME;

typedef struct {
    HORIZON_INPUT_PUMP_RESULT input;
    uint32_t redraw_performed;
    const HORIZON_APPLICATION_DESCRIPTOR *selected_application;
    uint32_t native_request_status;
} HORIZON_DESKTOP_RUNTIME_STEP_RESULT;

int horizon_desktop_runtime_initialize(PRISM_FRAMEBUFFER *framebuffer, HORIZON_DESKTOP_RUNTIME *runtime);
int horizon_desktop_runtime_step(
    HORIZON_DESKTOP_RUNTIME *runtime,
    uint32_t maximum_events,
    HORIZON_DESKTOP_RUNTIME_STEP_RESULT *result);
int horizon_desktop_runtime_set_native_request_status(
    HORIZON_DESKTOP_RUNTIME *runtime,
    HORIZON_NATIVE_REQUEST_STATUS status);

#endif
