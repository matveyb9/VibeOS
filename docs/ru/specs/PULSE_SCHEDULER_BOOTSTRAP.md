<p align="center">
  <a href="../../en/specs/PULSE_SCHEDULER_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap состояния scheduler Pulse

**Статус:** Реализован для проверки ранней policy; CPU context switching пока не реализован.

Теперь Pulse владеет task-state table фиксированной ёмкости и детерминированной round-robin policy выбора. Задача создаётся в `READY`, выбирается как `RUNNING`, при следующем выборе возвращается в `READY` либо явно отмечается `BLOCKED`. Реализация намеренно отделена от будущего слоя context switch.

| Свойство | Начальное поведение |
|---|---|
| Ёмкость | 32 task slot |
| Идентификаторы | Монотонные 32-битные значения от `0` |
| Policy | Round robin между `READY` task |
| Блокировка | `BLOCKED` task пропускаются |
| Синхронизация | Нет; interrupts остаются выключенными |
| Context switching | Не реализован |

Во время загрузки Pulse проверяет state machine: создаёт две ready task и наблюдает их первый round-robin порядок перед IDT breakpoint proof. `make test` также запускает независимые policy-проверки порядка, блокировки и случая отсутствия ready task.

Этот модуль намеренно не создаёт исполняемые задачи, не сохраняет register, не управляет stack и не включает timer. Эти действия требуют следующей работы с interrupts: нормализованных frame, return-capable exception path и маршрута interrupt controller/timer.
