;ORG 0x7c00
org 0
BITS 16

; jump short to avoid code being over-written by
; BIOS when making it a real bootable USB stick
jump_short:
    jmp short main
    nop
    times 33 db 0

main:
    jmp 0x7c0:real_main ; moves the cs register to point to 0x7c0:real_main

;handle_zero:
;	mov bx, 0
;	mov al, 'A'
;	call print_char
;	iret

;handle_one:
;	mov bx, 0
;	mov al, 'V'
;	call print_char
;	iret

real_main:
    cli ;clear int
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    sti ;start int
;    mov word[ss:0x00], handle_zero
;    mov word[ss:0x02], 0x7c0
;
;    mov word[ss:0x04], handle_one
;    mov word[ss:0x06], 0x7c0
    ; int 0x00 ; will trigger interrpt 0, which is handle_zero
    ; int 0x01 ; will trigger interrupt 1, which is handle_one
;    mov ax, 0x00
;    div ax ; division by zero will trigger interrupt 0x00 which is handle_zero

;	mov si, message
;	call print

;; Reading from a disk (int 13H)
;input:
;AH = 02H
;AL = number of sectors to read/write (must be nonzero)
;CH = cylinder number (0..79).
;CL = sector number (1..18).
;DH = head number (0..1).
;DL = drive number (0..3 , for the emulator it depends on quantity of FLOPPY_ files).
 ;; drive will be set when you boot from the same drive
;ES:BX points to data buffer.

;return:
;CF set on error.
;CF clear if successful.
;AH = status (0 - if successful).
;AL = number of sectors transferred.
; start reading second sector from disk
    mov ah, 02h
    mov al, 1h
    mov ch, 0h
    mov cl, 2h
    mov dh, 0h
    mov bx, buffer
    int 13h
    jc read_disk_error
    mov si, buffer
    call print
;; end reading from disk and printing
	jmp $

;; Error message when read disk fails
read_disk_error:
    mov si, error_message
    call print
    jmp $
;; End error message when read disk fails


print:	
	mov bx, 0
.loop:
	lodsb
	cmp al, 0
	je .done
	call print_char
	jmp .loop
.done:
	ret

print_char:
	mov ah, 0eh
	int 0x10
	ret

; message: db 'Hello world!', 0
error_message: db 'Failed to read sector from disk', 0

times 510-($ - $$) db 0
dw 0xAA55

buffer: