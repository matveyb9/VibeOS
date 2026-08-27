#!/usr/bin/env bash
# VibeOS Project Foundation — prerequisite check for Linux, macOS and MSYS2.
set -euo pipefail

required_commands=(clang lld-link make dd mformat mmd mcopy)
missing=0

for command_name in "${required_commands[@]}"; do
    if command -v "$command_name" >/dev/null 2>&1; then
        printf 'found   %s\n' "$command_name"
    else
        printf 'missing %s\n' "$command_name" >&2
        missing=1
    fi
done

qemu_x86_64=${QEMU_X86_64:-qemu-system-x86_64}
if command -v "$qemu_x86_64" >/dev/null 2>&1; then
    printf 'found   %s\n' "$qemu_x86_64"
else
    printf 'missing %s\n' "$qemu_x86_64" >&2
    missing=1
fi

if command -v timeout >/dev/null 2>&1 || command -v gtimeout >/dev/null 2>&1; then
    printf 'found   timeout utility\n'
else
    printf 'missing timeout or gtimeout\n' >&2
    missing=1
fi

if [[ -n ${OVMF_CODE:-} && -n ${OVMF_VARS:-} ]]; then
    if [[ -f $OVMF_CODE && -f $OVMF_VARS ]]; then
        printf 'found   OVMF from OVMF_CODE and OVMF_VARS\n'
    else
        printf 'missing OVMF file referenced by OVMF_CODE or OVMF_VARS\n' >&2
        missing=1
    fi
elif [[ -f /usr/share/OVMF/OVMF_CODE_4M.fd && -f /usr/share/OVMF/OVMF_VARS_4M.fd ]]; then
    printf 'found   system OVMF defaults\n'
else
    printf 'missing OVMF_CODE and OVMF_VARS; see docs/*/guides/HOST_ENVIRONMENTS.md\n' >&2
    missing=1
fi

if [[ $missing -ne 0 ]]; then
    exit 1
fi

echo 'VibeOS host prerequisites are available.'
