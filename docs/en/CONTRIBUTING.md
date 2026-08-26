<p align="center">
  <strong>🇺🇸 ENGLISH</strong> &nbsp;|&nbsp; <a href="../ru/CONTRIBUTING.md">🇷🇺 РУССКИЙ</a>
</p>

# Contributing to VibeOS

**Status:** Accepted project direction

VibeOS uses short topic branches and pull requests. The `main` branch is intended to remain buildable and bootable in the supported QEMU profile once the toolchain exists.

## Branches

Use lowercase English names with a clear prefix: `feature/`, `fix/`, `docs/`, `spec/`, `test/`, `refactor/`, `security/`, or `release/`. One branch solves one related task.

## Commits

Use an English subject in this form:

```text
type(scope): short imperative description
```

Every commit body includes both language blocks:

```text
**🇺🇸 [EN]**: Detailed English description.

**🇷🇺 [РУ]**: Подробное описание на русском языке.
```

Use `feat`, `fix`, `docs`, `spec`, `test`, `refactor`, `build`, `ci`, `security`, or `chore`. A breaking contract change adds an English `BREAKING CHANGE:` footer after both language blocks.

## Pull requests

Pull request titles use the same English subject format. The English description is required. A Russian summary is recommended, and it is required when the change affects security, ABI, VaultFS, boot, or another accepted architecture contract.

Every pull request explains purpose, behaviour changes, validation, documentation impact, compatibility impact, and security impact. No change is complete while an accepted document describes outdated behaviour.

## Releases

Release notes include an English section followed by a Russian section. Both headings are bold: **🇺🇸 ENGLISH** and **🇷🇺 РУССКИЙ**. Published releases will include signed images, checksums, support status, known limitations, and update or rollback guidance.
