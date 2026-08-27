/* VibeOS Horizon — early retained desktop-shell scene contract. */

#ifndef VIBEOS_HORIZON_H
#define VIBEOS_HORIZON_H

#include <canvas.h>

#define HORIZON_DESKTOP_MAX_WINDOWS UINT32_C(8)
#define HORIZON_DESKTOP_NO_WINDOW UINT32_MAX
#define HORIZON_NATIVE_APPLICATION_COUNT UINT32_C(3)

typedef enum {
    HORIZON_APPLICATION_PROMPT = 1,
    HORIZON_APPLICATION_CUE = 2,
    HORIZON_APPLICATION_VECTOR = 3
} HORIZON_APPLICATION_ID;

typedef struct {
    uint32_t id;
    const char *label;
} HORIZON_APPLICATION_DESCRIPTOR;

typedef enum {
    HORIZON_NATIVE_REQUEST_NONE = 0,
    HORIZON_NATIVE_REQUEST_FORMED = 1,
    HORIZON_NATIVE_REQUEST_ADMITTED = 2,
    HORIZON_NATIVE_REQUEST_REJECTED_NOT_INSTALLED = 3,
    HORIZON_NATIVE_REQUEST_REJECTED_INVALID = 4
} HORIZON_NATIVE_REQUEST_STATUS;

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
const HORIZON_APPLICATION_DESCRIPTOR *horizon_application_at(uint32_t logical_window);
int horizon_selected_application(
    const HORIZON_DESKTOP_STATE *state, const HORIZON_APPLICATION_DESCRIPTOR **application);
int horizon_runtime_probe(void);
int horizon_build_desktop_scene_for_state(
    uint32_t width, uint32_t height, const HORIZON_DESKTOP_STATE *state, CANVAS_SCENE *scene);
int horizon_build_desktop_scene_for_state_and_request(
    uint32_t width,
    uint32_t height,
    const HORIZON_DESKTOP_STATE *state,
    uint32_t native_request_status,
    CANVAS_SCENE *scene);
int horizon_render_desktop_for_state_and_request(
    PRISM_FRAMEBUFFER *framebuffer,
    const HORIZON_DESKTOP_STATE *state,
    uint32_t native_request_status);
int horizon_build_desktop_scene(uint32_t width, uint32_t height, CANVAS_SCENE *scene);
int horizon_render_desktop_for_state(PRISM_FRAMEBUFFER *framebuffer, const HORIZON_DESKTOP_STATE *state);
int horizon_render_desktop(PRISM_FRAMEBUFFER *framebuffer);

#endif
