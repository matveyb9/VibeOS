# VibeOS Project Foundation — x86_64 UEFI Prelude and Pulse bring-up.
# The build deliberately targets a single reproducible QEMU profile first.

SHELL := /bin/bash

BUILD_DIR := build
PRELUDE_DIR := src/boot/prelude
PULSE_DIR := src/kernel/pulse

PRELUDE_OBJ := $(BUILD_DIR)/prelude/main.obj
PRELUDE_BLOB_OBJ := $(BUILD_DIR)/prelude/embedded-pulse.obj
PRELUDE_EFI := $(BUILD_DIR)/prelude/BOOTX64.EFI
PULSE_ENTRY_OBJ := $(BUILD_DIR)/pulse/entry.obj
PULSE_INTERRUPTS_ENTRY_OBJ := $(BUILD_DIR)/pulse/interrupts/x86_64.obj
PULSE_MAIN_OBJ := $(BUILD_DIR)/pulse/main.obj
PULSE_MEMORY_OBJ := $(BUILD_DIR)/pulse/memory/early.obj
PULSE_PAGING_OBJ := $(BUILD_DIR)/pulse/memory/paging-x86_64.obj
PULSE_INTERRUPTS_OBJ := $(BUILD_DIR)/pulse/interrupts/idt-x86_64.obj
PULSE_ELF := $(BUILD_DIR)/pulse/PULSE.ELF
PULSE_BIN := $(BUILD_DIR)/pulse/pulse.bin
PULSE_MEMORY_TEST := $(BUILD_DIR)/tests/pulse-memory-bootstrap
PULSE_PAGING_TEST := $(BUILD_DIR)/tests/pulse-paging-bootstrap
PULSE_INTERRUPTS_TEST := $(BUILD_DIR)/tests/pulse-interrupts-bootstrap
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
	-Isrc/platform/dawn/include -I$(PULSE_DIR)/include
PULSE_ASFLAGS := --target=x86_64-unknown-elf -ffreestanding -mno-red-zone

.PHONY: all prelude pulse uefi-image check-uefi test clean

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

$(PULSE_ELF): $(PULSE_ENTRY_OBJ) $(PULSE_INTERRUPTS_ENTRY_OBJ) $(PULSE_MAIN_OBJ) $(PULSE_MEMORY_OBJ) $(PULSE_PAGING_OBJ) $(PULSE_INTERRUPTS_OBJ) $(PULSE_DIR)/linker/x86_64.ld
	@mkdir -p $(dir $@)
	$(LLD_LD) -m elf_x86_64 -nostdlib --build-id=none -T $(PULSE_DIR)/linker/x86_64.ld \
		-o $@ $(PULSE_ENTRY_OBJ) $(PULSE_INTERRUPTS_ENTRY_OBJ) $(PULSE_MAIN_OBJ) $(PULSE_MEMORY_OBJ) $(PULSE_PAGING_OBJ) $(PULSE_INTERRUPTS_OBJ)

$(PULSE_BIN): $(PULSE_ELF)
	$(LLVM_OBJCOPY) -O binary $< $@

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

$(PRELUDE_OBJ): $(PRELUDE_DIR)/main.c $(PRELUDE_DIR)/include/uefi.h src/platform/dawn/include/dawn.h
	@mkdir -p $(dir $@)
	$(CLANG) $(PRELUDE_CFLAGS) -c $< -o $@

$(PRELUDE_BLOB_OBJ): $(PRELUDE_DIR)/embedded-pulse.S $(PULSE_BIN)
	@mkdir -p $(dir $@)
	$(CLANG) --target=x86_64-pc-windows-msvc -c $< -o $@

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
	tools/check-uefi.sh $(ESP_IMAGE)

test: check-uefi $(PULSE_MEMORY_TEST) $(PULSE_PAGING_TEST) $(PULSE_INTERRUPTS_TEST)
	$(PULSE_MEMORY_TEST)
	$(PULSE_PAGING_TEST)
	$(PULSE_INTERRUPTS_TEST)
	tests/boot/check-prelude-artifact.sh $(ESP_IMAGE) $(PRELUDE_EFI) $(PULSE_ELF)

clean:
	rm -rf $(BUILD_DIR)
