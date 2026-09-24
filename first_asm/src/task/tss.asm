section .asm
global tss_load

tss_load:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    ltr ax
    pop ebp
    ret