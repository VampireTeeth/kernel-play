[BITS 32]
SEG_CODE equ 0x08
SEG_DATA equ 0x10

global kernel_entry
global int21h
global no_interrupt
global enable_interrupt
global disable_interrupt

extern kernel_main
extern int21h_handler
extern no_interrupt_handler

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
    call remap_pic
    call kernel_main
    jmp $

; Remapping the Programmable Interrupt Controller
; https://wiki.osdev.org/8259_PIC
remap_pic:
; We only re-map the master PIC
    push eax
    mov al, 0x11
    out 0x20, al ; signal the master port with init command: 0x11
    mov al, 0x20
    out 0x21, al ; tells master PIC vector offset starts at 0x20
    mov al, 0x1
    out 0x21, al ; end of remapping master PIC
    pop eax
    ret

int21h:
    cli
    pushad ; push all the general regs
    call int21h_handler
    popad ; pop all the general regs
    sti
    iret

no_interrupt:
    cli
    pushad ; push all the general regs
    call no_interrupt_handler
    popad ; pop all the general regs
    sti
    iret

enable_interrupt:
    sti
    ret

disable_interrupt:
    cli
    ret

times 512-($ - $$) db 0
