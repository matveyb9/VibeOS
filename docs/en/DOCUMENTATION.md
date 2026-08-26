<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../ru/DOCUMENTATION.md">🇷🇺 РУССКИЙ</a>
</p>

# Documentation policy

**Status:** Accepted project direction

English is the primary language of VibeOS documentation. Russian is the required second language. Every published document has a matching page in `docs/en/` and `docs/ru/`, while the root uses `README.md` and `README_RU.md`.

Each page starts with a centered language switch before its title. The active language is bold; the other language is a direct link to its paired document. Internal links stay within the current language tree. The language switch is the only ordinary link that crosses between language trees.

Documentation has one purpose per page. Explanations describe why a design exists. References and specifications define exact contracts. Guides show how to perform a task. Tutorials teach a complete learning path. ADRs record one architecturally significant decision and its consequences.

All documents begin with a status when the content is technical: `Draft`, `Accepted`, `Superseded`, or `Deprecated`. A change to an accepted ABI, format, or architecture contract requires the matching specification, migration guidance when needed, and an ADR when the decision is significant.

Guides for build, run, and debug must name their host platform, target platform, execution mode, and support status. VibeOS documentation must not silently assume Linux, one CPU architecture, or only QEMU.
