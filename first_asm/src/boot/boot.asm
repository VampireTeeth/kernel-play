org 0x7c00
BITS 16

SEG_CODE equ gdt_code - gdt_start
SEG_DATA equ gdt_data - gdt_start
; jump short to avoid code being over-written by
; BIOS when making it a real bootable USB stick
jump_short:
    jmp short main ;; 2-byte jmp short instruction
    nop ;; NOP 1-byte nop directive

    ; FAT16 Header
    OEMIdentifier           db 'VAMPOS  ' ; 8 bytes
    BytesPerSector          dw 0x200 ;512 bytes per sector
    SectorsPerCluster       db 0x80 ;128 sectors per cluster
    ReservedSectors         dw 200
    FATCopies               db 0x02
    RootDirEntries          dw 0x40 ;64 entries in the root directory
    NumSectors              dw 0x00
    MediaType               db 0xF8
    SectorsPerFat           dw 0x100
    SectorsPerTrack         dw 0x20
    NumberOfHeads           dw 0x40
    HiddenSectors           dd 0x00
    SectorsBig              dd 0x773594

    ; Extended BPB (Dos 4.0)
    DriveNumber             db 0x80
    WinNTBit                db 0x00
    Signature               db 0x29
    VolumeID                dd 0xD105
    VolumeIDString          db 'VAMPOS BOOT' ; 11-bytes
    SystemIDString          db 'FAT16   '

main:
    jmp 0x00:real_main ; moves the cs register to point to real_main

real_main:
    cli ;clear int
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ;start int

load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp SEG_CODE:load32

;; GDT Global Descriptor Table
;; https://wiki.osdev.org/Global_Descriptor_Table
gdt_start:
gdt_null:
    dd 0x00 ;; 4-bytes 0s
    dd 0x00 ;; 4-bytes 0s

gdt_code: ;; CS should point to here
    dw 0xff ;; limit 0-15 bits
    dw 0x00 ;; base 16-31 bits
    db 0x00 ;; base 32-39 bits
    db 10011010b ;; Access byte
    db 11001111b ;; flags (high half byte) and limit (low half byte)
    db 0x00 ;; Base

gdt_data: ;; DS, ES, FS, GS should point to here
    dw 0xff ;; limit 0-15 bits
    dw 0x00 ;; base 16-31 bits
    db 0x00 ;; base 32-39 bits
    db 10010010b ;; Access byte
    db 11001111b ;; flags (high half byte) and limit (low half byte)
    db 0x00 ;; Base

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ;; size: 2-bytes
    dd gdt_start ;; offset: 4-bytes

[BITS 32]
load32:
    mov eax, 1
    mov ecx, 100
    mov edi, 0x00100000
    call ata_lba_read

    ;call read_from_disk
    jmp SEG_CODE:0x00100000

ata_lba_read:
    ;; Send the higher 8 bits of LBA to disk controller
    mov ebx, eax ;; Backup eax to ebx
    shr eax, 24 ;; Shift eax by 24 bits, resulting eax containing the highest 8 bits
    or eax, 0xE0 ;; Select the master's drive
    mov dx, 0x1F6
    out dx, al ;; send the lower 8 bits of eax to port 0x1F6
    ;; Finished

    ;; Send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ;; Finished

    ;; Send the lower 8 bits of LBA to disk controller
    mov eax, ebx
    mov dx, 0x1F3
    out dx, al
    ;; Finished

    ;; Send the second lower 8 bits of LBA to disk controller
    mov eax, ebx
    shr eax, 8
    mov dx, 0x1F4
    out dx, al
    ;; Finished

    ;; Send the high 16 bits of LBA to disk controller
    mov eax, ebx
    shr eax, 16
    mov dx, 0x1F5
    out dx, al
    ;; Finished

    ;; Send
    mov al, 0x20
    mov dx, 0x1F7
    out dx, al
    ;; Finished

;; Read all sectors into the memory
next_sector:
    push ecx

;; Check if we can read from the disk
try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz try_again
;; At this point, we are ready to read
;; Read 256 words (512 bytes) at a time
mov ecx, 256
mov dx, 0x1F0
;; rep: repeat x times where x is the value in ecx
;; insw: read a word (2-bytes) from the port pointed by dx
;; store the data to ES:DI
;; (in our case, the 512 bytes (a sector)
;; will be read from the disk and stored at mem address starting at 0x0100000 because edi = 0x0100000)
rep insw

;; restore ecx from the stack and loop (loop will decrement ecx) until ecx is zero
pop ecx
loop next_sector
ret
;; Finished reading all sectors (100 sectors in our case) into ES:DI


times 510-($ - $$) db 0
dw 0xAA55