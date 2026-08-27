/* VibeOS Pulse — host checks for deterministic early scheduler policy. */

#include <stdint.h>
#include <stdio.h>

#include "scheduler.h"

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    uint32_t first_task;
    uint32_t second_task;
    uint32_t third_task;
    uint32_t selected_task;

    pulse_scheduler_initialize();
    if (!expect(pulse_scheduler_create_ready_task(&first_task), "first ready task is created") ||
        !expect(pulse_scheduler_create_ready_task(&second_task), "second ready task is created") ||
        !expect(pulse_scheduler_create_ready_task(&third_task), "third ready task is created") ||
        !expect(first_task == 0U && second_task == 1U && third_task == 2U, "task identifiers are monotonic") ||
        !expect(pulse_scheduler_select_next(&selected_task) && selected_task == first_task,
                    "first task is selected first") ||
        !expect(pulse_scheduler_select_next(&selected_task) && selected_task == second_task,
                    "second task follows round robin") ||
        !expect(pulse_scheduler_set_task_state(third_task, PULSE_TASK_BLOCKED), "third task becomes blocked") ||
        !expect(pulse_scheduler_select_next(&selected_task) && selected_task == first_task,
                    "blocked task is skipped") ||
        !expect(pulse_scheduler_set_task_state(first_task, PULSE_TASK_BLOCKED), "first task becomes blocked") ||
        !expect(pulse_scheduler_select_next(&selected_task) && selected_task == second_task,
                    "only runnable task is selected") ||
        !expect(pulse_scheduler_set_task_state(second_task, PULSE_TASK_BLOCKED), "second task becomes blocked") ||
        !expect(!pulse_scheduler_select_next(&selected_task), "no selection exists with no ready task")) {
        return 1;
    }

    puts("Pulse scheduler bootstrap unit tests passed.");
    return 0;
}
