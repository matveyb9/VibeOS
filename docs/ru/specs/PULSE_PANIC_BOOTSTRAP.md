<p align="center">
  <a href="../../en/specs/PULSE_PANIC_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Ранний panic bootstrap Pulse

**Статус:** Реализован как terminal diagnostic path для ранних x86_64 trap.

Первая IDT Pulse пока не умеет создать восстанавливаемый exception frame или продолжить работу после неожиданной fault. Поэтому default gate передаёт управление terminal panic-функции. Она выключает interrupts, выводит детерминированную диагностику в QEMU debug channel, запрашивает QEMU debug-exit и останавливает процессор, если управление сохраняется.

`make test` включает вторую изолированную сборку образа, в которой Pulse намеренно выполняет `UD2`. Invalid-opcode exception должна достигнуть default IDT path и вывести:

```text
PULSE PANIC: unhandled interrupt
```

Этот тест проверяет negative path отдельно от нормального breakpoint-trap proof.

## Текущие ограничения

Ранний panic пока не сохраняет register, не записывает exception vector, не показывает framebuffer report, не запускает Pulse Console и не создаёт recovery artifact. Он должен оставаться terminal до появления проверенных trap frame, выделенного IST stack и policy режима восстановления.
