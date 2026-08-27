#!/usr/bin/env bash
# VibeOS Project Foundation — smoke checks for generated Prelude artifacts.
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 <esp-image> <efi-application>" >&2
    exit 64
fi

esp_image=$1
efi_application=$2

for required_file in "$esp_image" "$efi_application"; do
    if [[ ! -f "$required_file" ]]; then
        echo "missing artifact: $required_file" >&2
        exit 66
    fi
done

for required_command in llvm-readobj mdir; do
    if ! command -v "$required_command" >/dev/null 2>&1; then
        echo "missing test command: $required_command" >&2
        exit 69
    fi
done

file_headers=$(llvm-readobj --file-headers "$efi_application")
grep -Fq 'Format: COFF-x86-64' <<<"$file_headers"
grep -Fq 'Machine: IMAGE_FILE_MACHINE_AMD64' <<<"$file_headers"
grep -Fq 'Subsystem: IMAGE_SUBSYSTEM_EFI_APPLICATION' <<<"$file_headers"

esp_listing=$(mdir -i "$esp_image" ::/EFI/BOOT)
grep -Eq 'BOOTX64[[:space:]]+EFI' <<<"$esp_listing"

echo 'Prelude artifact smoke tests passed.'
