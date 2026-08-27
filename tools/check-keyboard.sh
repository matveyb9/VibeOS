#!/usr/bin/env bash
# VibeOS Project Foundation — automated QEMU input verification for Atlas i8042 IRQ1.
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
debug_log="$build_dir/prelude-qemu.log"
monitor_socket="$build_dir/qemu-monitor.sock"

for required_command in "$qemu_x86_64" socat timeout; do
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
rm -f "$debug_log" "$monitor_socket"
timeout 20s "$qemu_x86_64" \
    -machine q35 -m 256M -no-reboot -display none \
    -drive if=pflash,format=raw,unit=0,readonly=on,file="$ovmf_code" \
    -drive if=pflash,format=raw,unit=1,file="$ovmf_vars" \
    -drive if=virtio,format=raw,file="$esp_image" \
    -debugcon "file:$debug_log" -global isa-debugcon.iobase=0x402 \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
    -monitor "unix:$monitor_socket,server=on,wait=off" &
qemu_pid=$!

cleanup() {
    kill "$qemu_pid" 2>/dev/null || true
    wait "$qemu_pid" 2>/dev/null || true
    rm -f "$monitor_socket"
}
trap cleanup EXIT

for _ in $(seq 1 100); do
    if [[ -f "$debug_log" ]] && grep -Fqx "ATLAS: keyboard irq probe ready" "$debug_log"; then
        break
    fi
    sleep 0.05
done
if [[ ! -S "$monitor_socket" ]] || ! grep -Fqx "ATLAS: keyboard irq probe ready" "$debug_log"; then
    echo "Keyboard probe never reached input-ready state." >&2
    [[ -f "$debug_log" ]] && cat "$debug_log" >&2
    exit 1
fi
printf 'sendkey h\n' | socat - UNIX-CONNECT:"$monitor_socket" >/dev/null

set +e
wait "$qemu_pid"
qemu_status=$?
set -e
trap - EXIT
rm -f "$monitor_socket"

for expected_marker in "${expected_markers[@]}"; do
    if ! grep -Fqx "$expected_marker" "$debug_log"; then
        echo "Keyboard QEMU marker was not observed: $expected_marker" >&2
        cat "$debug_log" >&2
        exit 1
    fi
done
if [[ $qemu_status -ne 1 ]]; then
    echo "Unexpected keyboard QEMU status: $qemu_status" >&2
    exit 1
fi
echo "Atlas keyboard UEFI verification passed."
