; VibeOS Prelude — Legacy BIOS stage two for the reproducible QEMU/SeaBIOS path.
; It owns E820, VBE 2.0 LFB selection, long-mode transition, and Dawn v3 handoff.
[bits 16]
[org 0x8000]

%define PULSE_TEMP_ADDRESS 0x10000
%define PULSE_LOAD_ADDRESS 0x200000
%define DAWN_CONTEXT_ADDRESS 0x7000
%define DAWN_MEMORY_MAP_ADDRESS 0x5000
%define VBE_MODE_INFO_ADDRESS 0x6000
%define DAWN_CONTEXT_MAGIC_LOW 0x43545831
%define DAWN_CONTEXT_MAGIC_HIGH 0x4441574e
%define DAWN_CONTEXT_VERSION 3
%define DAWN_CONTEXT_SIZE 104
%define DAWN_MEMORY_DESCRIPTOR_SIZE 24
%define DAWN_MEMORY_DESCRIPTOR_VERSION 1
%define DAWN_MEMORY_USABLE 1
%define DAWN_PIXEL_FORMAT_BGR888 3
%define DAWN_MEMORY_MAP_MAX_ENTRIES 32

    cli
    cld
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7a00
    mov [boot_drive], dl

    call enable_a20
    call load_pulse
    jc prelude_failure
    call capture_e820
    jc prelude_failure
    call capture_vbe_framebuffer
    jc prelude_failure
    call seal_dawn_context
    call debug_dawn_sealed
    call enter_long_mode

prelude_failure:
    mov si, failure_message
    call debug_write
    cli
.halt:
    hlt
    jmp .halt

enable_a20:
    in al, 0x92
    or al, 0x02
    out 0x92, al
    ret

load_pulse:
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, disk_address_packet
    int 0x13
    ret

capture_e820:
    xor ax, ax
    mov es, ax
    mov di, DAWN_MEMORY_MAP_ADDRESS
    mov word [memory_entry_count], 0
    xor ebx, ebx
.next:
    cmp word [memory_entry_count], DAWN_MEMORY_MAP_MAX_ENTRIES
    jae .failure
    mov dword [es:di + 20], 1
    mov eax, 0xe820
    mov edx, 0x534d4150
    mov ecx, 24
    int 0x15
    jc .done
    cmp eax, 0x534d4150
    jne .failure
    cmp ecx, 20
    jb .failure
    cmp dword [es:di + 16], 1
    jne .reserved
    cmp dword [es:di + 4], 0
    jne .usable
    cmp dword [es:di], 0x00100000
    jb .reserved
.usable:
    mov dword [es:di + 16], DAWN_MEMORY_USABLE
    jmp .normalized
.reserved:
    mov dword [es:di + 16], 0
.normalized:
    mov dword [es:di + 20], 0
    add di, DAWN_MEMORY_DESCRIPTOR_SIZE
    inc word [memory_entry_count]
    test ebx, ebx
    jnz .next
.done:
    cmp word [memory_entry_count], 0
    je .failure
    clc
    ret
.failure:
    stc
    ret

capture_vbe_framebuffer:
    mov ax, 0x0600
    mov es, ax
    xor di, di
    mov ax, 0x4f01
    mov cx, 0x0118
    int 0x10
    cmp ax, 0x004f
    jne .info_failure
    mov ax, 0x0600
    mov es, ax
    test word [es:0], 0x0080
    jz .attributes_failure
    cmp byte [es:25], 24
    jne .format_failure
    cmp dword [es:40], 0
    je .framebuffer_failure

    mov ax, 0x4f02
    mov bx, 0x4118
    int 0x10
    cmp ax, 0x004f
    jne .set_failure

    xor ax, ax
    mov ds, ax
    mov eax, [es:40]
    mov [DAWN_CONTEXT_ADDRESS + 72], eax
    mov dword [DAWN_CONTEXT_ADDRESS + 76], 0
    movzx eax, word [es:16]
    movzx ecx, word [es:20]
    mul ecx
    mov [DAWN_CONTEXT_ADDRESS + 80], eax
    mov [DAWN_CONTEXT_ADDRESS + 84], edx
    movzx eax, word [es:18]
    mov [DAWN_CONTEXT_ADDRESS + 88], eax
    movzx eax, word [es:20]
    mov [DAWN_CONTEXT_ADDRESS + 92], eax
    movzx eax, word [es:16]
    mov ebx, 3
    xor edx, edx
    div ebx
    mov [DAWN_CONTEXT_ADDRESS + 96], eax
    mov dword [DAWN_CONTEXT_ADDRESS + 100], DAWN_PIXEL_FORMAT_BGR888
    xor ax, ax
    mov es, ax
    clc
    ret
.info_failure:
    mov si, vbe_info_failure_message
    jmp .debug_failure
.attributes_failure:
    mov si, vbe_attributes_failure_message
    jmp .debug_failure
.format_failure:
    mov si, vbe_format_failure_message
    jmp .debug_failure
.framebuffer_failure:
    mov si, vbe_framebuffer_failure_message
    jmp .debug_failure
.set_failure:
    mov si, vbe_set_failure_message
.debug_failure:
    xor ax, ax
    mov ds, ax
    call debug_write
.failure:
    xor ax, ax
    mov ds, ax
    stc
    ret

seal_dawn_context:
    mov dword [DAWN_CONTEXT_ADDRESS], DAWN_CONTEXT_MAGIC_LOW
    mov dword [DAWN_CONTEXT_ADDRESS + 4], DAWN_CONTEXT_MAGIC_HIGH
    mov dword [DAWN_CONTEXT_ADDRESS + 8], DAWN_CONTEXT_VERSION
    mov dword [DAWN_CONTEXT_ADDRESS + 12], DAWN_CONTEXT_SIZE
    mov dword [DAWN_CONTEXT_ADDRESS + 16], DAWN_MEMORY_MAP_ADDRESS
    mov dword [DAWN_CONTEXT_ADDRESS + 20], 0
    movzx eax, word [memory_entry_count]
    imul eax, DAWN_MEMORY_DESCRIPTOR_SIZE
    mov [DAWN_CONTEXT_ADDRESS + 24], eax
    mov dword [DAWN_CONTEXT_ADDRESS + 28], 0
    mov dword [DAWN_CONTEXT_ADDRESS + 32], 0
    mov dword [DAWN_CONTEXT_ADDRESS + 36], 0
    mov dword [DAWN_CONTEXT_ADDRESS + 40], DAWN_MEMORY_DESCRIPTOR_SIZE
    mov dword [DAWN_CONTEXT_ADDRESS + 44], 0
    mov dword [DAWN_CONTEXT_ADDRESS + 48], DAWN_MEMORY_DESCRIPTOR_VERSION
    mov dword [DAWN_CONTEXT_ADDRESS + 52], 0
    mov dword [DAWN_CONTEXT_ADDRESS + 56], 0x00090000
    mov dword [DAWN_CONTEXT_ADDRESS + 60], 0
    mov dword [DAWN_CONTEXT_ADDRESS + 64], 0x00010000
    mov dword [DAWN_CONTEXT_ADDRESS + 68], 0
    ret

debug_dawn_sealed:
    mov si, dawn_sealed_message
    call debug_write
    ret

debug_write:
    mov dx, 0x0402
.next:
    lodsb
    test al, al
    jz .done
    out dx, al
    jmp .next
.done:
    ret

enter_long_mode:
    cli
    call build_page_tables
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax
    jmp 0x08:protected_mode

build_page_tables:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov di, 0x1000
    mov cx, 6144
    xor eax, eax
    rep stosb

    mov dword [0x1000], 0x2003
    mov dword [0x1004], 0
    mov dword [0x2000], 0x3003
    mov dword [0x2004], 0
    mov dword [0x2008], 0x4003
    mov dword [0x200c], 0
    mov dword [0x2010], 0xe003
    mov dword [0x2014], 0
    mov dword [0x2018], 0xf003
    mov dword [0x201c], 0

    mov di, 0x3000
    xor eax, eax
    call fill_page_directory
    mov di, 0x4000
    mov eax, 0x40000000
    call fill_page_directory
    mov di, 0xe000
    mov eax, 0x80000000
    call fill_page_directory
    mov di, 0xf000
    mov eax, 0xc0000000
    call fill_page_directory
    ret

fill_page_directory:
    or eax, 0x83
    mov cx, 512
.next_entry:
    mov [di], eax
    mov dword [di + 4], 0
    add eax, 0x200000
    add di, 8
    loop .next_entry
    ret

[bits 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x00090000
    mov eax, cr4
    or eax, 0x00000020
    mov cr4, eax
    mov eax, 0x00001000
    mov cr3, eax
    mov ecx, 0xc0000080
    rdmsr
    or eax, 0x00000100
    wrmsr
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    jmp 0x18:long_mode

[bits 64]
long_mode:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov rsp, 0x00090000
    cld
    mov rsi, PULSE_TEMP_ADDRESS
    mov rdi, PULSE_LOAD_ADDRESS
    mov rcx, PULSE_BYTES
    rep movsb
    mov rdi, DAWN_CONTEXT_ADDRESS
    mov rax, PULSE_LOAD_ADDRESS
    jmp rax

[bits 16]
boot_drive: db 0
memory_entry_count: dw 0
failure_message: db "PRELUDE BIOS: boot handoff failed", 10, 0
dawn_sealed_message: db "PRELUDE BIOS: Dawn Context sealed", 10, 0
vbe_info_failure_message: db "PRELUDE BIOS: VBE mode info failed", 10, 0
vbe_attributes_failure_message: db "PRELUDE BIOS: VBE LFB unavailable", 10, 0
vbe_format_failure_message: db "PRELUDE BIOS: VBE pixel format unsupported", 10, 0
vbe_framebuffer_failure_message: db "PRELUDE BIOS: VBE framebuffer unavailable", 10, 0
vbe_set_failure_message: db "PRELUDE BIOS: VBE mode set failed", 10, 0

disk_address_packet:
    db 16, 0
    dw PULSE_SECTORS
    dw 0x0000
    dw 0x1000
    dq (1 + STAGE2_SECTORS)

align 8
gdt_start:
    dq 0
    dq 0x00cf9a000000ffff
    dq 0x00cf92000000ffff
    dq 0x00af9a000000ffff
    dq 0x00cf92000000ffff
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times (STAGE2_SECTORS * 512) - ($ - $$) db 0
