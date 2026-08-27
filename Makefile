# VibeOS Project Foundation — x86_64 UEFI Prelude build.
# This file is intentionally limited to the first reproducible bring-up path.

SHELL := /bin/bash

BUILD_DIR := build
PRELUDE_DIR := src/boot/prelude
PRELUDE_OBJ := $(BUILD_DIR)/prelude/main.obj
PRELUDE_EFI := $(BUILD_DIR)/prelude/BOOTX64.EFI
ESP_IMAGE := $(BUILD_DIR)/vibeos-uefi-esp.img

CLANG ?= clang
LLD_LINK ?= lld-link

CFLAGS := --target=x86_64-pc-windows-msvc -std=c17 -ffreestanding -fshort-wchar \
	-fno-stack-protector -mno-red-zone -Wall -Wextra -Wpedantic -Werror \
	-DPRELUDE_QEMU_DEBUG -I$(PRELUDE_DIR)/include

.PHONY: all prelude uefi-image check-uefi clean

all: uefi-image

prelude: $(PRELUDE_EFI)

uefi-image: $(ESP_IMAGE)

$(PRELUDE_OBJ): $(PRELUDE_DIR)/main.c $(PRELUDE_DIR)/include/uefi.h
	@mkdir -p $(dir $@)
	$(CLANG) $(CFLAGS) -c $< -o $@

$(PRELUDE_EFI): $(PRELUDE_OBJ)
	@mkdir -p $(dir $@)
	$(LLD_LINK) /subsystem:efi_application /entry:efi_main /nodefaultlib /machine:x64 /out:$@ $<

$(ESP_IMAGE): $(PRELUDE_EFI)
	@mkdir -p $(dir $@)
	rm -f $@
	truncate -s 64M $@
	mformat -i $@ -F ::
	mmd -i $@ ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(PRELUDE_EFI) ::/EFI/BOOT/BOOTX64.EFI

check-uefi: $(ESP_IMAGE)
	tools/check-uefi.sh $(ESP_IMAGE)

clean:
	rm -rf $(BUILD_DIR)
