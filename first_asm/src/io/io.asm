section .asm

global insb
global insw
global outb
global outw

insb:
push ebp
mov ebp, esp
xor eax, eax ; clear eax
mov edx, [ebp + 8] ; get the first argument which is the port
in al, dx
pop ebp
ret

insw:
push ebp
mov ebp, esp
xor eax, eax ; clear eax
mov edx, [ebp + 8] ; get the first argument which is the port
in ax, dx
pop ebp
ret

outb:
push ebp
mov ebp, esp
mov edx, [ebp+8] ; port
mov eax, [ebp+12] ; data
out dx, al
pop ebp
ret

outw:
push ebp
mov ebp, esp
mov edx, [ebp+8] ; port
mov eax, [ebp+12] ; data
out dx, ax
pop ebp
ret
