/* linker.ld */
/**
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
**/

ENTRY(kernel_entry)
OUTPUT_FORMAT(binary)

SECTIONS
{
    . = 1M;

    .text : ALIGN(4096)
    {
        *(.text)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(COMMON)
        *(.bss)
    }

    .asm : ALIGN(4096)
    {
        *(.asm)
    }
}