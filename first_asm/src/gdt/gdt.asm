section .asm
global gdt_load

gdt_load:
    mov eax, [esp+4] ; take the first param
    mov [gdt_descriptor+2], eax
    mov ax, [esp+8] ; take the second param
    mov [gdt_descriptor], ax
    lgdt [gdt_descriptor]
    ret

section .data
gdt_descriptor:
    dw 0x00 ; Size
    dd 0x00 ; GDT start address