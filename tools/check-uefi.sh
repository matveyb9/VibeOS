#!/usr/bin/env bash
# VibeOS Project Foundation — portable automated QEMU verification for Prelude UEFI.
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "usage: $0 <esp-image>" >&2
    exit 64
fi

esp_image=$1
build_dir=$(dirname "$esp_image")
qemu_x86_64=${QEMU_X86_64:-qemu-system-x86_64}
ovmf_code=${OVMF_CODE:-/usr/share/OVMF/OVMF_CODE_4M.fd}
ovmf_vars_source=${OVMF_VARS:-/usr/share/OVMF/OVMF_VARS_4M.fd}
ovmf_vars="$build_dir/OVMF_VARS_4M.fd"
debug_log="$build_dir/prelude-qemu.log"

if command -v timeout >/dev/null 2>&1; then
    timeout_command=(timeout 20s)
elif command -v gtimeout >/dev/null 2>&1; then
    timeout_command=(gtimeout 20s)
else
    echo "A timeout utility is required: install GNU coreutils or provide timeout/gtimeout." >&2
    exit 69
fi

if ! command -v "$qemu_x86_64" >/dev/null 2>&1; then
    echo "missing QEMU executable: $qemu_x86_64" >&2
    exit 69
fi

for required_file in "$esp_image" "$ovmf_code" "$ovmf_vars_source"; do
    if [[ ! -f "$required_file" ]]; then
        echo "missing required file: $required_file" >&2
        exit 66
    fi
done

cp "$ovmf_vars_source" "$ovmf_vars"
rm -f "$debug_log"

set +e
"${timeout_command[@]}" "$qemu_x86_64" \
    -machine q35 -m 256M -no-reboot -display none \
    -drive if=pflash,format=raw,unit=0,readonly=on,file="$ovmf_code" \
    -drive if=pflash,format=raw,unit=1,file="$ovmf_vars" \
    -drive if=virtio,format=raw,file="$esp_image" \
    -debugcon "file:$debug_log" -global isa-debugcon.iobase=0x402 \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
qemu_status=$?
set -e

if ! grep -Fqx "PULSE: Dawn Context v1 accepted" "$debug_log"; then
    echo "Prelude UEFI verification marker was not observed." >&2
    [[ -f "$debug_log" ]] && cat "$debug_log" >&2
    exit 1
fi

# isa-debug-exit encodes guest value 0 as QEMU exit status 1.
if [[ $qemu_status -ne 1 ]]; then
    echo "Unexpected QEMU status: $qemu_status" >&2
    exit 1
fi

echo "Prelude UEFI verification passed."
