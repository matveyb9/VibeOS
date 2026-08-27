<p align="center">
  <a href="../../en/guides/BUILD_X86_64_BIOS.md">🇺🇸 ENGLISH</a> &nbsp;|&nbsp; <strong>🇷🇺 РУССКИЙ</strong>
</p>

# Сборка и запуск: x86_64 Legacy BIOS

**Статус:** Реализовано для воспроизводимого профиля QEMU/SeaBIOS. Это собственный путь Prelude; он не использует Multiboot, GRUB или другой bootloader.

Legacy BIOS path создаёт `build/vibeos-bios.img`. Его 512-byte first stage загружает отдельный Prelude stage two. Stage two загружает raw Pulse image через повторные 64-sector INT 13h extension read, нормализует BIOS E820 map в Dawn v4 descriptor, запрашивает VBE 2.0 linear mode `1024×768×24` (`0x118`), включает x86_64 long mode и передаёт Pulse тот же Dawn Context contract, который формирует UEFI Prelude.[1] [2]

## Требования

| Семейство host-системы | Нужные инструменты |
|---|---|
| Linux | `clang`, `lld`, LLVM `objcopy`, `nasm`, QEMU и стандартные shell tools. |
| macOS | LLVM, NASM, QEMU и POSIX-compatible shell. Распространённые packages предоставляет Homebrew. |
| Windows | MSYS2 UCRT64 или MINGW64 с LLVM, NASM, QEMU, GNU Make и Bash-compatible shell. |

Используйте тот же repository checkout и toolchain validation, что описаны в [Host environments](HOST_ENVIRONMENTS.md). Exact executable path можно передать как `QEMU_X86_64=/path/to/qemu-system-x86_64`.

## Команды

```bash
make bios-image
make check-bios
```

`make check-bios` запускает image на QEMU PC machine, ожидает marker Prelude `Dawn Context`, затем подтверждает выполнение общего Pulse timer path. `make test` включает эту проверку вместе с UEFI, keyboard, panic, artifact и host unit check.

## Граница staging payload

Stage two размещает Pulse во временной physical области `0x10000` и переносит его по execution address после входа в long mode. Каждый INT 13h extension request читает не более 64 sector, затем сдвигает и disk LBA, и destination segment. Поэтому QEMU profile проверяет несколько чтений, когда raw Pulse image превышает 64 sector; текущий проверяемый image это делает.

Сборка отклоняет raw Pulse image больше **512 KiB** — текущей staging области от `0x10000` до `0x8ffff`. Это намеренно проверяемая bootstrap-граница, а не лимит одного disk-transfer request. В будущем loader, способный выполнять установку, заменит этот fixed staging scheme disk format и loader layout, которые масштабируются дальше.

## Границы и безопасность

Этот profile пока проверен только на legacy BIOS emulation QEMU. Он владеет E820 collection и VBE mode choice, пока доступны BIOS service. Dawn Context v4 публикует первый MiB и загруженный Pulse image как explicit boot-owned reservation; затем Pulse исключает каждый покрытый frame 4 KiB, даже когда E820 сообщает, что он usable. Первый range содержит active BIOS bootstrap, temporary Pulse staging data и transition page table. Physical legacy-BIOS hardware, alternate video mode, USB input, ACPI device discovery и disk-filesystem loading пока не являются поддерживаемыми claims. Не записывайте development image на physical media, пока явно не проверены target hardware и recovery procedure.

## Источники

[1] [Intel® 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

[2] [Документация Linux kernel: vesafb](https://docs.kernel.org/fb/vesafb.html)
