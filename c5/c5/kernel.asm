; $ nasm -f elf kernel.asm -o -kernel.o
; $ ld -m elf_i386 -s kernel.o -o kernel.bin


[section .text]

global _start

_start:     ; 假设gs指向显存
    mov ah,0Fh
    mov al,'K'
    mov [gs:((80*1+39)*2)],ax
    jmp $
