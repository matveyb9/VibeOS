<p align="center">
  <a href="../../en/guides/HOST_ENVIRONMENTS.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Host-системы для Prelude

**Статус:** Экспериментальное руководство этапа Project Foundation

Для первой цели Prelude x86_64 UEFI VibeOS использует одно дерево исходного кода и единый интерфейс `make`. Инструкции ниже описывают среды разработчика, но пока не утверждают, что все host-профили входят в релизную валидацию.

| Host | Поддержка сборки | Проверка QEMU | Необходимое действие |
|---|---|---|---|
| Linux | Реализована и проверена на Ubuntu 24.04 | Реализована и проверена через QEMU и OVMF | Установить перечисленные пакеты и запустить `make check-uefi`. |
| macOS | Описана, но пока не проверена CI | Описана; требуется явно указать пару прошивок OVMF | Установить Homebrew-пакеты и задать `OVMF_CODE` и `OVMF_VARS`. |
| Windows 10/11 | Описана через MSYS2 MINGW64, но пока не проверена CI | Описана; требуется явно указать пару прошивок OVMF | Использовать MSYS2 MINGW64 shell и задать `OVMF_CODE` и `OVMF_VARS`. |

## Общие команды

Сначала запустите проверку предварительных требований:

```text
tools/check-host.sh
```

Затем соберите и запустите автоматический UEFI-probe:

```text
make check-uefi
```

`OVMF_CODE` и `OVMF_VARS` должны указывать на совместимые файлы прошивки OVMF: код и шаблон variable store. Скрипт проверки копирует шаблон variable store перед каждым запуском, поэтому проверка не изменяет исходный шаблон.

## Linux

На Debian или Ubuntu установите:

```text
sudo apt-get install clang lld qemu-system-x86 ovmf mtools dosfstools make
```

В первом профиле по умолчанию используются `/usr/share/OVMF/OVMF_CODE_4M.fd` и `/usr/share/OVMF/OVMF_VARS_4M.fd`.

## macOS

Установите основные инструменты командной строки и QEMU:

```text
xcode-select --install
brew install llvm qemu make mtools coreutils
```

Убедитесь, что каталог Homebrew LLVM с бинарными файлами находится в `PATH` раньше Apple toolchain, затем получите совместимый x86_64 OVMF code image и шаблон variable store. OVMF — реализация UEFI-прошивки EDK II для виртуальных машин QEMU/KVM.[1] Укажите пути до запуска общих команд:

```text
export OVMF_CODE=/absolute/path/to/OVMF_CODE.fd
export OVMF_VARS=/absolute/path/to/OVMF_VARS.fd
export PATH="$(brew --prefix llvm)/bin:$PATH"
```

Используйте `gtimeout` из `coreutils`: скрипт проверки выберет его автоматически, если `timeout` отсутствует. QEMU документирует Homebrew как поддерживаемый способ установки на macOS.[2]

## Windows 10 или 11

Установите MSYS2, обновите его и затем откройте **MINGW64** shell. На странице загрузки QEMU задокументировано имя пакета MINGW64 для 64-разрядной Windows.[2]

```text
pacman -Syu
pacman -S mingw-w64-x86_64-clang mingw-w64-x86_64-lld mingw-w64-x86_64-qemu mingw-w64-x86_64-mtools make coreutils
```

Разместите совместимые x86_64 OVMF-файлы в постоянном месте, например в `C:/VibeOS/firmware`, и задайте их в MSYS2 shell:

```text
export OVMF_CODE=/c/VibeOS/firmware/OVMF_CODE.fd
export OVMF_VARS=/c/VibeOS/firmware/OVMF_VARS.fd
```

Запустите `tools/check-host.sh`, затем `make check-uefi`. Этот профиль намеренно использует только интерфейс MSYS2 shell; PowerShell- и native batch-обёртки не будут добавляться, пока их нельзя проверить в проекте.

## Источники

[1] [TianoCore: OVMF](https://www.tianocore.org/tianocore-wiki.github.io/platforms-packages/platform-ports/ovmf.html)

[2] [QEMU: Download and host installation paths](https://www.qemu.org/download/)
