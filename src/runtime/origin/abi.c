/* VibeOS Origin — first versioned native capability-call ABI. */

#include "origin_abi.h"

int origin_abi_dispatch(ORIGIN_CALL *call) {
    if (call == (void *)0 || call->version != ORIGIN_ABI_VERSION || call->reserved != 0U) {
        if (call != (void *)0) {
            call->status = ORIGIN_STATUS_BAD_FRAME;
        }
        return 0;
    }

    call->output_key = VIBE_KEY_INVALID;
    call->output_object = 0;
    call->output_rights = 0;
    switch ((ORIGIN_OPERATION)call->operation) {
        case ORIGIN_OPERATION_INSPECT_KEY:
            if (!origin_key_inspect(call->input_key, &call->output_object, &call->output_rights)) {
                call->status = ORIGIN_STATUS_DENIED;
                return 0;
            }
            call->status = ORIGIN_STATUS_OK;
            return 1;
        case ORIGIN_OPERATION_NARROW_KEY:
            if (!origin_key_narrow(call->input_key, call->requested_rights, &call->output_key)) {
                call->status = ORIGIN_STATUS_DENIED;
                return 0;
            }
            call->status = ORIGIN_STATUS_OK;
            return 1;
        default:
            call->status = ORIGIN_STATUS_UNSUPPORTED;
            return 0;
    }
}
