; VibeOS Prelude — self-owned Legacy BIOS first stage for the QEMU PC profile.
[bits 16]
[org 0x7c00]

    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    mov [boot_drive], dl

    mov ax, 0x0800
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, STAGE2_SECTORS
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc boot_failure
    jmp 0x0800:0x0000

boot_failure:
    cli
.halt:
    hlt
    jmp .halt

boot_drive: db 0

times 510-($-$$) db 0
dw 0xaa55
