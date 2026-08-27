/* VibeOS Horizon — early retained desktop-shell scene contract. */

#ifndef VIBEOS_HORIZON_H
#define VIBEOS_HORIZON_H

#include <canvas.h>

#define HORIZON_DESKTOP_MAX_WINDOWS UINT32_C(8)
#define HORIZON_DESKTOP_NO_WINDOW UINT32_MAX

typedef enum {
    HORIZON_DESKTOP_ACTION_FOCUS_NEXT = 1,
    HORIZON_DESKTOP_ACTION_FOCUS_PREVIOUS = 2,
    HORIZON_DESKTOP_ACTION_SELECT_FOCUSED = 3
} HORIZON_DESKTOP_ACTION;

typedef struct {
    uint32_t window_count;
    uint32_t focused_window;
    uint32_t selected_window;
} HORIZON_DESKTOP_STATE;

int horizon_desktop_state_initialize(HORIZON_DESKTOP_STATE *state, uint32_t window_count);
int horizon_desktop_apply_action(HORIZON_DESKTOP_STATE *state, HORIZON_DESKTOP_ACTION action);
int horizon_desktop_state_is_valid(const HORIZON_DESKTOP_STATE *state);
int horizon_runtime_probe(void);
int horizon_build_desktop_scene_for_state(
    uint32_t width, uint32_t height, const HORIZON_DESKTOP_STATE *state, CANVAS_SCENE *scene);
int horizon_build_desktop_scene(uint32_t width, uint32_t height, CANVAS_SCENE *scene);
int horizon_render_desktop(PRISM_FRAMEBUFFER *framebuffer);

#endif
