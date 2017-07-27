; nasm -f elf kernel.asm -o kernel.o
; ld -m elf_i386 -s -Ttext 0x30400 -o kernel.bin kernel.o


SELECTOR_KERNEL_CS      equ     8

; 导入函数
extern cstart

; 导出全局变量
extern gdt_ptr

[section .bss]
StackSpace      resb    2 * 1024
StackTop:       ; 栈顶        

[section .text]

global _start

_start:     
        mov esp,StackTop
        sgdt    [gdt_ptr]
        call cstart
        lgdt [gdt_ptr]
    
        jmp SELECTOR_KERNEL_CS:csinit

csinit:
        push 0
        popfd

        hlt
