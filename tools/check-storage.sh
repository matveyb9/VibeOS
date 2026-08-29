#!/usr/bin/env bash
# VibeOS Atlas — automated QEMU ATA IDENTIFY verification on the legacy PC IDE path.
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "usage: $0 <esp-image> <expected-marker> [expected-marker ...]" >&2
    exit 64
fi

esp_image=$1
shift
expected_markers=("$@")
build_dir=$(dirname "$esp_image")
qemu_x86_64=${QEMU_X86_64:-qemu-system-x86_64}
ovmf_code=${OVMF_CODE:-/usr/share/OVMF/OVMF_CODE_4M.fd}
ovmf_vars_source=${OVMF_VARS:-/usr/share/OVMF/OVMF_VARS_4M.fd}
ovmf_vars="$build_dir/OVMF_VARS_4M.fd"
debug_log="$build_dir/storage-qemu.log"
ata_image="$build_dir/atlas-ata-identify.img"

for required_command in "$qemu_x86_64" timeout; do
    if ! command -v "$required_command" >/dev/null 2>&1; then
        echo "missing required command: $required_command" >&2
        exit 69
    fi
done
for required_file in "$esp_image" "$ovmf_code" "$ovmf_vars_source"; do
    if [[ ! -f "$required_file" ]]; then
        echo "missing required file: $required_file" >&2
        exit 66
    fi
done

cp "$ovmf_vars_source" "$ovmf_vars"
dd if=/dev/zero of="$ata_image" bs=1M count=1 status=none
rm -f "$debug_log"
set +e
timeout 20s "$qemu_x86_64" \
    -machine pc -m 256M -no-reboot -display none \
    -drive if=pflash,format=raw,unit=0,readonly=on,file="$ovmf_code" \
    -drive if=pflash,format=raw,unit=1,file="$ovmf_vars" \
    -drive if=virtio,format=raw,file="$esp_image" \
    -drive if=ide,index=0,media=disk,format=raw,file="$ata_image" \
    -debugcon "file:$debug_log" -global isa-debugcon.iobase=0x402 \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
qemu_status=$?
set -e
rm -f "$ata_image"

for expected_marker in "${expected_markers[@]}"; do
    if ! grep -Fqx "$expected_marker" "$debug_log"; then
        echo "Storage QEMU marker was not observed: $expected_marker" >&2
        [[ -f "$debug_log" ]] && cat "$debug_log" >&2
        exit 1
    fi
done
if [[ $qemu_status -ne 1 ]]; then
    echo "Unexpected storage QEMU status: $qemu_status" >&2
    exit 1
fi
echo "Atlas ATA UEFI verification passed."
