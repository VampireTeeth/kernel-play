section .asm

global paging_load_directory
global paging_enable_paging

paging_load_directory:
push ebp
mov ebp, esp
mov eax, [ebp+8]
mov cr3, eax
pop ebp
ret

paging_enable_paging:
push ebp
mov ebp, esp
mov eax, cr0
or eax, 0x80000001 ; enable the paging bit, which is the 31st bit
mov cr0, eax
pop ebp
ret
