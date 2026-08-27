/* VibeOS Pulse — early scheduler state and round-robin policy interface. */

#ifndef VIBEOS_PULSE_SCHEDULER_H
#define VIBEOS_PULSE_SCHEDULER_H

#include <stdint.h>

#define PULSE_SCHEDULER_MAX_TASKS UINT32_C(32)
#define PULSE_TASK_INVALID_ID UINT32_MAX

typedef enum {
    PULSE_TASK_EMPTY = 0,
    PULSE_TASK_READY = 1,
    PULSE_TASK_RUNNING = 2,
    PULSE_TASK_BLOCKED = 3
} PULSE_TASK_STATE;

typedef struct {
    uint32_t id;
    PULSE_TASK_STATE state;
} PULSE_TASK_SLOT;

typedef struct {
    PULSE_TASK_SLOT tasks[PULSE_SCHEDULER_MAX_TASKS];
    uint32_t task_count;
    uint32_t next_task_id;
    uint32_t current_slot;
} PULSE_SCHEDULER_STATE;

void pulse_scheduler_initialize(void);
int pulse_scheduler_create_ready_task(uint32_t *task_id);
int pulse_scheduler_set_task_state(uint32_t task_id, PULSE_TASK_STATE state);
int pulse_scheduler_select_next(uint32_t *task_id);
const PULSE_SCHEDULER_STATE *pulse_scheduler_state(void);

#endif
