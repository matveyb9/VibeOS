<p align="center">
  <a href="README.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# VibeOS

**VibeOS — независимая графическая настольная операционная система, создаваемая с нуля на C17.** Она предназначена для энтузиастов, которым нужна практическая нативная система, и для читателей, которым полезно изучать явные интерфейсы, небольшие компоненты и парную документацию.

VibeOS не является дистрибутивом Linux. Принятая архитектура намеренно начинается без POSIX profile, Linux ABI, Multiboot, GRUB и заимствованного runtime. Prelude — собственный загрузчик; Dawn Context — firmware-neutral handoff record; Pulse, Atlas, VaultFS, Prism, Canvas, Horizon, Parcel, Relay и Origin — нативные компоненты VibeOS.

## Текущий статус

VibeOS находится на стадии **раннего platform bring-up**, а не является устанавливаемым релизом ОС. Воспроизводимый x86_64 QEMU path теперь достигает общего Pulse runtime через UEFI/OVMF и Legacy BIOS/SeaBIOS Prelude loader. Проверяемые probe охватывают нормализованные Dawn Context v4 memory descriptor и explicit boot-owned reservation, ранний identity map на четыре ГиБ, interrupt/timer handling, bootstrap physical allocator и scheduler state, native component probe и software-rendered вывод Prism/Canvas/Horizon.

Legacy BIOS path использует двухэтапный собственный Prelude image, E820 normalization, VBE `0x118` linear output и framebuffer contract `BGR888`. Он проверен только в QEMU/SeaBIOS. Ни один из путей не является заявлением о совместимости с physical hardware, storage installation, user-space isolation, network support или полноте v1.0.

## Быстрый старт

На подготовленной development host-системе с нужным toolchain и QEMU запустите полный набор проверок:

```text
make test
```

Для отдельного firmware path используйте `make check-uefi` для UEFI/OVMF profile или `make check-bios` для Legacy BIOS/SeaBIOS profile. Руководства по сборке описывают требования к host-системе, создаваемые artifact и текущие границы проверки.

| Цель | Руководство |
|---|---|
| Подготовить Linux, macOS или Windows development host | [Окружения host-систем](docs/ru/guides/HOST_ENVIRONMENTS.md) |
| Собрать и проверить UEFI profile | [x86_64 UEFI](docs/ru/guides/BUILD_X86_64_UEFI.md) |
| Собрать и проверить Legacy BIOS profile | [x86_64 Legacy BIOS](docs/ru/guides/BUILD_X86_64_BIOS.md) |

## Что уже есть в bootstrap

Текущее source tree содержит ранние ограниченные реализации opaque Key, Relay link и channel, Origin ABI v1, Atlas RAM и keyboard probe, VaultFS superblock/journal/A-B state, policy Session и Parcel, а также графический bootstrap Prism/Canvas/Horizon. Они задают интерфейсы и проверяемое поведение; это ещё не полная process model, driver ecosystem, persistent filesystem, package-signing system или desktop application suite, запланированные для VibeOS.

## Краткая дорожная карта

Следующая крупная работа превращает bootstrap в реальные platform facility: explicit physical-memory reservation и virtual-memory policy, hardware discovery и driver, durable VaultFS tree, isolated native process, compositor surface и input, затем установка системы и широкая платформа приложений. Отдельный публичный VibeSDK repository остаётся более поздней вехой.

Последовательность и критерии готовности описаны в [дорожной карте](docs/ru/ROADMAP.md).

## Навигация по проекту

- [Обзор архитектуры](docs/ru/ARCHITECTURE.md) и [организация репозитория](docs/ru/REPOSITORY.md).
- [Dawn Context v4](docs/ru/specs/DAWN_CONTEXT.md), [bootstrap paging Pulse](docs/ru/specs/PULSE_PAGING_BOOTSTRAP.md) и [bootstrap Prism/Canvas](docs/ru/specs/PRISM_CANVAS_BOOTSTRAP.md).
- [Правила документации](docs/ru/DOCUMENTATION.md) и [рабочий процесс вкладов](docs/ru/CONTRIBUTING.md).
- Планируемая [платформа приложений и Vibe SDK](docs/ru/SDK.md).

## Участие

VibeOS готовится как открытый проект. Изменения используют компактные тематические ветки, проверяемые Pull Request, короткие английские темы коммитов с двуязычным описанием и синхронную английскую/русскую документацию.

Перед первым изменением прочтите [руководство по участию](docs/ru/CONTRIBUTING.md).

## Лицензия

Лицензия будет выбрана до принятия первого внешнего вклада с исходным кодом.
