<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/PULSE_PANIC_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Pulse early panic bootstrap

**Status:** Implemented as a terminal diagnostic path for early x86_64 traps.

The first Pulse IDT cannot yet construct a recoverable exception frame or continue execution after an unexpected fault. Its default gate therefore transfers to a terminal panic function. The function disables interrupts, emits a deterministic diagnostic to the QEMU debug channel, requests QEMU debug-exit, and halts if control remains.

`make test` includes a second isolated image build in which Pulse intentionally executes `UD2`. The invalid-opcode exception reaches the default IDT path and must emit:

```text
PULSE PANIC: unhandled interrupt
```

This test validates the negative path separately from the normal breakpoint-trap proof.

## Current limits

The early panic does not yet preserve registers, record the exception vector, display a framebuffer report, enter Pulse Console, or write a recovery artifact. It must remain terminal until a validated trap frame, dedicated IST stack, and recovery-mode policy are available.
