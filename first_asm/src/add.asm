global add_function
section .text
add_function:
    mov rax, rdi      ; First argument (System V ABI)
    add rax, rsi      ; Add second argument
    ret
