<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../../ru/specs/SESSION_MODES_BOOTSTRAP.md">🇷🇺 РУССКИЙ</a>
</p>

# Session-mode bootstrap

**Status:** Implemented as a pure launch policy; interactive selection and physical media discovery are not yet implemented.

VibeOS now has one explicit policy boundary for the planned boot modes. The policy distinguishes Live, Live with Vault persistence, installed, and Recovery sessions. It never selects an installed session without both an available and valid Vault; that condition instead enters Recovery. A persistent-live request without usable persistence degrades safely to non-persistent Live. An explicit recovery request overrides every other mode.

| Requested mode | Vault condition | Selected mode |
|---|---|---|
| Live | Any | Live |
| Persistent Live | Missing or invalid | Live |
| Persistent Live | Available and valid | Persistent Live |
| Installed | Missing or invalid | Recovery |
| Installed | Available and valid | Installed |
| Any | Recovery requested | Recovery |

The QEMU and host probes exercise all safety transitions. Prelude UI, hardware media discovery through Atlas, persistence mounting, installation writes, and Recovery interaction remain later integrations; this module is the narrow policy contract those layers must satisfy.
