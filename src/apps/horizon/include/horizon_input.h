/*
 * VibeOS Horizon input adapter — translates normalized Atlas events into
 * desktop intent; it owns neither hardware input, rendering, nor launch.
 */

#ifndef VIBEOS_HORIZON_INPUT_H
#define VIBEOS_HORIZON_INPUT_H

#include <atlas_input.h>
#include <horizon.h>

int horizon_input_apply_event(
    HORIZON_DESKTOP_STATE *state, const ATLAS_KEY_EVENT *event, uint32_t *handled);
int horizon_input_runtime_probe(void);

#endif
