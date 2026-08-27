/* VibeOS Pulse — initial physical layout loaded by Prelude at 2 MiB. */
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_pulse_entry)

SECTIONS
{
    . = 0x00200000;

    .text : ALIGN(4096)
    {
        *(.text.entry)
        *(.text .text.*)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4096)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}
