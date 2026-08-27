/*
 * VibeOS Horizon input adapter — translates normalized Atlas events into
 * desktop intent; it owns neither hardware input, rendering, nor launch.
 */

#ifndef VIBEOS_HORIZON_INPUT_H
#define VIBEOS_HORIZON_INPUT_H

#include <atlas_input.h>
#include <horizon.h>

#define HORIZON_INPUT_PUMP_MAX_EVENTS UINT32_C(8)

typedef struct {
    uint32_t dequeued_event_count;
    uint32_t handled_event_count;
    uint32_t redraw_requested;
} HORIZON_INPUT_PUMP_RESULT;

int horizon_input_apply_event(
    HORIZON_DESKTOP_STATE *state, const ATLAS_KEY_EVENT *event, uint32_t *handled);
int horizon_input_pump(
    HORIZON_DESKTOP_STATE *state, uint32_t maximum_events, HORIZON_INPUT_PUMP_RESULT *result);
int horizon_input_runtime_probe(void);

#endif
