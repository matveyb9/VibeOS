<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_SCHEDULER_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse scheduler-state bootstrap

**Status:** Implemented for early policy verification; CPU context switching is not yet implemented.

Pulse now owns a fixed-capacity task-state table and deterministic round-robin selection policy. A task is created in `READY`, selected as `RUNNING`, returned to `READY` on the next selection, or explicitly marked `BLOCKED`. The implementation is deliberately separate from the future context-switch layer.

| Property | Initial behavior |
|---|---|
| Capacity | 32 task slots |
| Identifiers | Monotonic 32-bit values from `0` |
| Policy | Round robin across `READY` tasks |
| Blocking | `BLOCKED` tasks are skipped |
| Synchronization | None; interrupts remain disabled |
| Context switching | Not implemented |

Pulse verifies the state machine during boot by creating two ready tasks and observing their first round-robin order before exercising the IDT breakpoint proof. `make test` also runs independent policy checks for ordering, blocking, and the no-ready-task outcome.

This module intentionally does not create executable tasks, save registers, manipulate stacks, or enable a timer. Those actions require the next interrupt work: normalised frames, a return-capable exception path, and an interrupt-controller/timer route.
