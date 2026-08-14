org 0x7c00

mov ax, 0x0300
mov ds, ax
mov [0xff], 0x30

;Boot signature
times 510-($-$$) db 0
dw 0xaa55