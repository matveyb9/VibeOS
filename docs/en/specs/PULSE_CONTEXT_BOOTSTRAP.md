<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_CONTEXT_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse cooperative context bootstrap

**Status:** Implemented and verified in the normal x86_64 QEMU probe; preemption is not implemented.

Pulse can now create and switch to an independently stacked kernel context. The x86_64 switch preserves the System V callee-saved register set, records the outgoing return point and stack pointer, restores an incoming context, and jumps to its instruction pointer. The probe starts a worker on a dedicated 4 KiB stack; the worker emits `PULSE: task context verified` and cooperatively returns to the bootstrap context.

| Property | Initial behavior |
|---|---|
| Context state | `RBX`, `RBP`, `R12`–`R15`, `RSP`, `RIP` |
| Entry stack | 16-byte aligned with call-compatible entry offset |
| Switch model | Explicit cooperative handoff |
| Probe task | One dedicated kernel worker stack |
| Preemption | Not implemented |
| User-mode contexts | Not implemented |

The context switch is deliberately independent of task-policy selection. The scheduler can decide *which* task should run; a future execution layer will associate each task slot with a complete context and allow a timer interrupt to invoke a safe preemptive switch.
