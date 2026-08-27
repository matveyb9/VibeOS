<p align="center">
  <a href="../../en/specs/PULSE_CONTEXT_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap cooperative context Pulse

**Статус:** Реализован и проверен в normal x86_64 QEMU-probe; preemption пока не реализован.

Теперь Pulse может создавать и переключать независимый kernel context со своим stack. x86_64 switch сохраняет набор System V callee-saved register, записывает исходящие return point и stack pointer, восстанавливает входящий context и переходит по его instruction pointer. Probe запускает worker на выделенном stack размером 4 КиБ; worker выводит `PULSE: task context verified` и cooperative возвращается к bootstrap context.

| Свойство | Начальное поведение |
|---|---|
| Состояние context | `RBX`, `RBP`, `R12`–`R15`, `RSP`, `RIP` |
| Entry stack | Выравнивание 16 байт с call-compatible entry offset |
| Модель switch | Явная cooperative передача управления |
| Probe task | Один выделенный kernel worker stack |
| Preemption | Не реализован |
| User-mode context | Не реализованы |

Context switch намеренно независим от выбора scheduler policy. Scheduler может определить, *какая* task должна выполняться; будущий execution layer свяжет каждый task slot с полным context и позволит timer interrupt вызвать безопасный preemptive switch.
