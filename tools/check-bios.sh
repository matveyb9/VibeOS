#!/usr/bin/env bash
# VibeOS Project Foundation — QEMU verification for the self-owned Legacy BIOS path.
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "usage: $0 <bios-image> <expected-marker> [expected-marker ...]" >&2
    exit 64
fi

bios_image=$1
shift
expected_markers=("$@")
build_dir=$(dirname "$bios_image")
qemu_x86_64=${QEMU_X86_64:-qemu-system-x86_64}
debug_log="$build_dir/prelude-bios-qemu.log"

if ! command -v "$qemu_x86_64" >/dev/null 2>&1; then
    echo "missing QEMU executable: $qemu_x86_64" >&2
    exit 69
fi
if [[ ! -f "$bios_image" ]]; then
    echo "missing Legacy BIOS image: $bios_image" >&2
    exit 66
fi

rm -f "$debug_log"
set +e
timeout 20s "$qemu_x86_64" \
    -machine q35 -m 256M -no-reboot -display none \
    -drive if=ide,format=raw,file="$bios_image" \
    -debugcon "file:$debug_log" -global isa-debugcon.iobase=0x402 \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04
qemu_status=$?
set -e

for expected_marker in "${expected_markers[@]}"; do
    if ! grep -Fqx "$expected_marker" "$debug_log"; then
        echo "Legacy BIOS verification marker was not observed: $expected_marker" >&2
        [[ -f "$debug_log" ]] && cat "$debug_log" >&2
        exit 1
    fi
done
if [[ $qemu_status -ne 1 ]]; then
    echo "Unexpected Legacy BIOS QEMU status: $qemu_status" >&2
    exit 1
fi
echo "Prelude Legacy BIOS verification passed."
