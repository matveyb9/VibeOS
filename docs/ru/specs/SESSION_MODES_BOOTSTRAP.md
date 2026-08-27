<p align="center">
  <a href="../../en/specs/SESSION_MODES_BOOTSTRAP.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Bootstrap session mode

**Статус:** Реализован как чистая launch policy; interactive selection и physical media discovery пока не реализованы.

Теперь VibeOS имеет одну явную policy boundary для планируемых boot mode. Policy различает Live, Live с Vault persistence, installed и Recovery session. Она никогда не выбирает installed session без одновременно доступного и valid Vault; вместо этого выбирается Recovery. Запрос persistent-live без пригодного persistence безопасно деградирует к Live без persistence. Явный запрос recovery имеет приоритет над любым другим mode.

| Запрошенный mode | Состояние Vault | Выбранный mode |
|---|---|---|
| Live | Любое | Live |
| Persistent Live | Нет или invalid | Live |
| Persistent Live | Доступен и valid | Persistent Live |
| Installed | Нет или invalid | Recovery |
| Installed | Доступен и valid | Installed |
| Любой | Запрошен Recovery | Recovery |

QEMU- и host-probe покрывают все безопасные переходы. Prelude UI, hardware media discovery через Atlas, mounting persistence, installation write и Recovery interaction останутся более поздними интеграциями; этот модуль является узким policy contract, которому эти слои должны соответствовать.
