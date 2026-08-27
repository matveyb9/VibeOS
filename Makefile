# VibeOS Project Foundation — x86_64 UEFI Prelude and Pulse bring-up.
# The build deliberately targets a single reproducible QEMU profile first.

SHELL := /bin/bash

BUILD_DIR ?= build
PULSE_PROBE ?= timer
PRELUDE_DIR := src/boot/prelude
PRELUDE_BIOS_DIR := $(PRELUDE_DIR)/bios
PULSE_DIR := src/kernel/pulse

PRELUDE_OBJ := $(BUILD_DIR)/prelude/main.obj
PRELUDE_DAWN_ACPI_OBJ := $(BUILD_DIR)/prelude/dawn/acpi-rsdp.obj
PRELUDE_BLOB_OBJ := $(BUILD_DIR)/prelude/embedded-pulse.obj
PRELUDE_EFI := $(BUILD_DIR)/prelude/BOOTX64.EFI
PULSE_ENTRY_OBJ := $(BUILD_DIR)/pulse/entry.obj
PULSE_INTERRUPTS_ENTRY_OBJ := $(BUILD_DIR)/pulse/interrupts/x86_64.obj
PULSE_CONTEXT_ENTRY_OBJ := $(BUILD_DIR)/pulse/schedule/context-x86_64.obj
PULSE_MAIN_OBJ := $(BUILD_DIR)/pulse/main.obj
PULSE_DAWN_ACPI_OBJ := $(BUILD_DIR)/pulse/dawn/acpi-rsdp.obj
PULSE_MEMORY_OBJ := $(BUILD_DIR)/pulse/memory/early.obj
PULSE_PAGING_OBJ := $(BUILD_DIR)/pulse/memory/paging-x86_64.obj
PULSE_INTERRUPTS_OBJ := $(BUILD_DIR)/pulse/interrupts/idt-x86_64.obj
PULSE_SCHEDULER_OBJ := $(BUILD_DIR)/pulse/schedule/round-robin.obj
PULSE_CONTEXT_OBJ := $(BUILD_DIR)/pulse/schedule/context.obj
PULSE_TIMER_OBJ := $(BUILD_DIR)/pulse/time/pit-x86_64.obj
PULSE_PIC_OBJ := $(BUILD_DIR)/pulse/interrupts/pic-x86_64.obj
PULSE_KEYS_OBJ := $(BUILD_DIR)/security/keys/key-space.obj
PULSE_RELAY_OBJ := $(BUILD_DIR)/ipc/relay/link.obj
PULSE_RELAY_CHANNEL_OBJ := $(BUILD_DIR)/ipc/relay/channel.obj
PULSE_ORIGIN_OBJ := $(BUILD_DIR)/runtime/origin/origin.obj
PULSE_ORIGIN_ABI_OBJ := $(BUILD_DIR)/runtime/origin/abi.obj
PULSE_ATLAS_RAM_OBJ := $(BUILD_DIR)/drivers/atlas/ram-block.obj
PULSE_ATLAS_KEYBOARD_OBJ := $(BUILD_DIR)/drivers/atlas/i8042-keyboard.obj
PULSE_ATLAS_KEYBOARD_IRQ_OBJ := $(BUILD_DIR)/drivers/atlas/i8042-keyboard-irq.obj
PULSE_ATLAS_PCI_OBJ := $(BUILD_DIR)/drivers/atlas/pci-inventory.obj
PULSE_VAULT_OBJ := $(BUILD_DIR)/storage/vault/vaultfs.obj
PULSE_SESSION_OBJ := $(BUILD_DIR)/services/session/boot-mode.obj
PULSE_PARCEL_OBJ := $(BUILD_DIR)/services/parcel/parcel.obj
PULSE_PRISM_OBJ := $(BUILD_DIR)/ui/prism/framebuffer.obj
PULSE_PRISM_BOOTSTRAP_OBJ := $(BUILD_DIR)/ui/prism/bootstrap.obj
PULSE_CANVAS_OBJ := $(BUILD_DIR)/ui/canvas/scene.obj
PULSE_HORIZON_OBJ := $(BUILD_DIR)/apps/horizon/shell.obj
PULSE_HORIZON_FOCUS_OBJ := $(BUILD_DIR)/apps/horizon/focus.obj
PULSE_PANIC_OBJ := $(BUILD_DIR)/pulse/debug/panic.obj
PULSE_ELF := $(BUILD_DIR)/pulse/PULSE.ELF
PULSE_BIN := $(BUILD_DIR)/pulse/pulse.bin
PULSE_MEMORY_TEST := $(BUILD_DIR)/tests/pulse-memory-bootstrap
PULSE_PAGING_TEST := $(BUILD_DIR)/tests/pulse-paging-bootstrap
PULSE_INTERRUPTS_TEST := $(BUILD_DIR)/tests/pulse-interrupts-bootstrap
PULSE_SCHEDULER_TEST := $(BUILD_DIR)/tests/pulse-scheduler-bootstrap
PULSE_CONTEXT_TEST := $(BUILD_DIR)/tests/pulse-context-bootstrap
PULSE_TIMER_TEST := $(BUILD_DIR)/tests/pulse-timer-bootstrap
PULSE_RELAY_TEST := $(BUILD_DIR)/tests/pulse-relay-bootstrap
PULSE_VAULT_TEST := $(BUILD_DIR)/tests/pulse-vault-bootstrap
PULSE_SESSION_TEST := $(BUILD_DIR)/tests/pulse-session-bootstrap
PULSE_PARCEL_TEST := $(BUILD_DIR)/tests/pulse-parcel-bootstrap
PULSE_CANVAS_TEST := $(BUILD_DIR)/tests/pulse-canvas-bootstrap
PULSE_HORIZON_TEST := $(BUILD_DIR)/tests/pulse-horizon-bootstrap
PULSE_HORIZON_FOCUS_TEST := $(BUILD_DIR)/tests/horizon-focus
PULSE_KEYBOARD_TEST := $(BUILD_DIR)/tests/pulse-keyboard-bootstrap
PULSE_PCI_TEST := $(BUILD_DIR)/tests/pulse-pci-bootstrap
DAWN_ACPI_TEST := $(BUILD_DIR)/tests/dawn-acpi-rsdp
ESP_IMAGE := $(BUILD_DIR)/vibeos-uefi-esp.img
BIOS_STAGE1_BIN := $(BUILD_DIR)/prelude/bios/stage1.bin
BIOS_STAGE2_BIN := $(BUILD_DIR)/prelude/bios/stage2.bin
BIOS_IMAGE := $(BUILD_DIR)/vibeos-bios.img
ESP_IMAGE_BYTES := 67108864
PRELUDE_BIOS_STAGE2_SECTORS := 16
PRELUDE_BIOS_PULSE_STAGING_BYTES := 524288

CLANG ?= clang
LLD_LINK ?= lld-link
LLD_LD ?= ld.lld
LLVM_OBJCOPY ?= llvm-objcopy
HOST_CC ?= cc
NASM ?= nasm

PRELUDE_CFLAGS := --target=x86_64-pc-windows-msvc -std=c17 -ffreestanding -fshort-wchar \
	-fno-stack-protector -mno-red-zone -Wall -Wextra -Wpedantic -Werror \
	-I$(PRELUDE_DIR)/include -Isrc/platform/dawn/include
PULSE_CFLAGS := --target=x86_64-unknown-elf -std=c17 -ffreestanding -fno-stack-protector \
	-fno-pic -fno-pie -mno-red-zone -Wall -Wextra -Wpedantic -Werror \
	-DPULSE_PROBE_$(PULSE_PROBE) -Isrc/platform/dawn/include -I$(PULSE_DIR)/include \
	-Isrc/security/keys/include -Isrc/ipc/relay/include -Isrc/runtime/origin/include \
	-Isrc/drivers/atlas/include -Isrc/storage/vault/include -Isrc/services/session/include \
	-Isrc/services/parcel/include -Isrc/ui/prism/include -Isrc/ui/canvas/include \
	-Isrc/apps/horizon/include
PULSE_ASFLAGS := --target=x86_64-unknown-elf -ffreestanding -mno-red-zone

.PHONY: all prelude pulse uefi-image bios-image check-uefi check-keyboard check-bios check-panic test test-bios test-panic clean

all: uefi-image

prelude: $(PRELUDE_EFI)

pulse: $(PULSE_ELF)

uefi-image: $(ESP_IMAGE)

bios-image: $(BIOS_IMAGE)

$(PULSE_ENTRY_OBJ): $(PULSE_DIR)/entry/x86_64.S
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_ASFLAGS) -c $< -o $@

$(PULSE_INTERRUPTS_ENTRY_OBJ): $(PULSE_DIR)/interrupts/x86_64.S
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_ASFLAGS) -c $< -o $@

$(PULSE_CONTEXT_ENTRY_OBJ): $(PULSE_DIR)/schedule/context-x86_64.S
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_ASFLAGS) -c $< -o $@

$(PULSE_MAIN_OBJ): $(PULSE_DIR)/main.c src/platform/dawn/include/dawn.h src/platform/dawn/include/dawn_acpi.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_DAWN_ACPI_OBJ): src/platform/dawn/acpi-rsdp.c src/platform/dawn/include/dawn_acpi.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_MEMORY_OBJ): $(PULSE_DIR)/memory/early.c $(PULSE_DIR)/include/memory.h src/platform/dawn/include/dawn.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PAGING_OBJ): $(PULSE_DIR)/memory/paging-x86_64.c $(PULSE_DIR)/include/paging.h $(PULSE_DIR)/include/memory.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_INTERRUPTS_OBJ): $(PULSE_DIR)/interrupts/idt-x86_64.c $(PULSE_DIR)/include/interrupts.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_SCHEDULER_OBJ): $(PULSE_DIR)/schedule/round-robin.c $(PULSE_DIR)/include/scheduler.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_CONTEXT_OBJ): $(PULSE_DIR)/schedule/context.c $(PULSE_DIR)/include/context.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_TIMER_OBJ): $(PULSE_DIR)/time/pit-x86_64.c $(PULSE_DIR)/include/timer.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PIC_OBJ): $(PULSE_DIR)/interrupts/pic-x86_64.c $(PULSE_DIR)/include/pic.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_KEYS_OBJ): src/security/keys/key-space.c src/security/keys/include/keys.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_RELAY_OBJ): src/ipc/relay/link.c src/ipc/relay/include/relay.h src/security/keys/include/keys.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_RELAY_CHANNEL_OBJ): src/ipc/relay/channel.c src/ipc/relay/include/relay.h src/security/keys/include/keys.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ORIGIN_OBJ): src/runtime/origin/origin.c src/runtime/origin/include/origin.h src/ipc/relay/include/relay.h src/security/keys/include/keys.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ORIGIN_ABI_OBJ): src/runtime/origin/abi.c src/runtime/origin/include/origin_abi.h src/security/keys/include/keys.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ATLAS_RAM_OBJ): src/drivers/atlas/ram-block.c src/drivers/atlas/include/atlas_block.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ATLAS_KEYBOARD_OBJ): src/drivers/atlas/i8042-keyboard.c src/drivers/atlas/include/atlas_input.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ATLAS_KEYBOARD_IRQ_OBJ): src/drivers/atlas/i8042-keyboard-irq.c src/drivers/atlas/include/atlas_input.h $(PULSE_DIR)/include/pic.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ATLAS_PCI_OBJ): src/drivers/atlas/pci-inventory.c src/drivers/atlas/include/atlas_pci.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_VAULT_OBJ): src/storage/vault/vaultfs.c src/storage/vault/include/vaultfs.h src/drivers/atlas/include/atlas_block.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_SESSION_OBJ): src/services/session/boot-mode.c src/services/session/include/session_mode.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PARCEL_OBJ): src/services/parcel/parcel.c src/services/parcel/include/parcel.h src/security/keys/include/keys.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PRISM_OBJ): src/ui/prism/framebuffer.c src/ui/prism/include/prism.h src/platform/dawn/include/dawn.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PRISM_BOOTSTRAP_OBJ): src/ui/prism/bootstrap.c src/ui/prism/include/prism.h src/ui/canvas/include/canvas.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_CANVAS_OBJ): src/ui/canvas/scene.c src/ui/canvas/include/canvas.h src/ui/prism/include/prism.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_HORIZON_OBJ): src/apps/horizon/shell.c src/apps/horizon/include/horizon.h src/ui/canvas/include/canvas.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_HORIZON_FOCUS_OBJ): src/apps/horizon/focus.c src/apps/horizon/include/horizon.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PANIC_OBJ): $(PULSE_DIR)/debug/panic.c $(PULSE_DIR)/include/panic.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ELF): $(PULSE_ENTRY_OBJ) $(PULSE_INTERRUPTS_ENTRY_OBJ) $(PULSE_CONTEXT_ENTRY_OBJ) $(PULSE_MAIN_OBJ) $(PULSE_DAWN_ACPI_OBJ) $(PULSE_MEMORY_OBJ) $(PULSE_PAGING_OBJ) $(PULSE_INTERRUPTS_OBJ) $(PULSE_SCHEDULER_OBJ) $(PULSE_CONTEXT_OBJ) $(PULSE_TIMER_OBJ) $(PULSE_PIC_OBJ) $(PULSE_KEYS_OBJ) $(PULSE_RELAY_OBJ) $(PULSE_RELAY_CHANNEL_OBJ) $(PULSE_ORIGIN_OBJ) $(PULSE_ORIGIN_ABI_OBJ) $(PULSE_ATLAS_RAM_OBJ) $(PULSE_ATLAS_KEYBOARD_OBJ) $(PULSE_ATLAS_KEYBOARD_IRQ_OBJ) $(PULSE_ATLAS_PCI_OBJ) $(PULSE_VAULT_OBJ) $(PULSE_SESSION_OBJ) $(PULSE_PARCEL_OBJ) $(PULSE_PRISM_OBJ) $(PULSE_PRISM_BOOTSTRAP_OBJ) $(PULSE_CANVAS_OBJ) $(PULSE_HORIZON_OBJ) $(PULSE_HORIZON_FOCUS_OBJ) $(PULSE_PANIC_OBJ) $(PULSE_DIR)/linker/x86_64.ld
	@mkdir -p $(dir $@)
	$(LLD_LD) -m elf_x86_64 -nostdlib --build-id=none -T $(PULSE_DIR)/linker/x86_64.ld \
		-o $@ $(PULSE_ENTRY_OBJ) $(PULSE_INTERRUPTS_ENTRY_OBJ) $(PULSE_CONTEXT_ENTRY_OBJ) $(PULSE_MAIN_OBJ) $(PULSE_DAWN_ACPI_OBJ) $(PULSE_MEMORY_OBJ) $(PULSE_PAGING_OBJ) $(PULSE_INTERRUPTS_OBJ) $(PULSE_SCHEDULER_OBJ) $(PULSE_CONTEXT_OBJ) $(PULSE_TIMER_OBJ) $(PULSE_PIC_OBJ) $(PULSE_KEYS_OBJ) $(PULSE_RELAY_OBJ) $(PULSE_RELAY_CHANNEL_OBJ) $(PULSE_ORIGIN_OBJ) $(PULSE_ORIGIN_ABI_OBJ) $(PULSE_ATLAS_RAM_OBJ) $(PULSE_ATLAS_KEYBOARD_OBJ) $(PULSE_ATLAS_KEYBOARD_IRQ_OBJ) $(PULSE_ATLAS_PCI_OBJ) $(PULSE_VAULT_OBJ) $(PULSE_SESSION_OBJ) $(PULSE_PARCEL_OBJ) $(PULSE_PRISM_OBJ) $(PULSE_PRISM_BOOTSTRAP_OBJ) $(PULSE_CANVAS_OBJ) $(PULSE_HORIZON_OBJ) $(PULSE_HORIZON_FOCUS_OBJ) $(PULSE_PANIC_OBJ)

$(PULSE_BIN): $(PULSE_ELF)
	$(LLVM_OBJCOPY) -O binary --set-section-flags .bss=alloc,load,contents $< $@

$(PULSE_MEMORY_TEST): tests/kernel/pulse_memory_bootstrap.c $(PULSE_DIR)/memory/early.c $(PULSE_DIR)/include/memory.h src/platform/dawn/include/dawn.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include -Isrc/platform/dawn/include \
		tests/kernel/pulse_memory_bootstrap.c $(PULSE_DIR)/memory/early.c -o $@

$(PULSE_PAGING_TEST): tests/kernel/pulse_paging_bootstrap.c $(PULSE_DIR)/memory/paging-x86_64.c $(PULSE_DIR)/memory/early.c $(PULSE_DIR)/include/paging.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include -Isrc/platform/dawn/include \
		tests/kernel/pulse_paging_bootstrap.c $(PULSE_DIR)/memory/paging-x86_64.c $(PULSE_DIR)/memory/early.c -o $@

$(PULSE_INTERRUPTS_TEST): tests/kernel/pulse_interrupts_bootstrap.c $(PULSE_DIR)/interrupts/idt-x86_64.c $(PULSE_DIR)/include/interrupts.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include \
		tests/kernel/pulse_interrupts_bootstrap.c $(PULSE_DIR)/interrupts/idt-x86_64.c -o $@

$(PULSE_SCHEDULER_TEST): tests/kernel/pulse_scheduler_bootstrap.c $(PULSE_DIR)/schedule/round-robin.c $(PULSE_DIR)/include/scheduler.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include \
		tests/kernel/pulse_scheduler_bootstrap.c $(PULSE_DIR)/schedule/round-robin.c -o $@

$(PULSE_CONTEXT_TEST): tests/kernel/pulse_context_bootstrap.c $(PULSE_DIR)/schedule/context.c $(PULSE_DIR)/include/context.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include \
		tests/kernel/pulse_context_bootstrap.c $(PULSE_DIR)/schedule/context.c -o $@

$(PULSE_TIMER_TEST): tests/kernel/pulse_timer_bootstrap.c $(PULSE_DIR)/time/pit-x86_64.c $(PULSE_DIR)/interrupts/pic-x86_64.c $(PULSE_DIR)/include/timer.h $(PULSE_DIR)/include/pic.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include \
		tests/kernel/pulse_timer_bootstrap.c $(PULSE_DIR)/time/pit-x86_64.c $(PULSE_DIR)/interrupts/pic-x86_64.c -o $@

$(PULSE_RELAY_TEST): tests/kernel/pulse_relay_bootstrap.c src/security/keys/key-space.c src/ipc/relay/link.c src/ipc/relay/channel.c src/runtime/origin/origin.c src/runtime/origin/abi.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/security/keys/include -Isrc/ipc/relay/include -Isrc/runtime/origin/include \
		tests/kernel/pulse_relay_bootstrap.c src/security/keys/key-space.c src/ipc/relay/link.c src/ipc/relay/channel.c src/runtime/origin/origin.c src/runtime/origin/abi.c -o $@

$(PULSE_VAULT_TEST): tests/kernel/pulse_vault_bootstrap.c src/drivers/atlas/ram-block.c src/storage/vault/vaultfs.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/drivers/atlas/include -Isrc/storage/vault/include \
		tests/kernel/pulse_vault_bootstrap.c src/drivers/atlas/ram-block.c src/storage/vault/vaultfs.c -o $@

$(PULSE_SESSION_TEST): tests/kernel/pulse_session_bootstrap.c src/services/session/boot-mode.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/services/session/include \
		tests/kernel/pulse_session_bootstrap.c src/services/session/boot-mode.c -o $@

$(PULSE_PARCEL_TEST): tests/kernel/pulse_parcel_bootstrap.c src/security/keys/key-space.c src/services/parcel/parcel.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/security/keys/include -Isrc/services/parcel/include \
		tests/kernel/pulse_parcel_bootstrap.c src/security/keys/key-space.c src/services/parcel/parcel.c -o $@

$(PULSE_CANVAS_TEST): tests/kernel/pulse_canvas_bootstrap.c src/ui/prism/framebuffer.c src/ui/canvas/scene.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/platform/dawn/include -Isrc/ui/prism/include -Isrc/ui/canvas/include \
		tests/kernel/pulse_canvas_bootstrap.c src/ui/prism/framebuffer.c src/ui/canvas/scene.c -o $@

$(PULSE_HORIZON_TEST): tests/kernel/pulse_horizon_bootstrap.c src/ui/prism/framebuffer.c src/ui/canvas/scene.c src/apps/horizon/shell.c src/apps/horizon/focus.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/platform/dawn/include -Isrc/ui/prism/include -Isrc/ui/canvas/include -Isrc/apps/horizon/include \
		tests/kernel/pulse_horizon_bootstrap.c src/ui/prism/framebuffer.c src/ui/canvas/scene.c src/apps/horizon/shell.c src/apps/horizon/focus.c -o $@

$(PULSE_HORIZON_FOCUS_TEST): tests/apps/horizon_focus.c src/apps/horizon/focus.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/platform/dawn/include -Isrc/ui/prism/include -Isrc/ui/canvas/include -Isrc/apps/horizon/include \
		tests/apps/horizon_focus.c src/apps/horizon/focus.c -o $@

$(PULSE_KEYBOARD_TEST): tests/kernel/pulse_keyboard_bootstrap.c src/drivers/atlas/i8042-keyboard.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/drivers/atlas/include \
		tests/kernel/pulse_keyboard_bootstrap.c src/drivers/atlas/i8042-keyboard.c -o $@

$(PULSE_PCI_TEST): tests/kernel/pulse_pci_bootstrap.c src/drivers/atlas/pci-inventory.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/drivers/atlas/include \
	tests/kernel/pulse_pci_bootstrap.c src/drivers/atlas/pci-inventory.c -o $@

$(DAWN_ACPI_TEST): tests/platform/dawn_acpi_rsdp.c src/platform/dawn/acpi-rsdp.c src/platform/dawn/include/dawn_acpi.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -no-pie -Wall -Wextra -Wpedantic -Werror -Isrc/platform/dawn/include \
		tests/platform/dawn_acpi_rsdp.c src/platform/dawn/acpi-rsdp.c -o $@

$(PRELUDE_OBJ): $(PRELUDE_DIR)/main.c $(PRELUDE_DIR)/include/uefi.h src/platform/dawn/include/dawn.h src/platform/dawn/include/dawn_acpi.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PRELUDE_CFLAGS) -c $< -o $@

$(PRELUDE_DAWN_ACPI_OBJ): src/platform/dawn/acpi-rsdp.c src/platform/dawn/include/dawn_acpi.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PRELUDE_CFLAGS) -c $< -o $@

$(PRELUDE_BLOB_OBJ): $(PRELUDE_DIR)/embedded-pulse.S $(PULSE_BIN)
	@mkdir -p $(dir $@)
	$(CLANG) --target=x86_64-pc-windows-msvc '-DPRELUDE_PULSE_BINARY="$(PULSE_BIN)"' -c $< -o $@

$(PRELUDE_EFI): $(PRELUDE_OBJ) $(PRELUDE_DAWN_ACPI_OBJ) $(PRELUDE_BLOB_OBJ)
	@mkdir -p $(dir $@)
	$(LLD_LINK) /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64 /out:$@ \
		$(PRELUDE_OBJ) $(PRELUDE_DAWN_ACPI_OBJ) $(PRELUDE_BLOB_OBJ)

$(ESP_IMAGE): $(PRELUDE_EFI)
	@mkdir -p $(dir $@)
	rm -f $@
	dd if=/dev/zero of=$@ bs=$(ESP_IMAGE_BYTES) count=1
	mformat -i $@ -F ::
	mmd -i $@ ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(PRELUDE_EFI) ::/EFI/BOOT/BOOTX64.EFI

$(BIOS_STAGE1_BIN): $(PRELUDE_BIOS_DIR)/stage1.asm
	@mkdir -p $(dir $@)
	$(NASM) -f bin -DSTAGE2_SECTORS=$(PRELUDE_BIOS_STAGE2_SECTORS) $< -o $@

$(BIOS_STAGE2_BIN): $(PRELUDE_BIOS_DIR)/stage2.asm $(PULSE_BIN) src/platform/dawn/include/dawn.h
	@mkdir -p $(dir $@)
	@pulse_bytes=$$(wc -c < $(PULSE_BIN)); \
	if (( pulse_bytes > $(PRELUDE_BIOS_PULSE_STAGING_BYTES) )); then \
		echo "Pulse raw image exceeds Legacy BIOS staging capacity: $$pulse_bytes > $(PRELUDE_BIOS_PULSE_STAGING_BYTES)" >&2; exit 1; \
	fi; \
	pulse_sectors=$$(( (pulse_bytes + 511) / 512 )); \
	$(NASM) -f bin -DSTAGE2_SECTORS=$(PRELUDE_BIOS_STAGE2_SECTORS) -DPULSE_BYTES=$$pulse_bytes -DPULSE_SECTORS=$$pulse_sectors $< -o $@

$(BIOS_IMAGE): $(BIOS_STAGE1_BIN) $(BIOS_STAGE2_BIN) $(PULSE_BIN)
	dd if=/dev/zero of=$@ bs=512 count=32768
	dd if=$(BIOS_STAGE1_BIN) of=$@ conv=notrunc
	dd if=$(BIOS_STAGE2_BIN) of=$@ bs=512 seek=1 conv=notrunc
	dd if=$(PULSE_BIN) of=$@ bs=512 seek=$$((1 + $(PRELUDE_BIOS_STAGE2_SECTORS))) conv=notrunc

check-uefi: $(ESP_IMAGE)
	tools/check-uefi.sh $(ESP_IMAGE) "ORIGIN: delegated key verified" "VAULT: redundant superblock recovered" "VAULT: journal commit verified" "VAULT: A/B slot state verified" "SESSION: launch policy verified" "PARCEL: signed manifest policy verified" "PRISM: framebuffer painted" "CANVAS: retained scene rendered" "HORIZON: desktop scene rendered" "HORIZON: desktop focus model verified" "ATLAS: keyboard event queue verified" "ATLAS: PCI inventory verified" "ATLAS: PCI resource inventory verified" "DAWN: ACPI RSDP handoff verified" "DAWN: ACPI root table metadata verified" "PULSE: task context verified" "PULSE: timer interrupt handled"

check-keyboard:
	$(MAKE) BUILD_DIR=build-keyboard PULSE_PROBE=keyboard uefi-image
	tools/check-keyboard.sh build-keyboard/vibeos-uefi-esp.img "ATLAS: keyboard irq probe ready" "ATLAS: keyboard IRQ1 event handled"

check-bios: $(BIOS_IMAGE)
	tools/check-bios.sh $(BIOS_IMAGE) "PRELUDE BIOS: Dawn Context sealed" "HORIZON: desktop focus model verified" "ATLAS: PCI resource inventory verified" "DAWN: ACPI RSDP handoff verified" "DAWN: ACPI root table metadata verified" "PULSE: timer interrupt handled"

check-panic: $(ESP_IMAGE)
	tools/check-uefi.sh $(ESP_IMAGE) "PULSE PANIC: unhandled interrupt"

test: check-uefi $(PULSE_MEMORY_TEST) $(PULSE_PAGING_TEST) $(PULSE_INTERRUPTS_TEST) $(PULSE_SCHEDULER_TEST) $(PULSE_CONTEXT_TEST) $(PULSE_TIMER_TEST) $(PULSE_RELAY_TEST) $(PULSE_VAULT_TEST) $(PULSE_SESSION_TEST) $(PULSE_PARCEL_TEST) $(PULSE_CANVAS_TEST) $(PULSE_HORIZON_TEST) $(PULSE_HORIZON_FOCUS_TEST) $(PULSE_KEYBOARD_TEST) $(PULSE_PCI_TEST) $(DAWN_ACPI_TEST)
	$(PULSE_MEMORY_TEST)
	$(PULSE_PAGING_TEST)
	$(PULSE_INTERRUPTS_TEST)
	$(PULSE_SCHEDULER_TEST)
	$(PULSE_CONTEXT_TEST)
	$(PULSE_TIMER_TEST)
	$(PULSE_RELAY_TEST)
	$(PULSE_VAULT_TEST)
	$(PULSE_SESSION_TEST)
	$(PULSE_PARCEL_TEST)
	$(PULSE_CANVAS_TEST)
	$(PULSE_HORIZON_TEST)
	$(PULSE_HORIZON_FOCUS_TEST)
	$(PULSE_KEYBOARD_TEST)
	$(PULSE_PCI_TEST)
	$(DAWN_ACPI_TEST)
	$(MAKE) check-keyboard
	$(MAKE) check-bios
	$(MAKE) BUILD_DIR=build-panic PULSE_PROBE=panic check-panic
	tests/boot/check-prelude-artifact.sh $(ESP_IMAGE) $(PRELUDE_EFI) $(PULSE_ELF)

test-panic:
	$(MAKE) BUILD_DIR=build-panic PULSE_PROBE=panic check-panic

test-bios: check-bios

clean:
	rm -rf $(BUILD_DIR)
