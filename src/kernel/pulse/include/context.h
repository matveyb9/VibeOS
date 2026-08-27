/* VibeOS Pulse — cooperative x86_64 kernel-context bootstrap contract. */

#ifndef VIBEOS_PULSE_CONTEXT_H
#define VIBEOS_PULSE_CONTEXT_H

#include <stdint.h>

typedef struct {
    uint64_t rbx;
    uint64_t rbp;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t stack_pointer;
    uint64_t instruction_pointer;
} PULSE_X86_CONTEXT;

typedef void (*PULSE_CONTEXT_ENTRY)(void);

int pulse_context_prepare(
    PULSE_X86_CONTEXT *context,
    void *stack_base,
    uint64_t stack_size,
    PULSE_CONTEXT_ENTRY entry);
int pulse_context_run_probe(void);
void pulse_context_switch(PULSE_X86_CONTEXT *outgoing, const PULSE_X86_CONTEXT *incoming);

#endif
