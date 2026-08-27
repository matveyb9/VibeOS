/* VibeOS Prism — early visual handoff and Canvas scene composition. */

#include <canvas.h>
#include <horizon.h>
#include <prism.h>

int prism_canvas_runtime_probe(const DAWN_CONTEXT *context) {
    PRISM_FRAMEBUFFER framebuffer;

    if (!prism_framebuffer_from_dawn(context, &framebuffer)) {
        return 0;
    }
    return horizon_render_desktop(&framebuffer);
}
