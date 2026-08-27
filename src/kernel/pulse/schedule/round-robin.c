/*
 * VibeOS Pulse — early deterministic round-robin scheduler policy.
 *
 * This unit owns scheduling state only. It neither switches CPU contexts nor
 * enables preemption; those require safe trap frames and timer routing first.
 */

#include "scheduler.h"

static PULSE_SCHEDULER_STATE scheduler_state;

void pulse_scheduler_initialize(void) {
    uint32_t slot;

    for (slot = 0; slot < PULSE_SCHEDULER_MAX_TASKS; ++slot) {
        scheduler_state.tasks[slot].id = PULSE_TASK_INVALID_ID;
        scheduler_state.tasks[slot].state = PULSE_TASK_EMPTY;
    }
    scheduler_state.task_count = 0;
    scheduler_state.next_task_id = 0;
    scheduler_state.current_slot = PULSE_SCHEDULER_MAX_TASKS - 1U;
}

int pulse_scheduler_create_ready_task(uint32_t *task_id) {
    uint32_t slot;

    if (task_id == (void *)0 || scheduler_state.task_count >= PULSE_SCHEDULER_MAX_TASKS ||
        scheduler_state.next_task_id == PULSE_TASK_INVALID_ID) {
        return 0;
    }

    for (slot = 0; slot < PULSE_SCHEDULER_MAX_TASKS; ++slot) {
        if (scheduler_state.tasks[slot].state == PULSE_TASK_EMPTY) {
            scheduler_state.tasks[slot].id = scheduler_state.next_task_id;
            scheduler_state.tasks[slot].state = PULSE_TASK_READY;
            *task_id = scheduler_state.next_task_id;
            ++scheduler_state.next_task_id;
            ++scheduler_state.task_count;
            return 1;
        }
    }

    return 0;
}

int pulse_scheduler_set_task_state(uint32_t task_id, PULSE_TASK_STATE state) {
    uint32_t slot;

    if (state != PULSE_TASK_READY && state != PULSE_TASK_BLOCKED) {
        return 0;
    }

    for (slot = 0; slot < PULSE_SCHEDULER_MAX_TASKS; ++slot) {
        if (scheduler_state.tasks[slot].id == task_id &&
            scheduler_state.tasks[slot].state != PULSE_TASK_EMPTY) {
            scheduler_state.tasks[slot].state = state;
            return 1;
        }
    }

    return 0;
}

int pulse_scheduler_select_next(uint32_t *task_id) {
    uint32_t scanned;

    if (task_id == (void *)0 || scheduler_state.task_count == 0U) {
        return 0;
    }

    if (scheduler_state.current_slot < PULSE_SCHEDULER_MAX_TASKS &&
        scheduler_state.tasks[scheduler_state.current_slot].state == PULSE_TASK_RUNNING) {
        scheduler_state.tasks[scheduler_state.current_slot].state = PULSE_TASK_READY;
    }

    for (scanned = 1; scanned <= PULSE_SCHEDULER_MAX_TASKS; ++scanned) {
        uint32_t candidate = (scheduler_state.current_slot + scanned) % PULSE_SCHEDULER_MAX_TASKS;

        if (scheduler_state.tasks[candidate].state == PULSE_TASK_READY) {
            scheduler_state.tasks[candidate].state = PULSE_TASK_RUNNING;
            scheduler_state.current_slot = candidate;
            *task_id = scheduler_state.tasks[candidate].id;
            return 1;
        }
    }

    return 0;
}

const PULSE_SCHEDULER_STATE *pulse_scheduler_state(void) {
    return &scheduler_state;
}
