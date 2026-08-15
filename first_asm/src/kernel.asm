[BITS 32]
SEG_CODE equ 0x08
SEG_DATA equ 0x10

global kernel_entry
extern kernel_main


kernel_entry:
    mov ax, SEG_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
    ; Enable the A20 line
    in al, 0x92
    or al, 2
    out 0x92, al
    call kernel_main
    jmp $

times 512-($ - $$) db 0
