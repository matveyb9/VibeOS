# VibeOS Project Foundation — x86_64 UEFI Prelude and Pulse bring-up.
# The build deliberately targets a single reproducible QEMU profile first.

SHELL := /bin/bash

BUILD_DIR ?= build
PULSE_PROBE ?= timer
PRELUDE_DIR := src/boot/prelude
PULSE_DIR := src/kernel/pulse

PRELUDE_OBJ := $(BUILD_DIR)/prelude/main.obj
PRELUDE_BLOB_OBJ := $(BUILD_DIR)/prelude/embedded-pulse.obj
PRELUDE_EFI := $(BUILD_DIR)/prelude/BOOTX64.EFI
PULSE_ENTRY_OBJ := $(BUILD_DIR)/pulse/entry.obj
PULSE_INTERRUPTS_ENTRY_OBJ := $(BUILD_DIR)/pulse/interrupts/x86_64.obj
PULSE_CONTEXT_ENTRY_OBJ := $(BUILD_DIR)/pulse/schedule/context-x86_64.obj
PULSE_MAIN_OBJ := $(BUILD_DIR)/pulse/main.obj
PULSE_MEMORY_OBJ := $(BUILD_DIR)/pulse/memory/early.obj
PULSE_PAGING_OBJ := $(BUILD_DIR)/pulse/memory/paging-x86_64.obj
PULSE_INTERRUPTS_OBJ := $(BUILD_DIR)/pulse/interrupts/idt-x86_64.obj
PULSE_SCHEDULER_OBJ := $(BUILD_DIR)/pulse/schedule/round-robin.obj
PULSE_CONTEXT_OBJ := $(BUILD_DIR)/pulse/schedule/context.obj
PULSE_TIMER_OBJ := $(BUILD_DIR)/pulse/time/pit-x86_64.obj
PULSE_KEYS_OBJ := $(BUILD_DIR)/security/keys/key-space.obj
PULSE_RELAY_OBJ := $(BUILD_DIR)/ipc/relay/link.obj
PULSE_RELAY_CHANNEL_OBJ := $(BUILD_DIR)/ipc/relay/channel.obj
PULSE_ORIGIN_OBJ := $(BUILD_DIR)/runtime/origin/origin.obj
PULSE_ORIGIN_ABI_OBJ := $(BUILD_DIR)/runtime/origin/abi.obj
PULSE_ATLAS_RAM_OBJ := $(BUILD_DIR)/drivers/atlas/ram-block.obj
PULSE_VAULT_OBJ := $(BUILD_DIR)/storage/vault/vaultfs.obj
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
ESP_IMAGE := $(BUILD_DIR)/vibeos-uefi-esp.img
ESP_IMAGE_BYTES := 67108864

CLANG ?= clang
LLD_LINK ?= lld-link
LLD_LD ?= ld.lld
LLVM_OBJCOPY ?= llvm-objcopy
HOST_CC ?= cc

PRELUDE_CFLAGS := --target=x86_64-pc-windows-msvc -std=c17 -ffreestanding -fshort-wchar \
	-fno-stack-protector -mno-red-zone -Wall -Wextra -Wpedantic -Werror \
	-I$(PRELUDE_DIR)/include -Isrc/platform/dawn/include
PULSE_CFLAGS := --target=x86_64-unknown-elf -std=c17 -ffreestanding -fno-stack-protector \
	-fno-pic -fno-pie -mno-red-zone -Wall -Wextra -Wpedantic -Werror \
	-DPULSE_PROBE_$(PULSE_PROBE) -Isrc/platform/dawn/include -I$(PULSE_DIR)/include \
	-Isrc/security/keys/include -Isrc/ipc/relay/include -Isrc/runtime/origin/include \
	-Isrc/drivers/atlas/include -Isrc/storage/vault/include
PULSE_ASFLAGS := --target=x86_64-unknown-elf -ffreestanding -mno-red-zone

.PHONY: all prelude pulse uefi-image check-uefi check-panic test test-panic clean

all: uefi-image

prelude: $(PRELUDE_EFI)

pulse: $(PULSE_ELF)

uefi-image: $(ESP_IMAGE)

$(PULSE_ENTRY_OBJ): $(PULSE_DIR)/entry/x86_64.S
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_ASFLAGS) -c $< -o $@

$(PULSE_INTERRUPTS_ENTRY_OBJ): $(PULSE_DIR)/interrupts/x86_64.S
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_ASFLAGS) -c $< -o $@

$(PULSE_CONTEXT_ENTRY_OBJ): $(PULSE_DIR)/schedule/context-x86_64.S
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_ASFLAGS) -c $< -o $@

$(PULSE_MAIN_OBJ): $(PULSE_DIR)/main.c src/platform/dawn/include/dawn.h
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

$(PULSE_VAULT_OBJ): src/storage/vault/vaultfs.c src/storage/vault/include/vaultfs.h src/drivers/atlas/include/atlas_block.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_PANIC_OBJ): $(PULSE_DIR)/debug/panic.c $(PULSE_DIR)/include/panic.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PULSE_CFLAGS) -c $< -o $@

$(PULSE_ELF): $(PULSE_ENTRY_OBJ) $(PULSE_INTERRUPTS_ENTRY_OBJ) $(PULSE_CONTEXT_ENTRY_OBJ) $(PULSE_MAIN_OBJ) $(PULSE_MEMORY_OBJ) $(PULSE_PAGING_OBJ) $(PULSE_INTERRUPTS_OBJ) $(PULSE_SCHEDULER_OBJ) $(PULSE_CONTEXT_OBJ) $(PULSE_TIMER_OBJ) $(PULSE_KEYS_OBJ) $(PULSE_RELAY_OBJ) $(PULSE_RELAY_CHANNEL_OBJ) $(PULSE_ORIGIN_OBJ) $(PULSE_ORIGIN_ABI_OBJ) $(PULSE_ATLAS_RAM_OBJ) $(PULSE_VAULT_OBJ) $(PULSE_PANIC_OBJ) $(PULSE_DIR)/linker/x86_64.ld
	@mkdir -p $(dir $@)
	$(LLD_LD) -m elf_x86_64 -nostdlib --build-id=none -T $(PULSE_DIR)/linker/x86_64.ld \
		-o $@ $(PULSE_ENTRY_OBJ) $(PULSE_INTERRUPTS_ENTRY_OBJ) $(PULSE_CONTEXT_ENTRY_OBJ) $(PULSE_MAIN_OBJ) $(PULSE_MEMORY_OBJ) $(PULSE_PAGING_OBJ) $(PULSE_INTERRUPTS_OBJ) $(PULSE_SCHEDULER_OBJ) $(PULSE_CONTEXT_OBJ) $(PULSE_TIMER_OBJ) $(PULSE_KEYS_OBJ) $(PULSE_RELAY_OBJ) $(PULSE_RELAY_CHANNEL_OBJ) $(PULSE_ORIGIN_OBJ) $(PULSE_ORIGIN_ABI_OBJ) $(PULSE_ATLAS_RAM_OBJ) $(PULSE_VAULT_OBJ) $(PULSE_PANIC_OBJ)

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

$(PULSE_TIMER_TEST): tests/kernel/pulse_timer_bootstrap.c $(PULSE_DIR)/time/pit-x86_64.c $(PULSE_DIR)/include/timer.h
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -I$(PULSE_DIR)/include \
		tests/kernel/pulse_timer_bootstrap.c $(PULSE_DIR)/time/pit-x86_64.c -o $@

$(PULSE_RELAY_TEST): tests/kernel/pulse_relay_bootstrap.c src/security/keys/key-space.c src/ipc/relay/link.c src/ipc/relay/channel.c src/runtime/origin/origin.c src/runtime/origin/abi.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/security/keys/include -Isrc/ipc/relay/include -Isrc/runtime/origin/include \
		tests/kernel/pulse_relay_bootstrap.c src/security/keys/key-space.c src/ipc/relay/link.c src/ipc/relay/channel.c src/runtime/origin/origin.c src/runtime/origin/abi.c -o $@

$(PULSE_VAULT_TEST): tests/kernel/pulse_vault_bootstrap.c src/drivers/atlas/ram-block.c src/storage/vault/vaultfs.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c17 -Wall -Wextra -Wpedantic -Werror -Isrc/drivers/atlas/include -Isrc/storage/vault/include \
		tests/kernel/pulse_vault_bootstrap.c src/drivers/atlas/ram-block.c src/storage/vault/vaultfs.c -o $@

$(PRELUDE_OBJ): $(PRELUDE_DIR)/main.c $(PRELUDE_DIR)/include/uefi.h src/platform/dawn/include/dawn.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PRELUDE_CFLAGS) -c $< -o $@

$(PRELUDE_BLOB_OBJ): $(PRELUDE_DIR)/embedded-pulse.S $(PULSE_BIN)
	@mkdir -p $(dir $@)
	$(CLANG) --target=x86_64-pc-windows-msvc '-DPRELUDE_PULSE_BINARY="$(PULSE_BIN)"' -c $< -o $@

$(PRELUDE_EFI): $(PRELUDE_OBJ) $(PRELUDE_BLOB_OBJ)
	@mkdir -p $(dir $@)
	$(LLD_LINK) /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64 /out:$@ \
		$(PRELUDE_OBJ) $(PRELUDE_BLOB_OBJ)

$(ESP_IMAGE): $(PRELUDE_EFI)
	@mkdir -p $(dir $@)
	rm -f $@
	dd if=/dev/zero of=$@ bs=$(ESP_IMAGE_BYTES) count=1
	mformat -i $@ -F ::
	mmd -i $@ ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(PRELUDE_EFI) ::/EFI/BOOT/BOOTX64.EFI

check-uefi: $(ESP_IMAGE)
	tools/check-uefi.sh $(ESP_IMAGE) "ORIGIN: delegated key verified" "VAULT: redundant superblock recovered" "VAULT: journal commit verified" "PULSE: task context verified" "PULSE: timer interrupt handled"

check-panic: $(ESP_IMAGE)
	tools/check-uefi.sh $(ESP_IMAGE) "PULSE PANIC: unhandled interrupt"

test: check-uefi $(PULSE_MEMORY_TEST) $(PULSE_PAGING_TEST) $(PULSE_INTERRUPTS_TEST) $(PULSE_SCHEDULER_TEST) $(PULSE_CONTEXT_TEST) $(PULSE_TIMER_TEST) $(PULSE_RELAY_TEST) $(PULSE_VAULT_TEST)
	$(PULSE_MEMORY_TEST)
	$(PULSE_PAGING_TEST)
	$(PULSE_INTERRUPTS_TEST)
	$(PULSE_SCHEDULER_TEST)
	$(PULSE_CONTEXT_TEST)
	$(PULSE_TIMER_TEST)
	$(PULSE_RELAY_TEST)
	$(PULSE_VAULT_TEST)
	$(MAKE) BUILD_DIR=build-panic PULSE_PROBE=panic check-panic
	tests/boot/check-prelude-artifact.sh $(ESP_IMAGE) $(PRELUDE_EFI) $(PULSE_ELF)

test-panic:
	$(MAKE) BUILD_DIR=build-panic PULSE_PROBE=panic check-panic

clean:
	rm -rf $(BUILD_DIR)
