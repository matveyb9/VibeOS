<p align="center">
  <a href="../../en/guides/BUILD_X86_64_UEFI.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Сборка и проверка Prelude на x86_64 UEFI

**Статус:** Экспериментальное руководство этапа Project Foundation

**Host:** Linux x86_64 или ARM64 с доступными перечисленными пакетами.

**Цель:** QEMU x86_64 UEFI.

**Путь загрузки:** UEFI.
**Поддержка:** Первый обязательный профиль проверки.

## Предварительные требования

Установите Clang, LLD, QEMU System x86, OVMF, mtools и основные инструменты сборки. На Debian или Ubuntu используются пакеты `clang`, `lld`, `qemu-system-x86`, `ovmf`, `mtools`, `dosfstools` и `make`.

## Сборка

```text
make uefi-image
```

Команда создаёт `build/prelude/BOOTX64.EFI` и FAT ESP-образ в `build/vibeos-uefi-esp.img`.

## Проверка

```text
make check-uefi
```

Профиль проверки запускает OVMF в QEMU, загружает независимое UEFI-приложение Prelude и проверяет диагностическую метку, которая выводится после успешного доступа к firmware console.

## Ожидаемый результат

```text
Prelude UEFI verification passed.
```

Руководства для host-систем Windows и macOS будут добавлены вместе с эквивалентными проверенными профилями toolchain. Пока они не заявляются реализованными.
