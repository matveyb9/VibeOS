/*
 * VibeOS Prelude — first native x86_64 UEFI entry point.
 *
 * Design note: Prelude owns early boot validation and the Dawn Context handoff.
 * This first milestone deliberately proves only the independent UEFI path,
 * firmware console access, and QEMU diagnostic channel before Pulse exists.
 */

#include "uefi.h"

static CHAR16 prelude_banner[] = {
    'V', 'i', 'b', 'e', 'O', 'S', ' ', 'P', 'r', 'e', 'l', 'u', 'd', 'e',
    ' ', '0', '.', '0', '.', '1', '\r', '\n',
    'U', 'E', 'F', 'I', ' ', 'h', 'a', 'n', 'd', 'o', 'f', 'f',
    ' ', 'p', 'r', 'o', 'b', 'e', ' ', 'c', 'o', 'm', 'p', 'l', 'e', 't', 'e',
    '.', '\r', '\n', 0
};

#if defined(PRELUDE_QEMU_DEBUG)
static void prelude_debug_putc(char character) {
    __asm__ volatile("outb %0, %w1" : : "a"(character), "d"((uint16_t)0x402));
}

static void prelude_debug_write(const char *message) {
    while (*message != '\0') {
        prelude_debug_putc(*message);
        ++message;
    }
}

__attribute__((noreturn)) static void prelude_debug_exit(void) {
    __asm__ volatile("outl %0, %w1" : : "a"(0), "d"((uint16_t)0x0f4));
    for (;;) {
        __asm__ volatile("hlt");
    }
}
#endif

EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
    (void)image_handle;

    if (system_table != (void *)0 && system_table->con_out != (void *)0 &&
        system_table->con_out->output_string != (void *)0) {
        system_table->con_out->output_string(system_table->con_out, prelude_banner);
    }

#if defined(PRELUDE_QEMU_DEBUG)
    prelude_debug_write("PRELUDE: UEFI handoff verified\n");
    prelude_debug_exit();
#endif

    return EFI_SUCCESS;
}
