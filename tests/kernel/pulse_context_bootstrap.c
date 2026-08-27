/* VibeOS Pulse — host checks for initial cooperative context construction. */

#include <stdint.h>
#include <stdio.h>

#include "context.h"

void pulse_context_switch(PULSE_X86_CONTEXT *outgoing, const PULSE_X86_CONTEXT *incoming) {
    (void)outgoing;
    (void)incoming;
}

static void test_entry(void) {}

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "failed: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    PULSE_X86_CONTEXT context;
    uint8_t stack[129];
    uintptr_t expected_top = ((uintptr_t)stack + sizeof(stack)) & ~(uintptr_t)0x0fU;

    if (!expect(!pulse_context_prepare((void *)0, stack, sizeof(stack), test_entry),
                    "null context is rejected") ||
        !expect(!pulse_context_prepare(&context, stack, 8U, test_entry),
                    "undersized stack is rejected") ||
        !expect(pulse_context_prepare(&context, stack, sizeof(stack), test_entry),
                    "aligned initial context is created")) {
        return 1;
    }

    if (!expect(context.stack_pointer == expected_top - 8U, "entry stack has SysV call alignment") ||
        !expect(context.instruction_pointer == (uint64_t)(uintptr_t)test_entry,
                    "entry instruction pointer is preserved") ||
        !expect(context.rbx == 0U && context.r15 == 0U, "callee-saved state is initialized")) {
        return 1;
    }

    puts("Pulse context bootstrap unit tests passed.");
    return 0;
}
